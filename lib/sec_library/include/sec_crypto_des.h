/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypt0_des.h
 * @brief    Header File for DES
 * @version  V1.0
 * @date     24. Oct 2022
 * @model    des
 ******************************************************************************/
#ifndef _SC_DES_H_
#define _SC_DES_H_

#include "sec_include_config.h"
#include <stdint.h>
#include "sec_crypto_errcode.h"

#ifdef CONFIG_SYSTEM_SECURE
#ifdef SEC_LIB_VERSION
#include <drv/des.h>
#else
#include "des.h"
#endif
#endif

#ifdef CONFIG_SEC_CRYPTO_DES_SW
#include "crypto_des.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
\brief DES data transfer mode config
*/
typedef enum {
    SC_DES_SLAVE_MODE = 0U,         ///< slave mode
    SC_DES_DMA_MODE,                ///< dma mode
} sc_des_trans_mode_t;

/**
\brief DES key-len-bits type
*/
typedef enum {
    SC_DES_KEY_LEN_BITS_64 = 0U,    ///< 64 Data bits
    SC_DES_KEY_LEN_BITS_128,        ///< 128 Data bits
    SC_TDES_KEY_LEN_BITS_192,       ///< 192 Data bits
} sc_des_key_bits_t;

/**
\brief DES Ctrl Block
*/
typedef struct {
#ifdef CONFIG_SYSTEM_SECURE
#ifdef CONFIG_CSI_V1
    des_handle_t  handle;
    unsigned char key[32];
    unsigned int  key_len;
#endif
#ifdef CONFIG_CSI_V2
    csi_des_t     csi_des;
    //unsigned char sc_ctx[SC_DES_CTX_SIZE];
#endif
#endif
#if defined(CONFIG_TEE_CA)
    unsigned char key[32];
    unsigned int  key_len;
#endif
#if defined(CONFIG_SEC_CRYPTO_DES_SW)
    sc_mbedtls_des_context des_ctx;
#endif
    //void *ctx;
} sc_des_t;

// Function documentation
/**
  \brief       Initialize DES Interface. Initializes the resources needed for the DES interface
  \param[in]   des    operate handle
  \param[in]   idx    device id
  \return      error code \ref uint32_t
*/
uint32_t sc_des_init(sc_des_t *des, uint32_t idx);

/**
  \brief       De-initialize DES Interface. stops operation and releases the software resources used by the interface
  \param[in]   des    handle to operate
  \return      None
*/
void sc_des_uninit(sc_des_t *des);

/**
  \brief       Set encrypt key
  \param[in]   des        handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref sc_des_key_bits_t
  \return      error code \ref uint32_t
*/
uint32_t sc_des_set_encrypt_key(sc_des_t *des, void *key, sc_des_key_bits_t key_len);

/**
  \brief       Set decrypt key
  \param[in]   des        handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref sc_des_key_bits_t
  \return      error code \ref uint32_t
*/
uint32_t sc_des_set_decrypt_key(sc_des_t *des, void *key, sc_des_key_bits_t key_len);

/**
  \brief       Des ecb encrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_des_ecb_encrypt(sc_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       Des ecb decrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_des_ecb_decrypt(sc_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       Des cbc encrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_des_cbc_encrypt(sc_des_t *des, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Des cbc decrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_des_cbc_decrypt(sc_des_t *des, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       TDes ecb encrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_tdes_ecb_encrypt(sc_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       TDes ecb decrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_tdes_ecb_decrypt(sc_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       TDes cbc encrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_tdes_cbc_encrypt(sc_des_t *des, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       TDes cbc decrypt
  \param[in]   des     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_tdes_cbc_decrypt(sc_des_t *des, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Config DES mode dma or slave
  \param[in]   mode    \ref sc_des_trans_mode_t 
  \return      None
*/
void sc_des_trans_config(sc_des_t *des, sc_des_trans_mode_t mode) ;

#ifdef __cplusplus
}
#endif

#endif /* _SC_DES_H_ */
