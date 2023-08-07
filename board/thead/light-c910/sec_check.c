/*
* Copyright (C) 2017-2021 Alibaba Group Holding Limited
*
* SPDX-License-Identifier: GPL-2.0+
*/

#include <common.h>
#include <command.h>
#include <console.h>
#include <malloc.h>
#include <linux/errno.h>
#include <asm/arch-thead/boot_mode.h>
#include "../../../lib/sec_library/include/csi_sec_img_verify.h"

extern int csi_efuse_api_init(void);
extern int csi_efuse_api_unint(void);
extern int csi_efuse_read_raw(uint32_t addr, void *data, uint32_t cnt);
extern int csi_efuse_write_raw(uint32_t addr, const void *data, uint32_t cnt);
extern uint32_t rambus_crypto_init(void);

int image_have_head(unsigned long img_src_addr)
{
    uint8_t *buffer = (uint8_t *)img_src_addr;

    if (memcmp(header_magic, &buffer[4], 4) == 0) {
        return 1;
    }

    return 0;
}

int csi_sec_init(void)
{
	int ret;
	char *version;

	/* Initialize eFuse module */
	ret = csi_efuse_api_init();
	if (ret) {
		printf("efuse init faild[%d]\n", ret);
		 goto exit;
	}

	ret = rambus_crypto_init();
	if (ret) {
		printf("rambus_crypto_init faild[%d]\n", ret);
		goto exit;
	}

	csi_sec_get_lib_version(&version);
	csi_sec_set_boot_stage(0);
	printf("sec version: %s \n", version);

exit:
	return ret;
}


#define LIGHT_MAC1ADDR_OFF	176
void designware_get_mac_from_fuse(unsigned char *mac)
{
	int ret;

	/* Initialize eFuse module */
	ret = csi_efuse_api_init();
	if (ret) {
		printf("efuse init faild[%d]\n", ret);
		return;
	}

	ret = csi_efuse_read_raw(LIGHT_MAC1ADDR_OFF, mac, 6);
	if (ret) {
		printf("efuse macaddr read faild[%d]\n", ret);
		return;
	}
}

#if CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_A) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_B) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_ANT_REF) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_LPI4A)
/* Secure function for image verificaiton here */
int get_image_version(unsigned long img_src_addr)
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
	
	return img->image_version;
}

int get_image_size(unsigned long img_src_addr)
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

void dump_image_header_info(long addr)
{
	img_header_t *phead = (img_header_t *)addr;

	printf("\n---------------------------------------------\n");
	printf("entry point: 0x%x\n", phead->entry_point);
	printf("image size: %d Bytes\n", phead->image_size);
	printf("head version: 0x%x\n", phead->head_version);
	printf("image version: 0x%x\n", phead->image_version);
	printf("image checksum: 0x%x\n", phead->image_checksum);
	printf("image run addr: 0x%llx\n", phead->image_run_addr);
	printf("image offset: 0x%x\n", phead->image_offset);
	printf("image digest scheme: %d\n", phead->digest_scheme);
	printf("image sign scheme: %d\n", phead->signature_scheme);
	printf("image encrypt type: %d\n", phead->encrypt_type);
	printf("\n---------------------------------------------\n");
}

int verify_customer_image(img_type_t type, long addr)
{
	int ret;
    
	/* Double check image number */
	if (image_have_head(addr) == 0)
		return -1;

	/* Dump image header information here */
	dump_image_header_info(addr);

	/* Call customer image verification function */
	if ((type == T_TF) || (type == T_TEE) || (type == T_KRLIMG)) {
		ret = csi_sec_custom_image_verify(addr, UBOOT_STAGE_ADDR);
		if (ret) {
			printf("Image(%d) is verified fail, Please go to check!\n\n", type);
			return ret;
		}
	} else if (type == T_UBOOT) {
		ret = csi_sec_uboot_image_verify(addr, addr - PUBKEY_HEADER_SIZE);
		if (ret) {
			printf("Image(%s) is verified fail, Please go to check!\n\n", "uboot");
			return ret;
		}
	}

	return 0;
}
#else
static int strtou32(const char *str, unsigned int base, u32 *result)
{
	char *ep;

	*result = simple_strtoul(str, &ep, base);
	if (ep == str || *ep != '\0')
		return -EINVAL;

	return 0;
}

static int confirm_prog(void)
{
	puts("Warning: Programming fuses is an irreversible operation!\n"
			"         This may brick your system.\n"
			"         Use this command only if you are sure of "
					"what you are doing!\n"
			"\nReally perform this fuse programming? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Fuse programming aborted\n");
	return 0;
}

static int do_fuse(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const char *op = argc >= 2 ? argv[1] : NULL;
	int confirmed = argc >= 3 && !strcmp(argv[2], "-y");
	u32 addr, cnt, val;
	u8 *data;
	int ret, i;

	/* Initialize eFuse module */
	ret = csi_efuse_api_init();
	if (ret) {
		printf("efuse init faild[%d]\n", ret);
		goto err;
	}

	argc -= 2 + confirmed;
	argv += 2 + confirmed;

	if (argc < 1 || strtou32(argv[0], 0, &addr))
		return CMD_RET_USAGE;

	if (!strcmp(op, "read")) {
		if (argc == 1)
			cnt = 1;
		else if (argc != 2 || strtou32(argv[1], 0, &cnt))
			return CMD_RET_USAGE;

		printf("Reading addr %u:\n", addr);
		{
			data = malloc(cnt);
			ret = csi_efuse_read_raw(addr, data, cnt);
			if (ret) {
				free(data);
				goto err;
			}
			for (i = 0; i < cnt; i++)
				printf(" 0x%.2x", data[i]);
			free(data);
		}
		putc('\n');
	} else if (!strcmp(op, "write")) {
		if (argc < 2)
			return CMD_RET_USAGE;

		data = malloc(argc - 1);
		printf("Programming addr %u  to\n", addr);

		for (i = 1; i < argc; i++) {
			if (strtou32(argv[i], 16, &val))
				return CMD_RET_USAGE;

			data[i-1] = val;
			printf(" 0x%.2x\n", val);
		}

		cnt = argc - 1;

		if (!confirmed && !confirm_prog()) {
			free(data);
			return CMD_RET_FAILURE;
		}

		ret = csi_efuse_write_raw(addr, data, cnt);
		if (ret) {
			free(data);
			goto err;
		}
		free(data);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;

err:
	puts("ERROR\n");
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	efuse, CONFIG_SYS_MAXARGS, 0, do_fuse,
	"eFuse sub-system",
	"read <addr> [<cnt>] - read 1 or 'cnt' fuse bytes,\n"
	" starting at 'addr'\n"
	"efuse write [-y] <addr> <hexval> [<hexval>...]  - program 1 or\n"
	" several fuse bytes, starting at 'addr'\n"
);
#endif
