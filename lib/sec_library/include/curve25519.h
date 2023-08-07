/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/curve25519.h
 * @brief      Header File for CURVE25519 Driver
 * @version    V3.3
 * @date       10.June 2022
 * @model      ECC
 ******************************************************************************/

#ifndef _DRV_CURVE25519_H_
#define _DRV_CURVE25519_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    csi_dev_t       dev;
} csi_curve25519_t;

/**
  \brief       Initialize CURVE25519.
  \param[in]   idx  device id
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_curve25519_init(void *ctx, uint32_t idx);

/**
  \brief       De-initialize CURVE25519 Interface. stops operation and releases the software resources used by the interface
  \param[in]   curve25519  ecc handle to operate.
  \return      none
*/
void csi_curve25519_uninit(void *ctx);

/**
  \brief       curve25519 gen public key
  \param[in]   ctx ctx handle to operate.
  \param[in]   privkey Pointer to the A(or B) private key.
  \param[out]  pubkey  Pointer to the A(or B) public key.
  \return      Error code \ref csi_error_t.
*/
csi_error_t csi_curve25519_gen_pubkey(void *ctx, const uint8_t privkey[32], uint8_t pubkey[32]);

/**
  \brief       curve25519 gen key pair
  \param[in]   ctx ctx handle to operate.
  \param[out]  privkey Pointer to the A(or B) private key.
  \param[out]  pubkey  Pointer to the A(or B) public key.
  \return      Error code \ref csi_error_t.
*/
csi_error_t csi_curve25519_gen_keypair(void *ctx, uint8_t privkey[32], uint8_t pubkey[32]);

/**
  \brief       curve25519 check key pair
  \param[in]   ctx ctx handle to operate.
  \param[in]   privkey Pointer to the B(or A) private key.
  \param[in]   pubkey  Pointer to the A(or B) public key.
  \param[out]  sk      Pointer to the share key.
  \param[out]  sk_len  Pointer to the share key length byte.
  \return      Error code \ref csi_error_t.
*/
csi_error_t csi_curve25519_check_keypair(void *ctx, const uint8_t privkey[32], const uint8_t pubkey[32]);

/**
  \brief       curve25519 check key pair
  \param[in]   ctx ctx handle to operate.
  \param[in]   privkey Pointer to the B(or A) private key.
  \param[in]   pubkey  Pointer to the A(or B) public key.
  \param[out]  sk      Pointer to the share key.
  \param[out]  sk_len  Pointer to the share key length byte.
  \return      Error code \ref csi_error_t.
*/
csi_error_t csi_curve25519_calc_secret(void *ctx, const uint8_t privkey[32], const uint8_t pubkey[32], uint8_t sk[32], uint32_t *sk_len);


#ifdef __cplusplus
}
#endif

#endif