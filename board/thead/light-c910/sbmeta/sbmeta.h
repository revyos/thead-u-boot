/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#ifndef _LIGHT_SBMETA_H
#define _LIGHT_SBMETA_H

#include "common.h"
#include "command.h"
#include <asm/arch-thead/boot_mode.h>

#define MAX_NAME_SIZE       32
#define MAX_DIGEST_SIZE     64
#define SBMETA_MAGIC        0x544D4253  /* = {'S', 'B', 'M', 'T'} */

#if CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_A) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_VAL_B) || CONFIG_IS_ENABLED(LIGHT_SEC_BOOT_WITH_VERIFY_LPI4A)
#define LIGHT_SBMETA_ADDR   0x10000000
#endif
#define SBMETA_DEV          0
#define SBMETA_PART         6
#define ENTRY_SIZE          128
#define PLAIN_SBMETA_TEXT   4096
#define SBMETA_SIZE         4736  /* 4K SMBETA image + 640 footer */
#define MAX_ENTRY_NUM       PLAIN_SBMETA_TEXT / ENTRY_SIZE  /* 4K/128=32 */
#define IMAGE_TYPE_NUM      7
#define DIGEST_TYPE_NUM     8
#define SIGN_TYPE_NUM       6
#define T_USER              7
#define SBMETA_FILENAME     "sbmeta.bin"

typedef struct {
    int         magiccode;
    uint8_t     dev;
    uint8_t     part;
    uint8_t     image_type;
    uint8_t     digest_scheme;
    uint8_t     sign_scheme;
    uint8_t     isencrypted;
    uint8_t     medium_type;
    uint8_t     reserve0;
    char        filename[MAX_NAME_SIZE];
    uint8_t     digest[MAX_DIGEST_SIZE];
    uint32_t    relocated_addr;
    uint32_t    reserved[4];
} sbmeta_info_t;

#endif
