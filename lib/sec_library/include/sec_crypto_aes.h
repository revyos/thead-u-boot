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
#ifndef _SC_AES_H_
#define _SC_AES_H_

#include "sec_include_config.h"
#include <stdint.h>
#include "sec_crypto_errcode.h"

#ifdef CONFIG_SYSTEM_SECURE
#ifdef SEC_LIB_VERSION
#include <drv/aes.h>
#else
#include "aes.h"
#endif
#endif

#ifdef CONFIG_SEC_CRYPTO_AES_SW
#include "crypto_aes.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SC_AES_KEY_LEN_BITS_128 = 0U, ///< 128 Data bits
    SC_AES_KEY_LEN_BITS_192,      ///< 192 Data bits
    SC_AES_KEY_LEN_BITS_256       ///< 256 Data bits
} sc_aes_key_bits_t;

/**
\brief AES Ctrl Block
*/
typedef struct {
#ifdef CONFIG_SYSTEM_SECURE
#ifdef CONFIG_CSI_V1
    aes_handle_t  handle;
    unsigned char key[32];
    unsigned int  key_len;
#endif
#ifdef CONFIG_CSI_V2
    csi_aes_t     csi_aes;
    //unsigned char sc_ctx[SC_AES_CTX_SIZE];
#endif
#endif
#if defined(CONFIG_TEE_CA)
    unsigned char key[32];
    unsigned int  key_len;
#endif
#if defined(CONFIG_SEC_CRYPTO_AES_SW)
    sc_mbedtls_aes_context aes_ctx;
#endif
    //void *ctx;
} sc_aes_t;

// Function documentation
/**
  \brief       Initialize AES Interface. Initializes the resources needed for the AES interface
  \param[in]   aes    operate handle
  \param[in]   idx    device id
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_init(sc_aes_t *aes, uint32_t idx);

/**
  \brief       De-initialize AES Interface. stops operation and releases the software resources used by the interface
  \param[in]   aes    handle to operate
  \return      None
*/
void sc_aes_uninit(sc_aes_t *aes);

/**
  \brief       Set encrypt key
  \param[in]   aes        handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref sc_aes_key_bits_t
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_set_encrypt_key(sc_aes_t *aes, void *key, sc_aes_key_bits_t key_len);

/**
  \brief       Set decrypt key
  \param[in]   aes        handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref sc_aes_key_bits_t
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_set_decrypt_key(sc_aes_t *aes, void *key, sc_aes_key_bits_t key_len);

/**
  \brief       Aes ecb encrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_ecb_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size);

/**
  \brief       Aes ecb decrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_ecb_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size);

/**
  \brief       Aes cbc encrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cbc_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cbc decrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cbc_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cfb1 encrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cfb1_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cfb1 decrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cfb1_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cfb8 encrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cfb8_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cfb8 decrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cfb8_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cfb128 decrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cfb128_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes cfb128 encrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_cfb128_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);
/**
  \brief       Aes ofb encrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[in]  key_len key bits
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_ofb_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes ofb decrypt
  \param[in]   aes     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[in]  key_len key bits
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_ofb_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes ctr encrypt
  \param[in]   aes              handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_ctr_encrypt(sc_aes_t *aes, void *in, void *out, uint32_t size,void *iv);

/**
  \brief       Aes ctr decrypt
  \param[in]   aes              handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vecotr
  \return      error code \ref uint32_t
*/
uint32_t sc_aes_ctr_decrypt(sc_aes_t *aes, void *in, void *out, uint32_t size,void *iv);

/**
  \brief       Aes gcm encrypt
  \param[in]   dev_aes          dev_aes handle to operate
  \param[in]   in               Pointer to the Source data.
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vector
  \return      error code \ref csi_error_t
*/
uint32_t sc_aes_gcm_encrypt(sc_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv);

/**
  \brief       Aes gcm decrypt
  \param[in]   dev_aes          dev_aes handle to operate
  \param[in]   in               Pointer to the Source data.
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vecotr
  \return      error code \ref csi_error_t
*/
uint32_t sc_aes_gcm_decrypt(sc_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv);

/**
  \brief       Aes gcm encrypt
  \param[in]   dev_aes          dev_aes handle to operate
  \param[in]   in               Pointer to the Source data.
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vector
  \param[in]   tag_out          tag output ,parse null if not needed
  \return      error code \ref csi_error_t
*/
uint32_t sc_aes_ccm_encrypt(sc_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv, uint8_t* tag_out);

/**
  \brief       Aes gcm decrypt
  \param[in]   dev_aes          dev_aes handle to operate
  \param[in]   in               Pointer to the Source data.
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vecotr
  \param[in]   tag_out tag output,parse null if not needed
  \return      error code \ref csi_error_t
*/
uint32_t sc_aes_ccm_decrypt(sc_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv, uint8_t* tag_out);

void sc_aes_dma_enable(sc_aes_t *aes, uint8_t en);

#ifdef __cplusplus
}
#endif

#endif /* _SC_AES_H_ */
