/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     seccrypt_aes.h
 * @brief    Header File for AES
 * @version  V1.0
 * @date     20. Jul 2020
 * @model    aes
 ******************************************************************************/
#ifndef _SC_GCM_H_
#define _SC_GCM_H_

#include "sec_include_config.h"
#include <stdint.h>
#include "sec_crypto_errcode.h"

#ifdef CONFIG_SEC_CRYPTO_AES_SW
#include "crypto_aes.h"
#include <sec_crypto_aes.h>
#endif

#ifdef CONFIG_SEC_CRYPTO_GCM_SW
#include "crypto_gcm.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*Function documentation*/
/**
  \brief       Initialize AES Interface. Initializes the resources needed for the AES interface
  \param[in]   aes    operate handle
  \param[in]   idx    device id
  \return      error code \ref uint32_t
*/
uint32_t sc_gcm_init(sc_aes_t *aes, uint32_t idx);

/**
  \brief       De-initialize AES Interface. stops operation and releases the software resources used by the interface
  \param[in]   aes    handle to operate
  \return      None
*/
void sc_gcm_uninit(sc_aes_t *aes);

/**
  \brief       Set encrypt key
  \param[in]   aes        handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref sc_aes_key_bits_t
  \return      error code \ref uint32_t
*/
uint32_t sc_gcm_set_encrypt_key(sc_aes_t *aes, void *key, sc_aes_key_bits_t key_len);

/**
  \brief       Set decrypt key
  \param[in]   aes        handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref sc_aes_key_bits_t
  \return      error code \ref uint32_t
*/
uint32_t sc_gcm_set_decrypt_key(sc_aes_t *aes, void *key, sc_aes_key_bits_t key_len);

/**
  \brief       gcm encrypt data
  \param[in]   aes        aes handle to operate
  \param[in]   length     Length of plaintext buffer
  \param[in]   iv_len     Length of iv buffer
  \param[in]   add        Pointer to the add buf, can be NULL
  \param[in]   add_len    Length of add buffer, can ben zero
  \param[in]   input      Pointer to the plaintext buf
  \param[in]   output     Pointer to the cipher buf
  \param[in]   tag_len    Length of tag buffer
  \param[in]   tag        Pointer to the tag buf
  \return      error code \ref uint32_t
*/
uint32_t sc_gcm_encrypt_and_tag(sc_aes_t *aes, uint32_t length,
                       const void *iv,
                       uint32_t iv_len,
                       const void *add,
                       uint32_t add_len,
                       const void *input,
                       void *output,
                       uint32_t tag_len,
                       void *tag);

/**
  \brief       Aes gcm decrypt
  \param[in]   aes        aes handle to operate
  \param[in]   length     Length of cipher buffer
  \param[in]   iv         Pointer to the iv buf
  \param[in]   iv_len     Length of iv buffer
  \param[in]   add        Pointer to the add buf, can be NULL
  \param[in]   add_len    Length of add buffer, can ben zero
  \param[in]   input      Pointer to the cipher buf
  \param[in]   output     Pointer to the plaintext buf
  \param[in]   tag_len    Length of tag buffer
  \param[in]   tag        Pointer to the tag buf
  \return      error code \ref uint32_t
*/
uint32_t sc_gcm_auth_decrypt(sc_aes_t *aes, uint32_t length,
                      const void *iv,
                      uint32_t iv_len,
                      const void *add,
                      uint32_t add_len,
                      const void *tag,
                      uint32_t tag_len,
                      const void *input,
                      void *output);

#ifdef __cplusplus
}
#endif

#endif /* _SC_AES_H_ */
