/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     keyram.h
 * @brief    CSI Source File for kdf Driver
 * @version  V1.0
 * @date     12. MAR 2021
 ******************************************************************************/

#include "kdf.h"
#include <stdio.h>
#include <string.h>

#define KDF_CON 0x000
#define KDF_KEYTRANSFER 0x004
#define KDF_KEYINDEX 0x008
#define KDF_INTCON 0x00C
#define KDF_ZONE1SADDR 0x020
#define KDF_ZONE2SADDR 0x030
#define KDF_ZONE3SADDR 0x040
#define KDF_RAMENC_KEY0H 0x058
#define KDF_RAMENC_KEY0L 0x05c
#define KDF_RAMENC_KEY1H 0x060
#define KDF_RAMENC_KEY1L 0x064
#define KDF_RAMENC_KEY2H 0x068
#define KDF_RAMENC_KEY2L 0x06c
#define KDF_RAMENC_KEY3H 0x070
#define KDF_RAMENC_KEY3L 0x074
#define KDF_MASK_RNG0 0x080
#define KDF_MASK_RNG1 0x084
#define KDF_MASK_RNG2 0x088
#define KDF_MASK_RNG3 0x08c
#define KDF_MASK_RNG4 0x090
#define KDF_MASK_RNG5 0x094
#define KDF_MASK_RNG6 0x098
#define KDF_MASK_RNG7 0x09c
#define KDF_STA 0x0a0
#define KDF_IPID 0x0fc
#define KDF_RAM 0x8000

/**
 * @brief keyram init.
 *
 * @return uint32_t
 */
uint32_t keyram_init(void);

/**
 * @brief keyram set key.
 *
 * @param key
 * @param len
 * @return uint32_t
 */
uint32_t keyram_set_key(csi_kdf_derived_key_t key, uint32_t len);

/**
 * @brief get key addr in keyram.
 *
 * @param key
 * @param addr
 * @return uint32_t
 */
uint32_t keyram_get_key_addr(csi_kdf_derived_key_t key, uint64_t *addr);

/**
 * @brief Clear key in keyram.
 *
 * @return uint32_t
 */
uint32_t keyram_clear(void);

/**
 * @brief Uninit. This function will lock KDF.
 */
void keyram_uninit(void);
