/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/sm2.h
 * @brief      Header File for SM2 Driver
 * @version    V2.0
 * @date       9. DEC 2020
 * @model      SM2
 ******************************************************************************/

#ifndef _DRV_SM2_H_
#define _DRV_SM2_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        uint32_t sm2_curve : 1; ///< supports 256bits curve
} sm2_capabilities_t;

/**
\brief SM2 ciphertext order
*/
typedef enum {
        SM2_C1C3C2 = 0,
        SM2_C1C2C3,
} sm2_cipher_order_e;

typedef enum {
        SM2_ENDIAN_LITTLE = 0, ///< Little Endian
        SM2_ENDIAN_BIG         ///< Big Endian
} sm2_endian_mode_e;

/**
\brief SM2 status
*/
typedef struct {
        uint32_t busy : 1; ///< Calculate busy flag
} csi_sm2_state_t;

/**
\brief SM2 key exchange role
*/
typedef enum { SM2_Role_Sponsor = 0, SM2_Role_Responsor } sm2_exchange_role_e;

/****** SM2 Event *****/
typedef enum {
        SM2_EVENT_MAKE_KEY_COMPLETE = 0, ///< Make key completed
        SM2_EVENT_ENCRYPT_COMPLETE,      ///< Encrypt completed
        SM2_EVENT_DECRYPT_COMPLETE,      ///< Decrypt completed
        SM2_EVENT_SIGN_COMPLETE,         ///< Sign completed
        SM2_EVENT_VERIFY_COMPLETE,       ///< Verify completed
        SM2_EVENT_EXCHANGE_KEY_COMPLETE, ///< Exchange key completed
} sm2_event_e;

typedef struct {
        csi_dev_t       dev;
        void *          cb;
        void *          arg;
        csi_sm2_state_t state;
        void *          prim;
} csi_sm2_t;

///< Pointer to \ref csi_sm2_callback_t : SM2 Event call back.
typedef void (*csi_sm2_callback_t)(sm2_event_e event);

/**
  \brief       Initialize SM2.
  \param[in]   sm2  sm2 handle to operate.
  \param[in]   idx  device id
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_init(csi_sm2_t *sm2, uint32_t idx);

/**
  \brief       De-initialize SM2 Interface. stops operation and releases the software resources used by the interface
  \param[in]   sm2  sm2 handle to operate.
  \return      none
*/
void csi_sm2_uninit(csi_sm2_t *sm2);

/**
  \brief       sm2 get capability.
  \param[in]   sm2  Operate handle.
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_config(csi_sm2_t *sm2, sm2_cipher_order_e co,
                           sm2_endian_mode_e endian);

/**
  \brief       Attach the callback handler to SM2
  \param[in]   sm2  Operate handle.
  \param[in]   cb    Callback function
  \param[in]   arg   User can define it by himself as callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm2_attach_callback(csi_sm2_t *sm2, csi_sm2_callback_t cb,
                                    void *arg);

/**
  \brief       Detach the callback handler
  \param[in]   sm2  Operate handle.
*/
void csi_sm2_detach_callback(csi_sm2_t *sm2);

/**
  \brief       sm2 get capability.
  \param[in]   sm2  Operate handle.
  \param[out]   cap  Pointer of sm2_capabilities_t.
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_get_capabilities(csi_sm2_t *sm2, sm2_capabilities_t *cap);

csi_error_t csi_sm2_check_keypair(csi_sm2_t *sm2, uint8_t pubkey[65],
                                  uint8_t prikey[32]);

/**
  \brief       generate sm2 key.
  \param[in]   sm2       sm2 handle to operate.
  \param[out]  private   Pointer to the sm2 private key, alloc by caller.
  \param[out]  public   Pointer to the sm2 public key, alloc by caller.
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_gen_key(csi_sm2_t *sm2, uint8_t pubkey[65],
                            uint8_t prikey[32]);

/**
  \brief       sm2 sign
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_sign(csi_sm2_t *sm2, uint8_t d[32], uint8_t prikey[32],
                         uint8_t s[64]);

/**
  \brief       sm2 sign
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_sign_async(csi_sm2_t *sm2, uint8_t d[32],
                               uint8_t prikey[32], uint8_t s[64]);

/* TODO */
/**
  \brief       sm2 verify
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool csi_sm2_verify(csi_sm2_t *sm2, uint8_t d[32], uint8_t pubkey[65],
                    uint8_t s[64]);

/**
  \brief       sm2 verify
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool csi_sm2_verify_async(csi_sm2_t *sm2, uint8_t d[32], uint8_t pubkey[65],
                          uint8_t s[64]);

/**
  \brief       sm2 encrypto
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   Plain       Pointer to the plaintext.
  \param[in]  PlainByteLen plaintext len
  \param[in]  pubKey public key.
  \param[out]  Cipher Pointer to the chipher
  \param[out]  CipherByteLen Pointer to the chipher len.
  \return      uint32_t
*/
csi_error_t csi_sm2_encrypt(csi_sm2_t *sm2, uint8_t *Plain,
                            uint32_t PlainByteLen, uint8_t pubKey[65],
                            uint8_t *Cipher, uint32_t *CipherByteLen);

/**
  \brief       sm2 encrypto
  \param[in]   sm2       sm2 handle to operate.
  \param[in]  Cipher Pointer to the chipher
  \param[in]  CipherByteLen chipher len.
  \param[in]  prikey private key.
  \param[out]   Plain       Pointer to the plaintext.
  \param[out]  PlainByteLen plaintext len
  \return      uint32_t
*/
csi_error_t csi_sm2_decrypt(csi_sm2_t *sm2, uint8_t *Cipher,
                            uint32_t CipherByteLen, uint8_t prikey[32],
                            uint8_t *Plain, uint32_t *PlainByteLen);

/**
  \brief       sm2 key exchange
  \param[in]   sm2       sm2 handle to operate.
  \return      uint32_t
*/
csi_error_t csi_sm2_exchangekey(csi_sm2_t *sm2, sm2_exchange_role_e role,
                                uint8_t *dA, uint8_t *PB, uint8_t *rA,
                                uint8_t *RA, uint8_t *RB, uint8_t *ZA,
                                uint8_t *ZB, uint32_t kByteLen, uint8_t *KA,
                                uint8_t *S1, uint8_t *SA);

/**
  \brief       sm2 key exchange get Z.
  \param[in]   sm2       sm2 handle to operate.
  \return      uint32_t
*/
csi_error_t csi_sm2_getZ(csi_sm2_t *sm2, uint8_t *ID, uint32_t byteLenofID,
                         uint8_t pubKey[65], uint8_t Z[32]);

/**
  \brief       sm2 key exchange get E
  \param[in]   sm2       sm2 handle to operate.
  \return      uint32_t
*/
csi_error_t csi_sm2_getE(csi_sm2_t *sm2, uint8_t *M, uint32_t byteLen,
                         uint8_t Z[32], uint8_t E[32]);

/**
  \brief       Get SM2 state.
  \param[in]   sm2      SM2 handle to operate.
  \param[out]  state    SM2 state \ref csi_sm2_state_t.
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm2_get_state(csi_sm2_t *sm2, csi_sm2_state_t *state);

/**
  \brief       Enable sm2 power manage
  \param[in]   sm2  SM2 handle to operate.
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm2_enable_pm(csi_sm2_t *sm2);

/**
  \brief       Disable sm2 power manage
  \param[in]   sm2  SM2 handle to operate.
*/
void csi_sm2_disable_pm(csi_sm2_t *sm2);

#ifdef __cplusplus
extern "C" {
#endif

#endif