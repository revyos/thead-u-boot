/*
 * (C) Copyright 2018, Linaro Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <command.h>
#include <env.h>
#include <configs/light-c910.h>
#include <asm/arch-thead/boot_mode.h>
#include "sec_library.h"

#define ENV_SECIMG_LOAD     "sec_m_load"
#define VAL_SECIMG_LOAD     "ext4load mmc ${mmcdev}:${mmcteepart} $tf_addr trust_firmware.bin; ext4load mmc ${mmcdev}:${mmcteepart} $tee_addr tee.bin\0"

#define RPMB_BLOCK_SIZE 256
#define RPMB_ROLLBACK_BLOCK_START 1

#ifndef LIGHT_KDF_RPMB_KEY
static const unsigned char emmc_rpmb_key_sample[32] = {0x33, 0x22, 0x11, 0x00, 0x77, 0x66, 0x55, 0x44, \
												0xbb, 0xaa, 0x99, 0x88, 0xff, 0xee, 0xdd, 0xcc, \
												0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
												0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif

extern int sprintf(char *buf, const char *fmt, ...);
extern char * get_slot_name_suffix(void);

static int get_rpmb_key(uint8_t key[32])
{
#ifndef LIGHT_KDF_RPMB_KEY
    memcpy(key, emmc_rpmb_key_sample, sizeof(emmc_rpmb_key_sample));

    return 0;
#else
	uint32_t kdf_rpmb_key_length = 0;
	int ret = 0;
	ret = csi_kdf_gen_hmac_key(key, &kdf_rpmb_key_length);
	if (ret != 0) {
		return -1;
	}

    return 0;
#endif
}

static int get_image_file_size(unsigned long img_src_addr)
{
	img_header_t *img = (img_header_t *)img_src_addr;
	uint8_t magiccode[4] = {0};

	magiccode[3] = img->magic_num & 0xff;
	magiccode[2] = (img->magic_num & 0xff00) >> 8;
	magiccode[1] = (img->magic_num & 0xff0000) >> 16;
	magiccode[0] = (img->magic_num & 0xff000000) >> 24;
	if (memcmp(header_magic, magiccode, 4) == 0) {
		return -1;
	}

	return img->image_size;
}

static int verify_and_load_image(unsigned long image_addr_src, unsigned long image_addr_dst)
{
    int ret = 0;
	unsigned int image_size = 0;

	if (image_have_head(image_addr_src) == 1) {
        ret = csi_sec_init();
        if (ret != 0) {
			return -1;
		}

        ret = csi_sec_custom_image_verify(image_addr_src, UBOOT_STAGE_ADDR);
        if (ret != 0) {
            printf("image verify error\r\n");
            return -2;
        }

		image_size = get_image_file_size(image_addr_src);
		if (image_size  < 0) {
            printf("image get size error\r\n");
			return -3;
		}

		memmove((void *)image_addr_dst, (const void *)(image_addr_src + HEADER_SIZE), image_size);
	} else {
        printf("in secure mode but image has no header\r\n");
		return -4;
	}

    return 0;
}

int verify_and_load_tee_tf_image(void)
{
    int ret = 0;

    ret = verify_and_load_image(LIGHT_TF_FW_TMP_ADDR, LIGHT_TF_FW_ADDR);
    if (ret != 0) {
        printf("verify tf image failed\r\n");
        return ret;
    }
    printf("verify trust firmware image success\r\n");

    ret = verify_and_load_image(LIGHT_TEE_FW_ADDR, LIGHT_TEE_FW_ADDR);
    if (ret != 0) {
        printf("verify tee image failed\r\n");
        return ret;
    }
    printf("verify tee image success\r\n");

    return 0;
}

/* In order to use common bootloader for both secure boot and non-secure boot,
   we only know the boot type through reading the sec_boot field in efuse. Due to
   the efuse is only accessed in lifecycle(DEV/OEM/PRO/RMP), we ensure it must be
   non-secure boot in lifecycle(INIT) */
bool get_system_boot_type(void)
{
	bool btype = true; /* false: non-secure boot | true: secure boot */
#if 0
	int lc = 0;
	sboot_st_t sb_flag = SECURE_BOOT_DIS;
	int ret = 0;
#endif
	int sb_emulater = 0;

	sb_emulater = env_get_ulong("sb_emulater", 10, 0);
	if (sb_emulater == 0) {
		btype = false;
	}
# if 0
	ret = csi_efuse_get_lc(&lc);
	/* 0: LC_INIT, 1: LC_DEV, 2: LC_OEM, 3: LC_PRO */
	if ((ret == 0) && (lc != 0)) {
		csi_efuse_api_init();

		/* Check platform secure boot enable ? */
		ret = csi_efuse_get_secure_boot_st(&sb_flag);
		if ((ret == 0) && (sb_flag == SECURE_BOOT_EN))
			btype = true;

		csi_efuse_api_uninit();
	}
#endif
	return btype;
}

int sec_read_rollback_index(size_t rollback_index_slot, uint64_t *out_rollback_index)
{
	char runcmd[64] = {0};
	unsigned char blkdata[RPMB_BLOCK_SIZE];
    size_t rpmb_block = (rollback_index_slot * sizeof(uint64_t)) / RPMB_BLOCK_SIZE + RPMB_ROLLBACK_BLOCK_START;
    size_t rpmb_offset = (rollback_index_slot * sizeof(uint64_t)) % RPMB_BLOCK_SIZE;

    sprintf(runcmd, "mmc rpmb read 0x%lx %ld 1", (unsigned long)blkdata, rpmb_block);
	if(run_command(runcmd, 0)) {
        printf("read_rollback_index failed, mmc read error\r\n");
        return -1;
    }

    *out_rollback_index = *(uint64_t*)(blkdata + rpmb_offset);
	return 0;
}

int sec_write_rollback_index(size_t rollback_index_slot, uint64_t rollback_index)
{
    char runcmd[64] = {0};
	unsigned char blkdata[RPMB_BLOCK_SIZE];
    size_t rpmb_block = (rollback_index_slot * sizeof(uint64_t)) / RPMB_BLOCK_SIZE + RPMB_ROLLBACK_BLOCK_START;
    size_t rpmb_offset = (rollback_index_slot * sizeof(uint64_t)) % RPMB_BLOCK_SIZE;
    uint8_t rpmb_key[32];

    sprintf(runcmd, "mmc rpmb read 0x%lx %ld 1", (unsigned long)blkdata, rpmb_block);
	if(run_command(runcmd, 0)) {
        printf("read_rollback_index failed, mmc read error\r\n");
        return -1;
    }

    *(uint64_t*)(blkdata + rpmb_offset) = rollback_index;

    if (get_rpmb_key(rpmb_key) != 0) {
        return -2;
    }

	sprintf(runcmd, "mmc rpmb write 0x%lx %ld 1 0x%lx", (unsigned long)blkdata, rpmb_block, (unsigned long)rpmb_key);
    if(run_command(runcmd, 0)) {
        printf("read_rollback_index failed, mmc write error\r\n");
        return -3;
    }

    return 0;
}

static int do_secimg_load(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    bool sb_enable = false;
    const char *secimgs_load_str = VAL_SECIMG_LOAD;
    int ret = -1;
    int teepart = 0;

#ifdef CONFIG_ANDROID_AB
	char *slot_suffix = get_slot_name_suffix();
    teepart = env_get_ulong("mmcteepart", 10, 8);
	if ((strcmp(slot_suffix, "_a") == 0) && (teepart != 8)) {
		/* Switch mmcbootpart to "_b" */
		env_set_ulong("mmcbootpart", 2);
		/* Switch mmcteepart to "_b" */
		env_set_ulong("mmcteepart", 8);
	} else if ((strcmp(slot_suffix, "_b") == 0) && (teepart != 9)){
		/* Switch mmcbootpart to "_b" */
		env_set_ulong("mmcbootpart", 3);
		/* Switch mmcteepart to "_b" */
		env_set_ulong("mmcteepart", 9);
	}
#endif

	sb_enable = get_system_boot_type();
	if (sb_enable) {
		/* By default, the value for ENV-SEC-M-LOAD is always to load opensbi image.
		* if secure boot is enable, we force to change the value to load tee image.
		* but Never to save it in volatile-RAM
		*/
		ret = env_set(ENV_SECIMG_LOAD, secimgs_load_str);
		if (ret != 0) {
			printf("Rewrite ENV (%s) fails\n", ENV_SECIMG_LOAD);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	secimg_load, 1, 1,	do_secimg_load,
	"Runtime-load secure image if secure system is enable",
	NULL
);
