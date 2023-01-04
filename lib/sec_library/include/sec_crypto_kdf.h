/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef __SC_KDF_H__
#define __SC_KDF_H__
#include "sec_crypto_errcode.h"
#include "sec_crypto_aes.h"
#include "sec_crypto_sm4.h"
#include "sec_crypto_mac.h"
#include <stdint.h>

typedef enum {
	SC_KDF_DERIVED_DFT_CHALLENGE_EK,
	SC_KDF_DERIVED_C910TJTAG_CHALLENGE_EK,
	SC_KDF_DERIVED_E902JTAG_CHALLENGE_EK,
	SC_KDF_DERIVED_IMAGE_EK,
	SC_KDF_DERIVED_SECURE_STORAGE_EK1,
	SC_KDF_DERIVED_SECURE_STORAGE_EK2,
	SC_KDF_DERIVED_SECURE_STORAGE_EK3,
	SC_KDF_DERIVED_SECURE_STORAGE_EK4,
	SC_KDF_DERIVED_SECURE_STORAGE_EK5,
	SC_KDF_DERIVED_SECURE_STORAGE_EK6,
	SC_KDF_DERIVED_SECURE_STORAGE_EK7,
	SC_KDF_DERIVED_SECURE_STORAGE_EK8,
	SC_KDF_DERIVED_SECURE_STORAGE_EK9,
	SC_KDF_DERIVED_SECURE_STORAGE_EK10,
	SC_KDF_DERIVED_SECURE_STORAGE_EK11,
	SC_KDF_DERIVED_SECURE_STORAGE_EK12,
	SC_KDF_DERIVED_SECURE_STORAGE_EK13,
	SC_KDF_DERIVED_SECURE_STORAGE_EK14,
	SC_KDF_DERIVED_SECURE_STORAGE_EK15,
	SC_KDF_DERIVED_SECURE_STORAGE_EK16,
	SC_KDF_DERIVED_RPMB_ACCESS_EK,
	SC_KDF_DERIVED_MAX,
} sc_kdf_derived_key_t;

typedef enum {
	SC_KDF_KEY_TYPE_AES_256,
	SC_KDF_KEY_TYPE_AES_192,
	SC_KDF_KEY_TYPE_AES_128,
	SC_KDF_KEY_TYPE_SM4,
	SC_KDF_KEY_TYPE_TDES_192,
	SC_KDF_KEY_TYPE_TDES_128,
	SC_KDF_KEY_TYPE_DES,
	/* for rpmb, str */
/* 	SC_KDF_KEY_TYPE_HMAC_SHA256,
 */
	SC_KDF_KEY_TYPE_MAX,
} sc_kdf_key_type_t;

/**
\brief KDF Ctrl Block
*/
typedef struct {
	union {
		sc_aes_t *aes;
		sc_sm4_t *sm4;
		sc_mac_t *mac;

	};
	sc_kdf_key_type_t type;
} sc_kdf_key_handle_t;

/**
\brief KDF Ctrl Block
*/
typedef struct {
	void *priv;
} sc_kdf_t;

/**
  \brief       kdf initialiez.
  \param[in]   kdf    Handle to operate.
  \param[in]   idx    Device id.
  \return      error code
*/
uint32_t sc_kdf_init(sc_kdf_t *kdf, uint32_t idx);

/**
  \brief       kdf uninitialiez.
  \param[in]   kdf    Handle to operate
*/
void sc_kdf_uninit(sc_kdf_t *kdf);

/**
  \brief       Set key to algorithim engine.
  \param[in]   handle    Handle to cipher.
  \param[in]   kdf    Handle to operate.
  \param[in]   dkey derived key type.
  \return      error code
*/
uint32_t sc_kdf_set_key(sc_kdf_t *kdf, sc_kdf_key_handle_t *handle,
			  sc_kdf_derived_key_t dkey);

#endif
