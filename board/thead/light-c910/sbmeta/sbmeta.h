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
#define SBMETA_PART         5
#define ENTRY_SIZE          128
#define PLAIN_SBMETA_TEXT   4096
#define SBMETA_SIZE         4736  /* 4K SMBETA image + 640 footer */
#define MAX_ENTRY_NUM       PLAIN_SBMETA_TEXT / ENTRY_SIZE  /* 4K/128=32 */
#define IMAGE_TYPE_NUM      7
#define DIGEST_TYPE_NUM     8
#define SIGN_TYPE_NUM       6
#define SBMETA_FILENAME     "sbmeta.bin"

#define SBMETA_SECURITY_LEVEL_H      3   /* verify signature and hash */
#define SBMETA_SECURITY_LEVEL_M      2   /* verify checksum */
#define SBMETA_SECURITY_LEVEL_L      1   /* no verification */

#endif
