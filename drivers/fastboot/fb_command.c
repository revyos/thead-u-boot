// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <part.h>
#include <stdlib.h>

#define BLOCK_SIZE 512
#define BOARD_ID_OFFSET 0x26

/**
 * image_size - final fastboot image size
 */
static u32 image_size;

/**
 * fastboot_bytes_received - number of bytes received in the current download
 */
static u32 fastboot_bytes_received;

/**
 * fastboot_bytes_expected - number of bytes expected in the current download
 */
static u32 fastboot_bytes_expected;

static void okay(char *, char *);
static void getvar(char *, char *);
static void download(char *, char *);
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void flash(char *, char *);
static void erase(char *, char *);
#endif
static void reboot_bootloader(char *, char *);
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT)
static void oem_format(char *, char *);
#endif
static void oem_command(char *, char *);
int image_have_head(unsigned long img_src_addr);

static const struct {
	const char *command;
	void (*dispatch)(char *cmd_parameter, char *response);
} commands[FASTBOOT_COMMAND_COUNT] = {
	[FASTBOOT_COMMAND_GETVAR] = {
		.command = "getvar",
		.dispatch = getvar
	},
	[FASTBOOT_COMMAND_DOWNLOAD] = {
		.command = "download",
		.dispatch = download
	},
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	[FASTBOOT_COMMAND_FLASH] =  {
		.command = "flash",
		.dispatch = flash
	},
	[FASTBOOT_COMMAND_ERASE] =  {
		.command = "erase",
		.dispatch = erase
	},
#endif
	[FASTBOOT_COMMAND_BOOT] =  {
		.command = "boot",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_CONTINUE] =  {
		.command = "continue",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT] =  {
		.command = "reboot",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT_BOOTLOADER] =  {
		.command = "reboot-bootloader",
		.dispatch = reboot_bootloader
	},
	[FASTBOOT_COMMAND_SET_ACTIVE] =  {
		.command = "set_active",
		.dispatch = okay
	},
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT)
	[FASTBOOT_COMMAND_OEM_FORMAT] = {
		.command = "oem format",
		.dispatch = oem_format,
	},
#endif
	[FASTBOOT_COMMAND_OEM_COMMAND] = {
		.command = "oem command",
		.dispatch = oem_command,
	},
};

/**
 * fastboot_handle_command - Handle fastboot command
 *
 * @cmd_string: Pointer to command string
 * @response: Pointer to fastboot response buffer
 *
 * Return: Executed command, or -1 if not recognized
 */
int fastboot_handle_command(char *cmd_string, char *response)
{
	int i;
	char *cmd_parameter;

	cmd_parameter = cmd_string;
	strsep(&cmd_parameter, ":");

	for (i = 0; i < FASTBOOT_COMMAND_COUNT; i++) {
		if (!strcmp(commands[i].command, cmd_string)) {
			if (commands[i].dispatch) {
				commands[i].dispatch(cmd_parameter,
							response);
				return i;
			} else {
				break;
			}
		}
	}

	pr_err("command %s not recognized.\n", cmd_string);
	fastboot_fail("unrecognized command", response);
	return -1;
}

/**
 * okay() - Send bare OKAY response
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Send a bare OKAY fastboot response. This is used where the command is
 * valid, but all the work is done after the response has been sent (e.g.
 * boot, reboot etc.)
 */
static void okay(char *cmd_parameter, char *response)
{
	fastboot_okay(NULL, response);
}

/**
 * getvar() - Read a config/version variable
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void getvar(char *cmd_parameter, char *response)
{
	fastboot_getvar(cmd_parameter, response);
}

/**
 * fastboot_download() - Start a download transfer from the client
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void download(char *cmd_parameter, char *response)
{
	char *tmp;

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}
	fastboot_bytes_received = 0;
	fastboot_bytes_expected = simple_strtoul(cmd_parameter, &tmp, 16);
	if (fastboot_bytes_expected == 0) {
		fastboot_fail("Expected nonzero image size", response);
		return;
	}
	/*
	 * Nothing to download yet. Response is of the form:
	 * [DATA|FAIL]$cmd_parameter
	 *
	 * where cmd_parameter is an 8 digit hexadecimal number
	 */
	if (fastboot_bytes_expected > fastboot_buf_size) {
		fastboot_fail(cmd_parameter, response);
	} else {
		printf("Starting download of %d bytes\n",
		       fastboot_bytes_expected);
		fastboot_response("DATA", response, "%s", cmd_parameter);
	}
}

/**
 * fastboot_data_remaining() - return bytes remaining in current transfer
 *
 * Return: Number of bytes left in the current download
 */
u32 fastboot_data_remaining(void)
{
	return fastboot_bytes_expected - fastboot_bytes_received;
}

/**
 * fastboot_data_download() - Copy image data to fastboot_buf_addr.
 *
 * @fastboot_data: Pointer to received fastboot data
 * @fastboot_data_len: Length of received fastboot data
 * @response: Pointer to fastboot response buffer
 *
 * Copies image data from fastboot_data to fastboot_buf_addr. Writes to
 * response. fastboot_bytes_received is updated to indicate the number
 * of bytes that have been transferred.
 *
 * On completion sets image_size and ${filesize} to the total size of the
 * downloaded image.
 */
void fastboot_data_download(const void *fastboot_data,
			    unsigned int fastboot_data_len,
			    char *response)
{
#define BYTES_PER_DOT	0x20000
	u32 pre_dot_num, now_dot_num;

	if (fastboot_data_len == 0 ||
	    (fastboot_bytes_received + fastboot_data_len) >
	    fastboot_bytes_expected) {
		fastboot_fail("Received invalid data length",
			      response);
		return;
	}
	/* Download data to fastboot_buf_addr */
	memcpy(fastboot_buf_addr + fastboot_bytes_received,
	       fastboot_data, fastboot_data_len);

	pre_dot_num = fastboot_bytes_received / BYTES_PER_DOT;
	fastboot_bytes_received += fastboot_data_len;
	now_dot_num = fastboot_bytes_received / BYTES_PER_DOT;

	if (pre_dot_num != now_dot_num) {
		putc('.');
		if (!(now_dot_num % 74))
			putc('\n');
	}
	*response = '\0';
}

/**
 * fastboot_data_complete() - Mark current transfer complete
 *
 * @response: Pointer to fastboot response buffer
 *
 * Set image_size and ${filesize} to the total size of the downloaded image.
 */
void fastboot_data_complete(char *response)
{
	/* Download complete. Respond with "OKAY" */
	fastboot_okay(NULL, response);
	printf("\ndownloading of %d bytes finished\n", fastboot_bytes_received);
	image_size = fastboot_bytes_received;
	env_set_hex("filesize", image_size);
	fastboot_bytes_expected = 0;
	fastboot_bytes_received = 0;
}

/**
 * check_image_board_id() - check if board id in image matched with board id in env 
 *
 * @image_data: Image data
 *
 * 0 if success otherwise failed
 */
int check_image_board_id(uint8_t *image_data)
{
	char *env_board_id = NULL;
    char board_id[3] = {0};

	env_board_id = env_get("board#");

	/*if current board id is null or image has no header,skip check*/
	if (env_board_id == NULL || env_board_id[0] == 0 || image_have_head((unsigned long)image_data) == 0) {
		return 0;
	}

	memcpy(board_id, image_data + BOARD_ID_OFFSET,sizeof(uint16_t));

    /*if image board id is null,skip check*/
    if (*(uint16_t*)board_id == 0) {
        return 0;
	}

    /*check if current board id match with board id in image*/
	if (strncmp(env_board_id, board_id, sizeof(board_id)) != 0) {
	    printf("U-BOOT image download via fastboot is interrupted due to the U-BOOT for board %s does not work in the board %s\r\n",board_id,env_board_id);
		return -1;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
/**
 * flash() - write the downloaded image to the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Writes the previously downloaded image to the partition indicated by
 * cmd_parameter. Writes to response.
 */
static void flash(char *cmd_parameter, char *response)
{
#ifdef THEAD_LIGHT_FASTBOOT
	char cmdbuf[32];
	u32 block_cnt;
	struct blk_desc *dev_desc;
    int ret = 0;

	if (strcmp(cmd_parameter, "uboot") == 0) {
        ret = check_image_board_id(fastboot_buf_addr);
		if (ret != 0) {
            fastboot_fail("U-BOOT image does not match the type of BOARD", response);
			return;
		}

		dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
		if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
			fastboot_fail("invalid mmc device", response);
			return;
        }

		run_command("mmc partconf 0 1 0 1", 0);

		block_cnt = image_size / BLOCK_SIZE;
		if (image_size % BLOCK_SIZE) {
			block_cnt = block_cnt +1;
		}

		sprintf(cmdbuf, "mmc write 0x%p 0 %x", fastboot_buf_addr, block_cnt);

		run_command(cmdbuf, 0);
		run_command("mmc partconf 0 1 0 0", 0);
	} else if ((strcmp(cmd_parameter, "fw") == 0)) {
		memcpy((void *)LIGHT_FW_ADDR, fastboot_buf_addr, image_size);
	} else if ((strcmp(cmd_parameter, "uImage") == 0)) {
		memcpy((void *)LIGHT_KERNEL_ADDR, fastboot_buf_addr, image_size);
	} else if ((strcmp(cmd_parameter, "dtb") == 0)) {
		memcpy((void *)LIGHT_DTB_ADDR, fastboot_buf_addr, image_size);
	} else if ((strcmp(cmd_parameter, "rootfs") == 0)) {
		memcpy((void *)LIGHT_ROOTFS_ADDR, fastboot_buf_addr, image_size);
	} else if ((strcmp(cmd_parameter, "aon") == 0)) {
		memcpy((void *)LIGHT_AON_FW_ADDR, fastboot_buf_addr, image_size);
	} else if ((strcmp(cmd_parameter, TF_PART_NAME) == 0)) {
		memcpy((void *)LIGHT_TF_FW_ADDR, fastboot_buf_addr, image_size);
	} else if ((strcmp(cmd_parameter, TEE_PART_NAME) == 0)) {
		memcpy((void *)LIGHT_TEE_FW_ADDR, fastboot_buf_addr, image_size);
	} 

	if(strcmp(cmd_parameter, "uboot") == 0 || (strcmp(cmd_parameter, "fw") == 0) ||
	   (strcmp(cmd_parameter, "uImage") == 0) || (strcmp(cmd_parameter, "dtb") == 0) ||
	   (strcmp(cmd_parameter, "rootfs") == 0) || (strcmp(cmd_parameter, "aon") == 0)) {
		fastboot_okay(NULL, response);
		return;
	}
#endif

#if CONFIG_IS_ENABLED(LIGHT_SEC_UPGRADE)
	if(strcmp(cmd_parameter, TF_IMG_UPD_NAME) == 0) {
		#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
		/* tee/tf/uboot image must be written into stash partition */
		sprintf(cmdbuf, "%s", STASH_PART_NAME);
		fastboot_mmc_flash_write(cmdbuf, fastboot_buf_addr, image_size, response);
		#endif
		/* Send ACK to host */
		fastboot_okay(NULL, response);
		
		/* set secure upgrade flag to indicate it is TF image upgrade*/
		sprintf(cmdbuf,"env set sec_upgrade_mode 0x%x", TF_SEC_UPGRADE_FLAG);
		run_command(cmdbuf, 0);
		run_command("saveenv", 0);
		run_command("reset", 0);
		return;
	} else if (strcmp(cmd_parameter, TEE_IMG_UPD_NAME) == 0) {
		#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
		/* tee/tf/uboot image must be written into stash partition */
		sprintf(cmdbuf, "%s", STASH_PART_NAME);
		fastboot_mmc_flash_write(cmdbuf, fastboot_buf_addr, image_size, response);
		#endif

		/* Send ACK to host */
		fastboot_okay(NULL, response);
		
		/* set secure upgrade flag to indicate it is TEE image upgrade*/
		sprintf(cmdbuf,"env set sec_upgrade_mode 0x%x", TEE_SEC_UPGRADE_FLAG);
		run_command(cmdbuf, 0);
		run_command("saveenv", 0);
		run_command("reset", 0);
		return;
	} else if (strcmp(cmd_parameter, UBOOT_IMG_UPD_NAME) == 0) {
		#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)

		env_set_hex("ubootupdsize", image_size);
		/* tee/tf/uboot image must be written into stash partition */
		sprintf(cmdbuf, "%s", STASH_PART_NAME);
		fastboot_mmc_flash_write(cmdbuf, fastboot_buf_addr, image_size, response);
		#endif

		/* Send ACK to host */
		fastboot_okay(NULL, response);
		
		/* set secure upgrade flag to indicate it is UBOOT image upgrade*/
		sprintf(cmdbuf,"env set sec_upgrade_mode 0x%x", UBOOT_SEC_UPGRADE_FLAG);
		run_command(cmdbuf, 0);
		run_command("saveenv", 0);
		run_command("reset", 0);
		return;
	} 
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	printf("cmd_parameter: %s, imagesize: %d\n", cmd_parameter, image_size);
	fastboot_mmc_flash_write(cmd_parameter, fastboot_buf_addr, image_size,
				 response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	fastboot_nand_flash_write(cmd_parameter, fastboot_buf_addr, image_size,
				  response);
#endif
}

/**
 * erase() - erase the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Erases the partition indicated by cmd_parameter (clear to 0x00s). Writes
 * to response.
 */
static void erase(char *cmd_parameter, char *response)
{
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	fastboot_mmc_erase(cmd_parameter, response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	fastboot_nand_erase(cmd_parameter, response);
#endif
}
#endif

/**
 * reboot_bootloader() - Sets reboot bootloader flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_bootloader(char *cmd_parameter, char *response)
{
	if (fastboot_set_reboot_flag())
		fastboot_fail("Cannot set reboot flag", response);
	else
		fastboot_okay(NULL, response);
}

#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT)
/**
 * oem_format() - Execute the OEM format command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void oem_format(char *cmd_parameter, char *response)
{
	char cmdbuf[32];

	if (!env_get("partitions")) {
		fastboot_fail("partitions not set", response);
	} else {
		sprintf(cmdbuf, "gpt write mmc %x $partitions",
			CONFIG_FASTBOOT_FLASH_MMC_DEV);
		if (run_command(cmdbuf, 0))
			fastboot_fail("", response);
		else
			fastboot_okay(NULL, response);
	}
}
#endif

/**
 * oem_command() - Execute the OEM command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void oem_command(char *cmd_parameter, char *response)
{
	if (run_command(cmd_parameter, 0))
		fastboot_fail("", response);
	else
		fastboot_okay(NULL, response);
}
