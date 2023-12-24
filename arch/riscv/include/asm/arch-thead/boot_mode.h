/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#ifndef _ASM_BOOT_MODE_H
#define _ASM_BOOT_MODE_H

#define HEADER_SIZE   2048
#define PUBKEY_HEADER_SIZE	0x1000

typedef struct __attribute__ ((__packed__)) {
	uint32_t entry_point;
	uint32_t magic_num;
	uint32_t image_size;
	uint16_t head_version;
	uint16_t image_version;
	uint32_t image_checksum;
	uint64_t image_run_addr;
	uint16_t image_offset;
	uint8_t  digest_scheme;
	uint8_t  signature_scheme;
	uint32_t option_flag;
	uint16_t encrypt_type;
	uint16_t reserve0;
	uint32_t reserve[6];
} img_header_t;  /*64 bytes*/

typedef enum image_type {
	T_DTB = 0,
	T_KRLIMG = 1,
	T_SBI = 2,
	T_AON = 3,
	T_ROOTFS = 4,
	T_TF = 2,
	T_TEE = 5,
	T_UBOOT = 6,
	T_USER = 7,
	T_SBMETA = 8,
} img_type_t;

static const char header_magic[4] = {'T', 'H', 'E', 'D'};

int csi_sec_init(void);
int image_have_head(unsigned long img_src_addr);
int get_image_version(unsigned long img_src_addr);
int get_image_size(unsigned long img_src_addr);
int verify_customer_image(img_type_t type, long addr);
#endif
