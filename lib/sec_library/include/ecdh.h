/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/ecdh.h
 * @brief      Header File for ECDH Driver
 * @version    V3.3
 * @date       10.June 2022
 * @model      ECC
 ******************************************************************************/

#ifndef _DRV_ECDH_H_
#define _DRV_ECDH_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSI_ECDH_PUBKEY_LEN   (65-1)
#define CSI_ECDH_PRIVKEY_LEN  (32)
#define CSI_ECDH_SHARE_LEN    (64)
#define CSI_ECDH_SHAREKEY_LEN (32)

/**
  \brief       ecdh cacl share secret
  \param[in]  ecc       ecc handle to operate.
  \param[in]  pubkey    Pointer to the A public key.
  \param[in]  prikey    Pointer to the B private key.
  \param[out] shareKey  Pointer to the share secret.
  \param[out] len       length of the share secret.
  \return     Error code \ref csi_error_t
*/
csi_error_t csi_ecdh_calc_secret(csi_ecc_t *ecc, uint8_t privkey[32], uint8_t pubkey[65], uint8_t shareKey[32], uint32_t *len);

#ifdef __cplusplus
}
#endif

#endif