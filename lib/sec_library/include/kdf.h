/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef __KDF_H__
#define __KDF_H__
#ifdef SEC_LIB_VERSION
#include "drv/aes.h"
#include "drv/sm4.h"
#include "drv/common.h"
#else
#include "aes.h"
#include "sm4.h"
#include "common.h"
#endif

#include <stdint.h>

typedef enum {
        KDF_ROOT_CV_UNIQUE_KEY,
        KDF_ROOT_CV_COMMON_KEY,
        KDF_ROOT_USER_UNIQUE_KEY,
        KDF_ROOT_USER_COMMON_KEY,
        KDF_ROOT_KEY_MAX,
} csi_kdf_root_key_t;

typedef enum {
        KDF_DERIVED_DFT_CHALLENGE_EK,
        KDF_DERIVED_C910TJTAG_CHALLENGE_EK,
        KDF_DERIVED_E902JTAG_CHALLENGE_EK,
        KDF_DERIVED_IMAGE_EK,
        KDF_DERIVED_SECURE_STORAGE_EK1,
        KDF_DERIVED_SECURE_STORAGE_EK2,
        KDF_DERIVED_SECURE_STORAGE_EK3,
        KDF_DERIVED_SECURE_STORAGE_EK4,
        KDF_DERIVED_SECURE_STORAGE_EK5,
        KDF_DERIVED_SECURE_STORAGE_EK6,
        KDF_DERIVED_SECURE_STORAGE_EK7,
        KDF_DERIVED_SECURE_STORAGE_EK8,
        KDF_DERIVED_SECURE_STORAGE_EK9,
        KDF_DERIVED_SECURE_STORAGE_EK10,
        KDF_DERIVED_SECURE_STORAGE_EK11,
        KDF_DERIVED_SECURE_STORAGE_EK12,
        KDF_DERIVED_SECURE_STORAGE_EK13,
        KDF_DERIVED_SECURE_STORAGE_EK14,
        KDF_DERIVED_SECURE_STORAGE_EK15,
        KDF_DERIVED_SECURE_STORAGE_EK16,
        KDF_DERIVED_RPMB_ACCESS_EK,
        KDF_DERIVED_MAX,
} csi_kdf_derived_key_t;

typedef enum {
        KDF_KEY_TYPE_AES_256,
        KDF_KEY_TYPE_AES_192,
        KDF_KEY_TYPE_AES_128,
        KDF_KEY_TYPE_SM4,
        KDF_KEY_TYPE_TDES_192,
        KDF_KEY_TYPE_TDES_128,
        KDF_KEY_TYPE_DES,
	/* for rpmb, str */
/* 	KDF_KEY_TYPE_HMAC_SHA256,
 */
        KDF_KEY_TYPE_MAX,
} csi_kdf_key_type_t;

typedef struct key_attr_tag {
        uint8_t len_in_byte;
        uint8_t valid;
        uint32_t offset;
} csi_kdf_key_attr_t;

/**
\brief KDF Ctrl Block
*/
typedef struct {
        union {
                csi_aes_t *aes;
                csi_sm4_t *sm4;
        };
        csi_kdf_key_type_t type;
} csi_kdf_key_handle_t;

/**
\brief KDF Ctrl Block
*/
typedef struct {
        csi_dev_t dev;
        void *priv;
} csi_kdf_t;

/**
  \brief       kdf initialiez.
  \param[in]   kdf    Handle to operate.
  \param[in]   idx    Device id.
  \return      error code
*/
csi_error_t csi_kdf_init(csi_kdf_t *kdf, uint32_t idx);

/**
  \brief       kdf uninitialiez.
  \param[in]   kdf    Handle to operate
*/
void csi_kdf_uninit(csi_kdf_t *kdf);

/**
  \brief       Derive key function.
  \param[in]   kdf    Handle to operate.
  \param[in]   rkey derive source root key type.
  \param[in]   dkey derived key type from root key.
  \return      error code
*/
csi_error_t csi_kdf_derived_key(csi_kdf_t *kdf, csi_kdf_root_key_t rkey,
                                csi_kdf_derived_key_t dkey);

/**
  \brief       Destroy derived key in storage.
  \param[in]   kdf    Handle to operate.
  \param[in]   dkey derived key type.
  \return      error code
*/
csi_error_t csi_kdf_destory_key(csi_kdf_t *kdf, csi_kdf_derived_key_t dkey);
// xiaoxia: keyram layout config macro

/**
  \brief       Set key to algorithim engine.
  \param[in]   kdf    Handle to operate.
  \param[in]   handle    Handle to cipher.
  \param[in]   dkey derived key type.
  \return      error code
*/
csi_error_t csi_kdf_set_key(csi_kdf_t *kdf, csi_kdf_key_handle_t *handle,
                            csi_kdf_derived_key_t dkey);

/**
  \brief       Clear key in algorithim engine.
  \param[in]   kdf    Handle to operate.
  \param[in]   dkey derived key type.
  \return      error code
*/
csi_error_t csi_kdf_clear_key(csi_kdf_t *kdf, csi_kdf_derived_key_t dkey);

/**
  \brief       Get key attribute.
  \param[in]   kdf    Handle to operate.
  \param[in]   dkey derived key type.
  \param[out]  attr     Buffer to get attribute.
  \return      error code
*/
csi_error_t csi_kdf_get_key_attr(csi_kdf_t *kdf, csi_kdf_derived_key_t dkey,
                                 csi_kdf_key_attr_t *attr);


/**
  \brief       kdf generate hmac key.
  \param[in]   kdf    Handle to operate
*/
csi_error_t csi_kdf_gen_hmac_key(uint8_t* key,uint32_t * length);

#endif
