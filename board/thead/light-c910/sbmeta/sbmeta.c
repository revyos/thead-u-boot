/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#include "sbmeta.h"

#define NO_DEBUG 0
#if NO_DEBUG
#define print_info(fmt, args...)
#else
#define print_info(fmt, args...)    printf(fmt, ##args)
#endif

#if CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_A) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_B) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_LPI4A)
#if CONFIG_IS_ENABLED(LIGHT_SEC_UPGRADE)
/* digest_size corresponding to digest_scheme specified in sbmeta_info_t */
static const int digest_size[] = {0, 20, 16, 28, 32, 48, 64, 32, 64};
static const char* image_name_s[] = {
    "dtb", "kernel", "tf", "aon", "rootfs", "tee", "uboot", "user"
};

static const uint32_t image_addrs[] = {
    LIGHT_DTB_ADDR,
    LIGHT_KERNEL_ADDR,
    LIGHT_TF_FW_TMP_ADDR,
    LIGHT_AON_FW_ADDR,
    LIGHT_ROOTFS_ADDR,
    LIGHT_TEE_FW_ADDR,
    CONFIG_SYS_TEXT_BASE,
};

static int is_sbmeta_info(uint32_t entry_src_addr)
{
    uint32_t *buffer = (uint32_t *)entry_src_addr;

    /* sbmeta_info_t entry should start with magic code 'S''B''M''T' */
    if (*buffer != SBMETA_MAGIC) {
        return -1;
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
        print_info("Error: medium type %s is not supported now\r\n");
        return CMD_RET_FAILURE;
    }

    /* only support dtb, krlimg/tf, sbi, aon, rootfs, tee, uboot and user-defined type */
    if (sbmeta_info->image_type > IMAGE_TYPE_NUM || sbmeta_info->image_type < 0) {
        print_info("Error: image type is out of range\r\n");
        return CMD_RET_FAILURE;
    }

    /* only support none, sha1, md5, sha224, sha256, sha384, sha512, sm3 and reserved scheme */
    if (sbmeta_info->digest_scheme > DIGEST_TYPE_NUM || sbmeta_info->digest_scheme < 0) {
        print_info("Error: digest type is out of range\r\n");
        return CMD_RET_FAILURE;
    }

    /* only support none, rsa1024, rsa2048, ecc256, ecc160, sm2 and reserved scheme */
    if (sbmeta_info->sign_scheme > SIGN_TYPE_NUM || sbmeta_info->sign_scheme < 0) {
        print_info("Error: signature type is out of range\r\n");
        return CMD_RET_FAILURE;
    }

    /* DTB, TF, TEE, Kernel will be loaded from default partitions specified in env */
    if (sbmeta_info->image_type != T_ROOTFS && sbmeta_info->image_type != T_USER) {
        print_info("Image has been loaded\r\n");
    }

    /* dump sbmeta_info_t */
    print_info("image medium type: %d\n", sbmeta_info->medium_type);
    print_info("image load part: mmc %d:%d\n", sbmeta_info->dev, sbmeta_info->part);
    print_info("image type: %d \n", sbmeta_info->image_type);
    print_info("image digest scheme: %d\n", sbmeta_info->digest_scheme);
    print_info("image sign scheme: %d\n", sbmeta_info->sign_scheme);
    print_info("image enable encryption: %s\n", sbmeta_info->isencrypted ? "en" : "dis");
    print_info("image file name: %s\n", sbmeta_info->filename);
    print_info("image digest:");
    for (int i = 0; i < digest_size[sbmeta_info->digest_scheme]; i++) {
        print_info("%02X", sbmeta_info->digest[i]);
    }
    print_info("\r\n");
    
    return 0;
}


/* Verify image specified in sbmeta_info_t. The image has been loaded to memory before */
static int sbmeta_verify_image(uint32_t image_load_addr, uint8_t image_type)
{
    uint32_t image_size = 0;
    char *image_name = NULL;
    
    /* check image_type to avoid array index out of bounds */
    if (image_type > IMAGE_TYPE_NUM || image_type < 0) {
        print_info("Error: image type is out of range\r\n");
        return CMD_RET_FAILURE;
    }
    image_name = image_name_s[image_type];

    /* if image has secure header, do verification. otherwise */
    if (image_have_head(image_load_addr) == 1) {
        /* check tee/tf version if needed */
#ifdef LIGHT_IMG_VERSION_CHECK_IN_BOOT
        if (image_type == T_TF) {
            print_info("check TF version in boot \n");
            if (check_tf_version_in_boot(LIGHT_TF_FW_TMP_ADDR) != 0) {
                return CMD_RET_FAILURE;
            }
        }

        if (image_type == T_TEE) {
            print_info("check TEE version in boot \n");
            if (check_tee_version_in_boot(LIGHT_TEE_FW_ADDR) != 0) {
                return CMD_RET_FAILURE;
            }
        }
#endif

        /* start verifying images */
        print_info("Process %s image verification ...\n", image_name);
        dump_image_header_info(image_load_addr);
        if (image_type == T_UBOOT) {
            if (csi_sec_uboot_image_verify(image_load_addr, image_load_addr - PUBKEY_HEADER_SIZE) != 0) {
                print_info("Image(%s) is verified fail, Please go to check!\n\n", image_name);
                return CMD_RET_FAILURE;
            }
        } else {
            if (csi_sec_custom_image_verify(image_load_addr, UBOOT_STAGE_ADDR) != 0) {
                print_info("Image(%s) is verified fail, Please go to check!\n\n", image_name);
                return CMD_RET_FAILURE;
            }
        }
        
        image_size = get_image_size(image_load_addr);
        print_info("%s image size: %d\n", image_name, image_size);
        if (image_size < 0) {
            print_info("GET %s image size error\n", image_name);
            return CMD_RET_FAILURE;
        }
        
        /* move image headers always */
        if (image_type == T_TF) {
            memmove((void *)LIGHT_TF_FW_ADDR, (const void *)(image_load_addr + HEADER_SIZE), image_size);
        } else {
            memmove((void *)image_load_addr, (const void *)(image_load_addr + HEADER_SIZE), image_size);
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
    char *image_name = NULL;
    sbmeta_info_t *sbmeta_info = NULL;

    /* Load sbmeta image to memory */
    snprintf(cmd, sizeof(cmd), "ext4load mmc %x:%x 0x%p %s", SBMETA_DEV, SBMETA_PART, LIGHT_SBMETA_ADDR, SBMETA_FILENAME);
    if (run_command(cmd, 0) != 0) {
        /* if sbmeta doesn't exist, do secboot by default */
        print_info("SBMETA doesn't exist, go to verify tf/tee\r\n");

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
        print_info("Process SBMETA image verification...\r\n");
        dump_image_header_info(LIGHT_SBMETA_ADDR);
        if (csi_sec_custom_image_verify(LIGHT_SBMETA_ADDR, UBOOT_STAGE_ADDR) != 0) {
            print_info("SBMETA is verified fail, Please go to check!\r\n");
            return CMD_RET_FAILURE;
        }

        sbmeta_size = get_image_size(LIGHT_SBMETA_ADDR);
        print_info("sbmeta_size:%d\r\n", sbmeta_size);
        if (sbmeta_size != SBMETA_SIZE) {
            print_info("Error: SBMETA header is wrong! Size must equal to %d bytes!\r\n", SBMETA_SIZE);
            return CMD_RET_FAILURE;
        }
        /* move image headers always */
        memmove((void *)LIGHT_SBMETA_ADDR, (const void *)(LIGHT_SBMETA_ADDR + HEADER_SIZE), sbmeta_size);
    } else {
        /* if sbmeta image is not secure, reset */
        print_info("Error: SBMETA image must be with signature\r\n");
        return CMD_RET_FAILURE;
    }

    /* Parse sbmeta_info_t in image sbmeta, then load and verify specified images */
    info_addr = LIGHT_SBMETA_ADDR;
    for (count = 0; count < MAX_ENTRY_NUM; count++) {
        if (is_sbmeta_info(info_addr) == 0) {
            /* Dump and check sbmeta info */
            sbmeta_info = (sbmeta_info_t*)info_addr;
            if (dump_sbmeta_info(sbmeta_info) != 0) {
                return CMD_RET_FAILURE;
            }

            image_name = image_name_s[sbmeta_info->image_type];
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
                         image_load_addr, sbmeta_info->filename);
                if (run_command(cmd, 0) != 0) {
                    return CMD_RET_FAILURE;
                }
            }

            /* Check and verify user-specified image */
            if (sbmeta_verify_image(image_load_addr, sbmeta_info->image_type) != 0) {
                return CMD_RET_FAILURE;
            }
        } else {
            break;
        }
    }

    /* if sbmeta didn't specify images, reset */
    if (count == 0) {
        print_info("Error: SBMETA doesn't specify any images!\r\n");
        return CMD_RET_FAILURE;
    }

    /* Clear sbmeta buffer in memory */
    memset((void *)LIGHT_SBMETA_ADDR, 0, PLAIN_SBMETA_TEXT);
    return 0;
}

static int do_sbmetaboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    if (light_sbmetaboot(argc, argv) != 0) {
        run_command("reset", 0);
        return -1;
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
