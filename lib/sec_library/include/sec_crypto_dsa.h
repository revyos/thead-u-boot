/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypt_dsa.h
 * @brief    Header File for DSA
 * @version  V3.3
 * @date     05. Dec 2022
 * @model    dsa
 ******************************************************************************/
#ifndef _SC_DSA_H_
#define _SC_DSA_H_
#include "sec_include_config.h"

#define CONFIG_SEC_CRYPTO_DSA

#ifdef CONFIG_SEC_CRYPTO_DSA

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SEC_LIB_VERSION
#include "drv/dsa.h"
#else
#include "dsa.h"
#endif

typedef enum{
    SC_DSA_SHA1 = 0,
    SC_DSA_SHA224,
    SC_DSA_SHA256,
    SC_DSA_SHA_MAX,
}sc_dsa_sha_type;

/**
\brief DSA status
*/
typedef struct {
        uint32_t busy : 1; /*Calculate busy flag*/
} sc_dsa_state_t;

typedef struct {
#ifdef CONFIG_CSI_V2
        csi_dsa_t dsa;
#endif
} sc_dsa_t;

/**
  \brief       Initialize DSA.
  \param[in]   dsa  dsa handle to operate.
  \param[in]   idx  device id
  \return      \ref uint32_t
*/
uint32_t sc_dsa_init(sc_dsa_t *dsa, uint32_t idx);

/**
  \brief       De-initialize ECC Interface. stops operation and releases the
  software resources used by the interface \param[in]   dsa  dsa handle to
  operate. \return      none
*/
void sc_dsa_uninit(sc_dsa_t *dsa);

/**
  \brief       Initialize DSA.
  \param[in]   dsa  dsa handle to operate.
  \param[in]   idx  device id
  \return      \ref uint32_t
*/
uint32_t sc_dsa_config(sc_dsa_t *dsa, sc_dsa_sha_type type);

/**
  \brief       dsa gen key pairs
  \param[in]   dsa      dsa handle to operate.
  \param[out]  privkey  Pointer to the private key
  \param[out]  pubkey   Pointer to the public key
  \return      Error code \ref csi_error_t
*/
uint32_t sc_dsa_gen_keypairs(sc_dsa_t *dsa, uint8_t *prikey, uint8_t *pubkey);

/**
  \brief       dsa sign
  \param[in]   dsa     dsa handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
uint32_t sc_dsa_sign(sc_dsa_t *dsa, uint8_t *d, uint8_t *prikey, uint8_t *s);

/**
  \brief       dsa verify
  \param[in]   dsa     dsa handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool sc_dsa_verify(sc_dsa_t *dsa, uint8_t *d, uint8_t *pubkey, uint8_t *s);


#ifdef __cplusplus
}
#endif

#endif

#endif /* _SC_ECC_H_ */

