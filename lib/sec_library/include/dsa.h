/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/ecc.h
 * @brief      Header File for ECC Driver
 * @version    V3.3
 * @date       30. May 2022
 * @model      ECC
 ******************************************************************************/

#ifndef _DRV_DSA_H_
#define _DRV_DSA_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif


#define CSI_DSA_SHA1_PUBKEY_LEN        (128)
#define CSI_DSA_SHA1_PRIVKEY_LEN       (20)
#define CSI_DSA_SHA1_RK_LEN            (20) /*random*/
#define CSI_DSA_SHA1_SIGNATURE_LEN     (40)
#define CSI_DSA_SHA1_DIGEST_LEN        (20)

#define CSI_DSA_SHA256_PUBKEY_LEN      (256)
#define CSI_DSA_SHA256_PRIVKEY_LEN     (32)
#define CSI_DSA_SHA256_RK_LEN          (32) /*random*/
#define CSI_DSA_SHA256_SIGNATURE_LEN   (64)
#define CSI_DSA_SHA256_DIGEST_LEN      (32)

#define CSI_DSA_SHA224_PUBKEY_LEN      (256)
#define CSI_DSA_SHA224_PRIVKEY_LEN     (28)
#define CSI_DSA_SHA224_RK_LEN          (28) /*random*/
#define CSI_DSA_SHA224_SIGNATURE_LEN   (56)
#define CSI_DSA_SHA224_DIGEST_LEN      (28)

#define CSI_DSA_SHA1_SHIFT_BYTES          (1*4)
#define CSI_DSA_SHA224_SHIFT_BYTES        (1*4)

#define DSA_SHA1_GROUP_P_G_BYTES          (128)
#define DSA_SHA1_GROUP_PARAM_WORDS        (76)
#define DSA_SHA1_GROUP_N_BYTES            (20)
#define DSA_SHA1_A_LEN_WORDS              (0x20)
#define DSA_SHA1_B_LEN_WORDS              (0x05)

#define DSA_SHA256_GROUP_P_G_BYTES        (256)
#define DSA_SHA256_GROUP_PARAM_WORDS      (140)
#define DSA_SHA256_GROUP_N_BYTES          (32)
#define DSA_SHA256_A_LEN_WORDS            (0x40)
#define DSA_SHA256_B_LEN_WORDS            (0x08)

#define DSA_SHA224_GROUP_P_G_BYTES        (256)
#define DSA_SHA224_GROUP_PARAM_WORDS      (140)
#define DSA_SHA224_GROUP_N_BYTES          (28)
#define DSA_SHA224_A_LEN_WORDS            (0x40)
#define DSA_SHA224_B_LEN_WORDS            (0x07)


/**
\brief DSA sha type
*/
typedef enum {
    DSA_SHA1   = 0,
    DSA_SHA224,
    DSA_SHA256,
    DSA_SHA_TYPE_MAX,
} dsa_sha_type;

/**
\brief DSA group param
*/
typedef struct {
    dsa_sha_type  type;
    uint32_t      *group;
    uint32_t      words;
    uint32_t      offset;
} csi_dsa_group_t;

/**
\brief DSA g param
*/
typedef struct {
    dsa_sha_type  type;
    uint8_t       *p;
    uint8_t       *g;
    uint8_t       *n;
} csi_dsa_gpn_t;

typedef enum{
    CSI_DSA_SHA1_SIGN = 0,
    CSI_DSA_SHA1_VERIFY,
    CSI_DSA_SHA224_SIGN,
    CSI_DSA_SHA224_VERIFY,
    CSI_DSA_SHA256_SIGN,
    CSI_DSA_SHA256_VERIFY,
    CSI_DSA_FUN_MAX,
}dsa_fun_type;

typedef struct {
  dsa_fun_type    type;
  uint32_t        dsa_pka_a_offset;
  uint32_t        dsa_pka_b_offset;
  uint32_t        dsa_pka_c_offset;
  uint32_t        dsa_pka_d_offset;
  uint32_t        dsa_pka_r_offset;
  uint32_t        dsa_pka_s_offset;
  uint32_t        dsa_pka_function;
  uint32_t        dsa_pka_A_len;
  uint32_t        dsa_pka_B_len;
} csi_dsa_pka_offset_t;

/**
\brief ECC handle
*/
typedef struct {
    csi_dev_t       dev;
    dsa_sha_type    sha_type;
} csi_dsa_t;

/**
  \brief       Initialize ECC.
  \param[in]   idx  device id
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_dsa_init(csi_dsa_t *dsa, uint32_t idx);

/**
  \brief       De-initialize ECC Interface. stops operation and releases the software resources used by the interface
  \param[in]   ecc  ecc handle to operate.
  \return      none
*/
void csi_dsa_uninit(csi_dsa_t *dsa);

/**
  \brief       config dsa sha type
  \param[in]   ecc  ECC handle to operate.
  \param[in]   type  \ref dsa_sha_type.
*/
csi_error_t csi_dsa_config(csi_dsa_t *dsa, dsa_sha_type type);
/**
  \brief       dsa gen key pairs
  \param[in]   ecc      dsa handle to operate.
  \param[out]  privkey  Pointer to the private key
  \param[out]  pubkey   Pointer to the public key
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_dsa_gen_keypairs(csi_dsa_t *dsa, uint8_t *prikey, uint8_t *pubkey);

/**
  \brief       dsa sign
  \param[in]   ecc      dsa handle to operate.
  \param[in]   d        Pointer to the digest.
  \param[out]  privkey  Pointer to the private key
  \param[out]  s        Pointer to the signature
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_dsa_sign(csi_dsa_t *dsa, uint8_t *d, uint8_t *prikey, uint8_t *s);

/**
  \brief       dsa verify
  \param[in]   ecc      ecc handle to operate.
  \param[in]   d        Pointer to the digest.
  \param[out]  privkey  Pointer to the private key
  \param[out]  s        Pointer to the signature
  \return      verify result 
*/
bool csi_dsa_verify(csi_dsa_t *dsa, uint8_t *d, uint8_t *pubkey, uint8_t *s);

#ifdef __cplusplus
}
#endif

#endif