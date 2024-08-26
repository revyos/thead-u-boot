// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mapmem.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include <env.h>
#include <env_internal.h>
#include "tpm-user-utils.h"
#include "sec_library.h"

enum mboot_type {
	UBOOT_IMAGE = 0,
	KERNEL_IMAGE,
	PARTITIONS_STR,
	MBOOT_TYPE_MAX,
};
enum pcr_index {
	PCR_0 = 0,
	PCR_1,
	PCR_2,
	PCR_3,
	PCR_4,
	PCR_5,
	PCR_6,
	PCR_7,
};
static uint8_t image_digest[32] __attribute__((aligned(64))) = { 0 };

#define CHECK_RET_WITH_RET(x, ret)                                                                 \
    do {                                                                                           \
        if (!(x)) {                                                                                \
            return ret;                                                                            \
        }                                                                                          \
    } while (0)

static uint32_t hash_image_sha256(long image_addr, size_t size, void *digest, uint32_t *digest_len)
{
	uint32_t ret;
	sc_sha_t sha;
	sc_sha_context_t ctx;

	CHECK_RET_WITH_RET(ret = csi_sec_library_init(), ret);
	CHECK_RET_WITH_RET(ret = sc_sha_init(&sha, 0), ret);
	CHECK_RET_WITH_RET(ret = sc_sha_start(&sha, &ctx, SC_SHA_MODE_256), ret);
	CHECK_RET_WITH_RET(ret = sc_sha_trans_config(&sha, &ctx, SC_SHA_DMA_MODE), ret);
	CHECK_RET_WITH_RET(ret = sc_sha_update(&sha, &ctx, (void *)image_addr, size), ret);
	CHECK_RET_WITH_RET(ret = sc_sha_finish(&sha, &ctx, digest, digest_len), ret);

	return SC_OK;
}

static int do_measured_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct tpm_chip_priv *priv;
	uint32_t index, type;
	uint32_t rc;
	int ret;
	long image_addr = 0;
	size_t image_size = 0;
	uint32_t image_digest_len = 0;
	char *partitions_str = NULL;

	if (argc != 1)
		return CMD_RET_USAGE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	tpm_init(dev); /* Initialization TPM2 chip */
	rc = tpm2_startup(dev, TPM2_SU_CLEAR); /* Startup TPM2 chip with mode TPM_ST_CLEAR*/
	if (rc)
		report_return_code(rc);

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -EINVAL;

	for (type = UBOOT_IMAGE; type < MBOOT_TYPE_MAX; type++) {
		if (type == UBOOT_IMAGE) { /*U-BOOT Image */
			index = PCR_0;
			image_addr = CONFIG_SPL_TEXT_BASE;
			image_size = CONFIG_SPL_MAX_SIZE+CONFIG_SYS_MONITOR_LEN;
		} else if (type == KERNEL_IMAGE) { /* KERNEL Image */
			index = PCR_0;
			image_addr = 0x00200000;
			if (fs_set_blk_dev("mmc", "0:2", 2))
				return -EINVAL;
			if (fs_size("Image", &image_size) < 0)
				return -EINVAL;
		} else if (type == PARTITIONS_STR) { /* PARTITIONS */
			index = PCR_5;
			partitions_str = env_get("partitions");
			image_addr = (long)partitions_str;
			image_size = strlen(partitions_str);
		}

		rc = hash_image_sha256(image_addr, image_size, image_digest, &image_digest_len);
		if (rc)
			return -EINVAL;

		rc = tpm2_pcr_extend(dev, index, image_digest);
		if (rc)
			break;
	}

	return report_return_code(rc);
}

U_BOOT_CMD(
	measured_boot, CONFIG_SYS_MAXARGS, 1, do_measured_boot,
	"extend hash(u-boot), hash(kernel), hash(partitions str) to pcr0 and pcr5",
	""
);
