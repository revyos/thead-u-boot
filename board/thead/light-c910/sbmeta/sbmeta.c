/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#include "sbmeta.h"
#include "sec_crypto_sha.h"

#define LOGLEVEL_ERROR          1
#define LOGLEVEL_INFO           2
#define LOGLEVEL_DEBUG          3
#define SBMETA_LOGLEVEL         1
#define trace_printer(level, fmt,...)   printf("%s"fmt, level, ##__VA_ARGS__)
#if (SBMETA_LOGLEVEL < 1)
#define EMSG(...)
#else
#define EMSG(fmt, args...)      trace_printer("error: ", fmt, ##args)
#endif

#if (SBMETA_LOGLEVEL < 2)
#define IMSG(...)
#else
#define IMSG(fmt, args...)      trace_printer("info: ", fmt, ##args)
#endif

#if (SBMETA_LOGLEVEL < 3)
#define DMSG(...)
#else
#define DMSG(fmt, args...)      trace_printer("", fmt, ##args)
#endif

#if CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_A) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_B) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_LPI4A)
#if CONFIG_IS_ENABLED(LIGHT_SEC_UPGRADE)
/* digest_size corresponding to digest_scheme specified in sbmeta_info_t */
static const int digest_size[] = {0, 20, 16, 28, 32, 48, 64, 32};
static const char* image_name_s[] = {
    "dtb", "kernel", "tf", "aon", "rootfs", "tee", "uboot", "user"
};
/* index to get sc_sha_mode_t value */
static const int sha_idx2ctl[] = {0, 1, 8, 3, 2, 5, 4, 9};

static const unsigned long image_addrs[] = {
    LIGHT_DTB_ADDR,
    LIGHT_KERNEL_ADDR,
    LIGHT_TF_FW_TMP_ADDR,
    LIGHT_AON_FW_ADDR,
    LIGHT_ROOTFS_ADDR,
    LIGHT_TEE_FW_ADDR,
    CONFIG_SYS_TEXT_BASE,
};

typedef struct {
    int         magiccode;
    uint8_t     dev;
    uint8_t     part;
    uint8_t     image_type;
    uint8_t     digest_scheme;
    uint8_t     sign_scheme;
    uint8_t     isencrypted;
    uint8_t     medium_type;
    uint8_t     checksum_scheme;
    char        filename[MAX_NAME_SIZE];
    uint8_t     digest[MAX_DIGEST_SIZE];
    uint32_t    relocated_addr;
    uint32_t    reserved[4];
} sbmeta_info_t;

static int is_sbmeta_info(uint32_t entry_src_addr)
{
    uint32_t *buffer = (uint32_t *)(uintptr_t)entry_src_addr;

    /* sbmeta_info_t entry should start with magic code 'S''B''M''T' */
    if (*buffer != SBMETA_MAGIC) {
        return CMD_RET_FAILURE;
    }

    return 0;
}

static int dump_sbmeta_info(sbmeta_info_t *sbmeta_info)
{
    if (sbmeta_info == NULL) {
        return CMD_RET_FAILURE;
    }
    /* only support emmc now */
    if (sbmeta_info->medium_type != 0) {
        EMSG("medium type %d is not supported now\r\n", sbmeta_info->medium_type);
        return CMD_RET_FAILURE;
    }
    /* only support dtb, krlimg/tf, sbi, aon, rootfs, tee, uboot and user-defined type */
    if (sbmeta_info->image_type > IMAGE_TYPE_NUM || sbmeta_info->image_type < 0) {
        EMSG("image type is out of range\r\n");
        return CMD_RET_FAILURE;
    }
    /* only support none, sha1, md5, sha224, sha256, sha384, sha512, sm3 and reserved scheme */
    if (sbmeta_info->digest_scheme > DIGEST_TYPE_NUM || sbmeta_info->digest_scheme < 0) {
        EMSG("digest type is out of range\r\n");
        return CMD_RET_FAILURE;
    }
    /* only support none, rsa1024, rsa2048, ecc256, ecc160, sm2 and reserved scheme */
    if (sbmeta_info->sign_scheme > SIGN_TYPE_NUM || sbmeta_info->sign_scheme < 0) {
        EMSG("signature type is out of range\r\n");
        return CMD_RET_FAILURE;
    }
    /* DTB, TF, TEE, Kernel will be loaded from default partitions specified in env */
    if (sbmeta_info->image_type != T_ROOTFS && sbmeta_info->image_type != T_USER) {
        IMSG("Image has been loaded\r\n");
    }

    /* dump sbmeta_info_t */
    DMSG("image medium type: %d\n", sbmeta_info->medium_type);
    DMSG("image load part: mmc %d:%d\n", sbmeta_info->dev, sbmeta_info->part);
    DMSG("image type: %d \n", sbmeta_info->image_type);
    DMSG("image digest scheme: %d\n", sbmeta_info->digest_scheme);
    DMSG("image sign scheme: %d\n", sbmeta_info->sign_scheme);
    DMSG("image enable encryption: %s\n", sbmeta_info->isencrypted ? "en" : "dis");
    DMSG("image file name: %s\n", sbmeta_info->filename);
    DMSG("image digest:");
    for (int i = 0; i < digest_size[sbmeta_info->digest_scheme]; i++) {
        DMSG("%02X", sbmeta_info->digest[i]);
    }
    DMSG("\r\n");
    DMSG("\n\n");

    return 0;
}

static int sbmeta_field_verify(sbmeta_info_t *sbmeta_info, unsigned long img_src_addr)
{
    uint8_t digest_scheme = 0;
    uint8_t sign_scheme = 0;
    uint8_t is_encrypted = 0;
    img_header_t *phead = NULL;

    if (sbmeta_info == NULL) {
        return CMD_RET_FAILURE;
    }

    /* if image has secure header, check with sbmeta field */
    if (image_have_head(img_src_addr)) {
        phead = (img_header_t *)img_src_addr;
        digest_scheme = phead->digest_scheme;
        sign_scheme = phead->signature_scheme;
        is_encrypted = (phead->option_flag & 0x2) >> 1;
    }

    if (sbmeta_info->digest_scheme != digest_scheme) {
        EMSG("digest type %d is not expected: %d\r\n", digest_scheme, sbmeta_info->digest_scheme);
        return CMD_RET_FAILURE;
    }

    /* only support none, rsa1024, rsa2048, ecc256, ecc160, sm2 and reserved scheme */
    if (sbmeta_info->sign_scheme != sign_scheme) {
        EMSG("signature type %d is not expected: %d\r\n", sign_scheme, sbmeta_info->sign_scheme);
        return CMD_RET_FAILURE;
    }

    if (sbmeta_info->isencrypted != is_encrypted) {
        EMSG("encryption %d is not expected: %d\r\n", is_encrypted, sbmeta_info->isencrypted);
        return CMD_RET_FAILURE;
    }

    return 0;
}

static int check_digest(uint8_t *buffer, uint32_t buffer_size, uint8_t digest_scheme, uint8_t *digest)
{
    uint32_t len = 0;
    uint8_t sum[64];
    sc_sha_t sha;
    sc_sha_context_t ctx;
    int mode = 0;

    if (!buffer || digest_scheme > DIGEST_TYPE_NUM) {
        EMSG("wrong parameter\r\n");
        return CMD_RET_FAILURE;
    }

    if (digest_scheme == 0) {
        return 0;
    }
    mode = sha_idx2ctl[digest_scheme];

    if (sc_sha_init(&sha, 0) != 0) {
        EMSG("sha initialize failed\r\n");
        return CMD_RET_FAILURE;
    }

    if (sc_sha_start(&sha, &ctx, mode) != 0) {
        EMSG("sha start failed\r\n");
        return CMD_RET_FAILURE;
    }

    if (sc_sha_update(&sha, &ctx, buffer, buffer_size) != 0) {
        EMSG("sha update failed\r\n");
        return CMD_RET_FAILURE;
    }

    if (sc_sha_finish(&sha, &ctx, sum, &len) != 0) {
        EMSG("sha finish failed\r\n");
        return CMD_RET_FAILURE;
    }

	sc_sha_uninit(&sha);

    /* check digest value */
    if (memcmp(digest, sum, len) != 0) {
        EMSG("check digest failed\r\n");
        return CMD_RET_FAILURE;
    }

    return 0;
}

/* Verify image specified in sbmeta_info_t. The image has been loaded to memory before */
static int sbmeta_verify_image(uint32_t image_load_addr, sbmeta_info_t *sbmeta_info)
{
    uint32_t image_size = 0;
    const char *image_name;
    uint8_t image_type = sbmeta_info->image_type;
    uint8_t checksum_scheme = sbmeta_info->checksum_scheme;
    uint8_t *digest = sbmeta_info->digest;
    uint8_t is_encrypted = sbmeta_info->isencrypted;
    uint32_t security_level = env_get_hex("sbmeta_security_level", 3);
    uint32_t filesize = 0;
    char buf[64] = {0};

    /* check image_type to avoid array index out of bounds */
    if (image_type > IMAGE_TYPE_NUM || image_type < 0) {
        EMSG("image type is out of range\r\n");
        return CMD_RET_FAILURE;
    }
    image_name = image_name_s[image_type];

    /* check tee/tf version if needed */
#ifdef LIGHT_IMG_VERSION_CHECK_IN_BOOT
   if (image_have_head(image_load_addr) == 1) {
        if (image_type == T_TF) {
            IMSG("check TF version in boot \n");
            if (check_tf_version_in_boot(LIGHT_TF_FW_TMP_ADDR) != 0) {
                return CMD_RET_FAILURE;
            }
        }

        if (image_type == T_TEE) {
            IMSG("check TEE version in boot \n");
            if (check_tee_version_in_boot(LIGHT_TEE_FW_ADDR) != 0) {
                return CMD_RET_FAILURE;
            }
        }
   }
#endif

    /* start verifying images */
    IMSG("Process %s image verification ...\n", image_name);
    if (security_level == 3 || is_encrypted != 0) {
        if (verify_customer_image(image_type, image_load_addr) != 0) {
            return CMD_RET_FAILURE;
        }
    } else if (security_level == 2) {
        if (memcmp(digest, buf, 64) == 0) {
            EMSG("sbmeta info doesn't specify digest value in security level 2\r\n");
            return CMD_RET_FAILURE;
        }

        snprintf(buf, sizeof(buf), "ext4size mmc %x:%x %s", sbmeta_info->dev, sbmeta_info->part, sbmeta_info->filename);
        if (run_command(buf, 0) != 0) {
            EMSG("get file size error\r\n");
            return CMD_RET_FAILURE;
        }

        filesize = env_get_hex("filesize", 0);
        if (check_digest((uint8_t *)(uintptr_t)image_load_addr, filesize, checksum_scheme, digest) != 0) {
            return CMD_RET_FAILURE;
        }
    }

    /* move image headers always */
    if (image_have_head(image_load_addr) == 1) {
        image_size = get_image_size(image_load_addr);
        IMSG("%s image size: %d\n", image_name, image_size);
        if (image_size < 0) {
            EMSG("GET %s image size error\n", image_name);
            return CMD_RET_FAILURE;
        }
        if (image_type == T_TF) {
            memmove((void *)(uintptr_t)LIGHT_TF_FW_ADDR, (const void *)(uintptr_t)(image_load_addr + HEADER_SIZE), image_size);
        } else {
            memmove((void *)(uintptr_t)image_load_addr, (const void *)(uintptr_t)(image_load_addr + HEADER_SIZE), image_size);
        }
    } else {
        /* TF should be moved to LIGHT_TF_FW_ADDR all the cases*/
        if (image_type == T_TF) {
            /* while image_size is unknown, reload the image */
            run_command("ext4load mmc 0:3 0x0 trust_firmware.bin", 0);
        }
    }

    return 0;
}

static int light_sbmetaboot(int argc, char *const argv[])
{
    int count = 0;
    uint32_t sbmeta_size = 0;
    uint32_t info_addr = 0;
    uint32_t image_load_addr = 0;
    char cmd[64] = {0};
    sbmeta_info_t *sbmeta_info = NULL;

    /* Load sbmeta image to memory */
    snprintf(cmd, sizeof(cmd), "ext4load mmc $mmcdev:%x 0x%p %s", SBMETA_PART, (void *)(uintptr_t)LIGHT_SBMETA_ADDR, SBMETA_FILENAME);
    if (run_command(cmd, 0) != 0) {
        /* if sbmeta doesn't exist, do secboot by default */
        IMSG("SBMETA doesn't exist, go to verify tf/tee\r\n");

        /*
        * Verify tf and tee by command secboot.
        * Note that tf and tee has been loaded in "run bootcmd_load"
        */
        if (run_command("secboot", 0) != 0) {
            return CMD_RET_FAILURE;
        }

        return 0;
    }

    /* initialize crypto algorithm interfaces */
    if (csi_sec_init() != 0) {
        return CMD_RET_FAILURE;
    }

    /* Check and verify sbmeta image */
    if (image_have_head(LIGHT_SBMETA_ADDR) == 1) {
#ifdef LIGHT_IMG_VERSION_CHECK_IN_BOOT
		IMSG("check SBMETA version in boot \n");
		ret = check_sbmeta_version_in_boot(LIGHT_SBMETA_ADDR);
		if (ret != 0) {
			return CMD_RET_FAILURE;
		}
#endif
        IMSG("Process SBMETA image verification...\r\n");
        if (verify_customer_image(T_SBMETA, LIGHT_SBMETA_ADDR) != 0) {
            return CMD_RET_FAILURE;
        }

        sbmeta_size = get_image_size(LIGHT_SBMETA_ADDR);
        IMSG("sbmeta_size:%d\r\n", sbmeta_size);
        if (sbmeta_size != SBMETA_SIZE) {
            EMSG("SBMETA header is wrong! Size must equal to %d bytes!\r\n", SBMETA_SIZE);
            return CMD_RET_FAILURE;
        }
        /* move image headers always */
        memmove((void *)LIGHT_SBMETA_ADDR, (const void *)(LIGHT_SBMETA_ADDR + HEADER_SIZE), sbmeta_size);
    } else {
        /* if sbmeta image is not secure, reset */
        IMSG("SBMETA image must be with signature\r\n");
        return CMD_RET_FAILURE;
    }

    /* Parse sbmeta_info_t in image sbmeta, then load and verify specified images */
    info_addr = LIGHT_SBMETA_ADDR;
    for (count = 0; count < MAX_ENTRY_NUM; count++) {
        if (is_sbmeta_info(info_addr) == 0) {
            /* Dump and check sbmeta info */
            sbmeta_info = (sbmeta_info_t *)(uintptr_t)info_addr;
            if (dump_sbmeta_info(sbmeta_info) != 0) {
                return CMD_RET_FAILURE;
            }

            info_addr += ENTRY_SIZE;

            /*
             * If image_type != T_USER, load to address specified in light-c910.h;
             * otherwise, load to user-specified address.
            */
            if (sbmeta_info->image_type != T_USER) {
                image_load_addr = image_addrs[sbmeta_info->image_type];
            } else {
                image_load_addr = sbmeta_info->relocated_addr;
            }

            /*
            * Load image specified in sbmeta info
            * Note: only load images don't exist in env "bootcmd_load"
            */
            if (sbmeta_info->image_type == T_ROOTFS || sbmeta_info->image_type == T_USER) {
                snprintf(cmd, sizeof(cmd), "ext4load mmc %x:%x %p %s", sbmeta_info->dev,
                         sbmeta_info->part, \
                         (void *)(uintptr_t)image_load_addr, sbmeta_info->filename);
                if (run_command(cmd, 0) != 0) {
                    return CMD_RET_FAILURE;
                }
            }

            if (sbmeta_field_verify(sbmeta_info, image_load_addr) != 0) {
                return CMD_RET_FAILURE;
            }

            /* Check and verify user-specified image */
            if (sbmeta_verify_image(image_load_addr, sbmeta_info) != 0) {
                return CMD_RET_FAILURE;
            }
        } else {
            break;
        }
    }

    /* if sbmeta didn't specify images, reset */
    if (count == 0) {
        EMSG("SBMETA doesn't specify any images!\r\n");
        return CMD_RET_FAILURE;
    }

    /* Clear sbmeta buffer in memory */
    memset((void *)LIGHT_SBMETA_ADDR, 0, PLAIN_SBMETA_TEXT);
    return 0;
}

static int do_sbmetaboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    if (light_sbmetaboot(argc, argv) != 0) {
        EMSG("sbmetaboot failed\r\n");
        while (1);
        return CMD_RET_FAILURE;
    }
    return 0;
}

U_BOOT_CMD(
    sbmetaboot, CONFIG_SYS_MAXARGS, 1, do_sbmetaboot,
    "load and verify image sbmeta, then verify image files specified in sbmeta",
    ""
);
#endif
#endif
