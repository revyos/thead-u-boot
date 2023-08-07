/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#ifndef __CSI_EFUSE_API_H__
#define __CSI_EFUSE_API_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SECURE_BOOT_DIS	= 0x5a5a5a5a,
	SECURE_BOOT_EN = ~(SECURE_BOOT_DIS),
} sboot_st_t;


typedef enum {
	IMAGE_ENCRYPT_DIS	= 0x5a5a5a5a,
	IMAGE_ENCRYPT_EN = ~(IMAGE_ENCRYPT_DIS),
} img_encrypt_st_t;

int csi_efuse_api_init(void);
void csi_efuse_api_uninit(void);

int csi_efuse_get_secure_boot_st(sboot_st_t *sboot_st);

int csi_efuse_get_digest_signature_scheme(uint32_t *digest_scheme, uint32_t *signature_scheme);

int csi_efuse_get_bl2_encrypt_st(img_encrypt_st_t *encrypt_st);

int csi_efuse_set_bl2_encrypt_st(img_encrypt_st_t encrypt_st);

int csi_efuse_get_bl3_encrypt_st(img_encrypt_st_t *encrypt_st);

int csi_efuse_set_bl3_encrypt_st(img_encrypt_st_t encrypt_st);

int csi_efuse_get_bl1_version(uint32_t *version);

int csi_efuse_set_bl1_version(uint32_t version);

int csi_efuse_get_bl2_version(uint32_t *version);

int csi_efuse_set_bl2_version(uint32_t version);

int csi_efuse_read_raw(uint32_t addr, void *data, uint32_t cnt);

int csi_efuse_write_raw(uint32_t addr, const void *data, uint32_t cnt);

int csi_efuse_get_lc(int *lc);

#ifdef __cplusplus
}
#endif

#endif	/* __CSI_EFUSE_API_H__ */
