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

#ifndef _DRV_ECC_H_
#define _DRV_ECC_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECC_PRIME_CURVE_G_BYTES 64
#define ECC_PRIME_CURVE_P_BYTES 70

typedef struct {
    uint32_t ecc_curve : 1; ///< supports 256bits curve
} ecc_capabilities_t;

/**
\brief ECC ciphertext order
*/
typedef enum {
    ECC_C1C3C2 = 0,
    ECC_C1C2C3,
} ecc_cipher_order_e;

typedef enum {
    ECC_ENDIAN_LITTLE = 0, ///< Little Endian
    ECC_ENDIAN_BIG         ///< Big Endian
} ecc_endian_mode_e;

typedef enum {
    ECC_PRIME256V1 = 0,
} ecc_prime_curve_type;

/**
\brief ECC key exchange role
*/
typedef enum { ECC_Role_Sponsor = 0, ECC_Role_Responsor } ecc_exchange_role_e;

/****** ECC Event *****/
typedef enum {
    ECC_EVENT_MAKE_KEY_COMPLETE = 0, ///< Make key completed
    ECC_EVENT_ENCRYPT_COMPLETE,      ///< Encrypt completed
    ECC_EVENT_DECRYPT_COMPLETE,      ///< Decrypt completed
    ECC_EVENT_SIGN_COMPLETE,         ///< Sign completed
    ECC_EVENT_VERIFY_COMPLETE,       ///< Verify completed
    ECC_EVENT_EXCHANGE_KEY_COMPLETE, ///< Exchange key completed
} ecc_event_e;

typedef struct {
    ecc_prime_curve_type type;
    uint32_t *p;
} csi_ecc_prime_curve_t;

typedef struct {
    ecc_prime_curve_type type;
    uint8_t *G;
    uint8_t *n;
} csi_ecc_curve_g_t;

/**
\brief ECC status
*/
typedef struct {
    uint32_t busy : 1; ///< Calculate busy flag
} csi_ecc_state_t;

typedef struct {
    csi_dev_t       dev;
    void *          cb;
    void *          arg;
    csi_ecc_state_t state;
    ecc_prime_curve_type  type;
} csi_ecc_t;

///< Pointer to \ref csi_ecc_callback_t : ECC Event call back.
typedef void (*csi_ecc_callback_t)(ecc_event_e event);

/**
  \brief       Initialize ECC.
  \param[in]   ecc  ecc handle to operate.
  \param[in]   idx  device id
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_init(csi_ecc_t *ecc, uint32_t idx);

/**
  \brief       De-initialize ECC Interface. stops operation and releases the software resources used by the interface
  \param[in]   ecc  ecc handle to operate.
  \return      none
*/
void csi_ecc_uninit(csi_ecc_t *ecc);

/**
  \brief       ecc get capability.
  \param[in]   ecc  Operate handle.
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_config(csi_ecc_t *ecc, ecc_cipher_order_e co,
                           ecc_endian_mode_e endian);

/**
  \brief       Attach the callback handler to ECC
  \param[in]   ecc  Operate handle.
  \param[in]   cb    Callback function
  \param[in]   arg   User can define it by himself as callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_ecc_attach_callback(csi_ecc_t *ecc, csi_ecc_callback_t cb,
                                    void *arg);

/**
  \brief       Detach the callback handler
  \param[in]   ecc  Operate handle.
*/
csi_error_t csi_ecc_detach_callback(csi_ecc_t *ecc);

/**
  \brief       ecc get capability.
  \param[in]   ecc  Operate handle.
  \param[out]   cap  Pointer of ecc_capabilities_t.
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_get_capabilities(csi_ecc_t *ecc, ecc_capabilities_t *cap);

csi_error_t csi_ecc_check_keypair(csi_ecc_t *ecc, uint8_t pubkey[65], uint8_t prikey[32]);

/**
  \brief       generate ecc key.
  \param[in]   ecc       ecc handle to operate.
  \param[out]  private   Pointer to the ecc private key, alloc by caller.
  \param[out]  public   Pointer to the ecc public key, alloc by caller.
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_gen_key(csi_ecc_t *ecc, uint8_t pubkey[65],
                            uint8_t prikey[32]);

/**
  \brief       generate ecc pubkey by privkey.
  \param[in]   ecc       ecc handle to operate.
  \param[in]   private   Pointer to the ecc private key, alloc by caller.
  \param[out]  public   Pointer to the ecc public key, alloc by caller.
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_gen_pubkey(csi_ecc_t *ecc, uint8_t pubkey[65],
                            uint8_t prikey[32]);

/**
  \brief       ecc sign
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_sign(csi_ecc_t *ecc, uint8_t d[32], uint8_t prikey[32],
                         uint8_t s[64]);

/**
  \brief       ecc sign
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
csi_error_t csi_ecc_sign_async(csi_ecc_t *ecc, uint8_t d[32],
                               uint8_t prikey[32], uint8_t s[64]);

/* TODO */
/**
  \brief       ecc verify
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool csi_ecc_verify(csi_ecc_t *ecc, uint8_t d[32], uint8_t pubkey[65],
                    uint8_t s[64]);

/**
  \brief       ecc verify
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool csi_ecc_verify_async(csi_ecc_t *ecc, uint8_t d[32], uint8_t pubkey[65],
                          uint8_t s[64]);

/**
  \brief       ecc encrypto
  \param[in]   ecc       ecc handle to operate.
  \param[in]   Plain       Pointer to the plaintext.
  \param[in]  PlainByteLen plaintext len
  \param[in]  pubKey public key.
  \param[out]  Cipher Pointer to the chipher
  \param[out]  CipherByteLen Pointer to the chipher len.
  \return      uint32_t
*/
csi_error_t csi_ecc_encrypt(csi_ecc_t *ecc, uint8_t *Plain,
                            uint32_t PlainByteLen, uint8_t pubKey[65],
                            uint8_t *Cipher, uint32_t *CipherByteLen);

/**
  \brief       ecc encrypto
  \param[in]   ecc       ecc handle to operate.
  \param[in]  Cipher Pointer to the chipher
  \param[in]  CipherByteLen chipher len.
  \param[in]  prikey private key.
  \param[out]   Plain       Pointer to the plaintext.
  \param[out]  PlainByteLen plaintext len
  \return      uint32_t
*/
csi_error_t csi_ecc_decrypt(csi_ecc_t *ecc, uint8_t *Cipher,
                            uint32_t CipherByteLen, uint8_t prikey[32],
                            uint8_t *Plain, uint32_t *PlainByteLen);

/**
  \brief       ecc key exchange
  \param[in]   ecc       ecc handle to operate.
  \return      uint32_t
*/
csi_error_t csi_ecc_exchangekey(csi_ecc_t *ecc, ecc_exchange_role_e role,
                                uint8_t *dA, uint8_t *PB, uint8_t *rA,
                                uint8_t *RA, uint8_t *RB, uint8_t *ZA,
                                uint8_t *ZB, uint32_t kByteLen, uint8_t *KA,
                                uint8_t *S1, uint8_t *SA);

/**
  \brief       ecc key exchange get Z.
  \param[in]   ecc       ecc handle to operate.
  \return      uint32_t
*/
csi_error_t csi_ecc_getZ(csi_ecc_t *ecc, uint8_t *ID, uint32_t byteLenofID,
                         uint8_t pubKey[65], uint8_t Z[32]);

/**
  \brief       ecc key exchange get E
  \param[in]   ecc       ecc handle to operate.
  \return      uint32_t
*/
csi_error_t csi_ecc_getE(csi_ecc_t *ecc, uint8_t *M, uint32_t byteLen,
                         uint8_t Z[32], uint8_t E[32]);

/**
  \brief       Get ECC state.
  \param[in]   ecc      ECC handle to operate.
  \param[out]  state    ECC state \ref csi_ecc_state_t.
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_ecc_get_state(csi_ecc_t *ecc, csi_ecc_state_t *state);

/**
  \brief       Enable ecc power manage
  \param[in]   ecc  ECC handle to operate.
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_ecc_enable_pm(csi_ecc_t *ecc);

/**
  \brief       Disable ecc power manage
  \param[in]   ecc  ECC handle to operate.
*/
void csi_ecc_disable_pm(csi_ecc_t *ecc);

#ifdef __cplusplus
extern "C" {
#endif

#endif