/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypto_curve25519.h
 * @brief    Header File for curve25519( a state-of-the-art Diffie-Hellman function)
 * @version  V3.3
 * @date     10. June 2022
 * @model    ecdh
 ******************************************************************************/
#ifndef _SC_CURVE25519_H_
#define _SC_CURVE25519_H_
#include "sec_include_config.h"

#define CONFIG_SEC_CRYPTO_CURVE25519

#ifdef CONFIG_SEC_CRYPTO_CURVE25519

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SEC_LIB_VERSION
#include "drv/curve25519.h"
#include "sec_crypto_curve25519.h"
#else
#include "curve25519.h"
#include "sec_crypto_curve25519.h"
#endif

typedef struct {
#ifdef CONFIG_CSI_V2
        csi_curve25519_t ctx;
#endif
} sc_curve25519_t;

/**
  \brief       Initialize CURVE25519.
  \param[in]   idx  device id
  \return      Error code \ref csi_error_t
*/
csi_error_t sc_curve25519_init(void *ctx, uint32_t idx);

/**
  \brief       De-initialize CURVE25519 Interface. stops operation and releases the software resources used by the interface
  \param[in]   curve25519  ecc handle to operate.
  \return      none
*/
void sc_curve25519_uninit(void *ctx);

/**
  \brief       curve25519 gen public key
  \param[in]   ctx ctx handle to operate.
  \param[in]   privkey Pointer to the A(or B) private key.
  \param[out]  pubkey  Pointer to the A(or B) public key.
  \return      \ref uint32_t.
*/
uint32_t sc_curve25519_gen_pubkey(void *ctx, const uint8_t privkey[32], uint8_t pubkey[32]);

/**
  \brief       curve25519 gen key pair
  \param[in]   ctx ctx handle to operate.
  \param[out]  privkey Pointer to the A(or B) private key.
  \param[out]  pubkey  Pointer to the A(or B) public key.
  \return      \ref uint32_t.
*/
uint32_t sc_curve25519_gen_keypair(void *ctx, uint8_t privkey[32], uint8_t pubkey[32]);

/**
  \brief       curve25519 check key pair
  \param[in]   ctx ctx handle to operate.
  \param[in]   privkey Pointer to the A(or B) private key.
  \param[in]   pubkey  Pointer to the A(or B) public key.
  \return      \ref uint32_t.
*/
uint32_t sc_curve25519_check_keypair(void *ctx, const uint8_t privkey[32], const uint8_t pubkey[32]);

/**
  \brief       curve25519 check key pair
  \param[in]   ctx ctx handle to operate.
  \param[in]   privkey Pointer to the B(or A) private key.
  \param[in]   pubkey  Pointer to the A(or B) public key.
  \param[out]  sk      Pointer to the share key.
  \param[out]  sk_len  Pointer to the share key length byte.
  \return      \ref uint32_t.
*/
uint32_t sc_curve25519_calc_secret(void *ctx, const uint8_t privkey[32], const uint8_t pubkey[32], uint8_t sk[32], uint32_t *sk_len);

#ifdef __cplusplus
}
#endif

#endif

#endif /* _SC_CURVE15519_H_ */
