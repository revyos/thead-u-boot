/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypto_ecdh.h
 * @brief    Header File for curve25519( a state-of-the-art Diffie-Hellman function)
 * @version  V3.3
 * @date     10. June 2022
 * @model    ecdh
 ******************************************************************************/
#ifndef _SC_ECDH_H_
#define _SC_ECDH_H_
#include "sec_include_config.h"

#define CONFIG_SEC_CRYPTO_ECC

#ifdef CONFIG_SEC_CRYPTO_ECC

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SEC_LIB_VERSION
#include "drv/ecdh.h"
#include "drv/ecc.h"
#include "sec_crypto_ecc.h"
#else
#include "ecdh.h"
#include "ecc.h"
#include "sec_crypto_ecc.h"
#endif

/**
  \brief       ecdh calc secret
  \param[in]   ecc ecc handle to operate.
  \param[in]   pubkey  Pointer to the A(or B) public key.
  \param[out]  privkey Pointer to the B(or A) private key.
  \param[out]  out Pointer to the share secret.
  \param[out]  len length of the share secret.
  \return      \ref uint32_t.
*/

uint32_t sc_ecdh_calc_secret(sc_ecc_t *ecc, uint8_t privkey[32],
                            uint8_t pubkey[65], uint8_t out[32], 
                            uint32_t *len, sc_ecc_curve_type type) ;

#ifdef __cplusplus
}
#endif

#endif

#endif /* _SC_CURVE15519_H_ */
