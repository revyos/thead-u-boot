/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypt_sm4.h
 * @brief    Header File for SM4
 * @version  V1.0
 * @date     20. Jul 2021
 * @model    sm4
 ******************************************************************************/

#ifndef _SC_SM4_H_
#define _SC_SM4_H_
#include "sec_include_config.h"

#ifdef CONFIG_CSI_V2
#ifdef SEC_LIB_VERSION
#include "drv/sm4.h"
#else
#include "sm4.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
\brief SM4 Ctrl Block
*/
typedef struct {
#ifdef CONFIG_CSI_V2
        csi_sm4_t sm4;
#endif
} sc_sm4_t;

// Function documentation
/**
  \brief       Initialize sm4 Interface. Initializes the resources needed for
  the sm4 interface \param[in]   sm4    operate handle \param[in]   idx device
  id \return      error code \ref uint32_t
*/
uint32_t sc_sm4_init(sc_sm4_t *sm4, uint32_t idx);

/**
  \brief       De-initialize sm4 Interface. stops operation and releases the
  software resources used by the interface \param[in]   sm4    handle to operate
  \return      None
*/
void sc_sm4_uninit(sc_sm4_t *sm4);

/**
  \brief       Set encrypt key
  \param[in]   sm4        handle to operate
  \param[in]   key        Pointer to the key buf
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_set_encrypt_key(sc_sm4_t *sm4, uint8_t *key, csi_sm4_key_bits_t key_len);

/**
  \brief       Set decrypt key
  \param[in]   sm4        handle to operate
  \param[in]   key        Pointer to the key buf
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_set_decrypt_key(sc_sm4_t *sm4, uint8_t *key, csi_sm4_key_bits_t key_len);

/**
  \brief       sm4 ecb encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_ecb_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size);

/**
  \brief       sm4 ecb decrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_ecb_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size);

/**
  \brief       sm4 cbc encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cbc_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size, uint8_t *iv);

/**
  \brief       sm4 cbc decrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cbc_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size, uint8_t *iv);

/**
  \brief       sm4 cfb1 encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cfb1_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                               uint32_t size, uint8_t *iv);

/**
  \brief       sm4 cfb1 decrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cfb1_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                               uint32_t size, uint8_t *iv);

/**
  \brief       sm4 cfb8 encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cfb8_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                               uint32_t size, uint8_t *iv);

/**
  \brief       sm4 cfb8 decrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cfb8_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                               uint32_t size, uint8_t *iv);

/**
  \brief       sm4 cfb128 decrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cfb128_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                                 uint32_t size, uint8_t *iv, uint32_t *num);

/**
  \brief       sm4 cfb128 encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_cfb128_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                                 uint32_t size, uint8_t *iv, uint32_t *num);
/**
  \brief       sm4 ofb encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_ofb_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size, uint8_t *iv, uint32_t *num);

/**
  \brief       sm4 ofb encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_ofb_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size, uint8_t *iv, uint32_t *num);

/**
  \brief       sm4 ctr encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   nonce_counter      counter
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_ctr_encrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size, uint8_t nonce_counter[16]);
/**
  \brief       sm4 ctr encrypt
  \param[in]   sm4     handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   nonce_counter      counter
  \return      error code \ref uint32_t
*/
uint32_t sc_sm4_ctr_decrypt(sc_sm4_t *sm4, uint8_t *in, uint8_t *out,
                              uint32_t size, uint8_t nonce_counter[16]);

#ifdef __cplusplus
extern "C" {
#endif

#endif /* _SC_SM4_H_ */
