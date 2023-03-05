/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypt_sm2.h
 * @brief    Header File for SM2
 * @version  V1.0
 * @date     20. Jul 2021
 * @model    sm2
 ******************************************************************************/
#ifndef _SC_SM2_H_
#define _SC_SM2_H_
#include "sec_include_config.h"

#ifdef CONFIG_SEC_CRYPTO_SM2

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SEC_LIB_VERSION
#include "drv/sm2.h"
#else
#include "sm2.h"
#endif


typedef struct {
        uint32_t sm2_curve : 1; ///< supports 256bits curve
} sc_sm2_capabilities_t;

/**
\brief SM2 ciphertext order
*/
typedef enum {
        SC_SM2_C1C3C2 = 0,
        SC_SM2_C1C2C3,
} sc_sm2_cipher_order_e;

typedef enum {
        SC_SM2_ENDIAN_LITTLE = 0, ///< Little Endian
        SC_SM2_ENDIAN_BIG         ///< Big Endian
} sc_sm2_endian_mode_e;

/**
\brief SM2 status
*/
typedef struct {
        uint32_t busy : 1; ///< Calculate busy flag
} sc_sm2_state_t;

/**
\brief SM2 key exchange role
*/
typedef enum { SC_SM2_Role_Sponsor = 0, SC_SM2_Role_Responsor } sc_sm2_exchange_role_e;

/****** SM2 Event *****/
typedef enum {
        SC_SM2_EVENT_MAKE_KEY_COMPLETE = 0, ///< Make key completed
        SC_SM2_EVENT_ENCRYPT_COMPLETE,      ///< Encrypt completed
        SC_SM2_EVENT_DECRYPT_COMPLETE,      ///< Decrypt completed
        SC_SM2_EVENT_SIGN_COMPLETE,         ///< Sign completed
        SC_SM2_EVENT_VERIFY_COMPLETE,       ///< Verify completed
        SC_SM2_EVENT_EXCHANGE_KEY_COMPLETE, ///< Exchange key completed
} sc_sm2_event_e;

typedef struct {
#ifdef CONFIG_CSI_V2
        csi_sm2_t sm2;
#endif
} sc_sm2_t;

///< Pointer to \ref sc_sm2_callback_t : SM2 Event call back.
typedef void (*sc_sm2_callback_t)(sc_sm2_event_e event);

/**
  \brief       Initialize SM2.
  \param[in]   sm2  sm2 handle to operate.
  \param[in]   idx  device id
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_init(sc_sm2_t *sm2, uint32_t idx);

/**
  \brief       De-initialize SM2 Interface. stops operation and releases the
  software resources used by the interface \param[in]   sm2  sm2 handle to
  operate. \return      none
*/
void sc_sm2_uninit(sc_sm2_t *sm2);

/**
  \brief       sm2 get capability.
  \param[in]   sm2  Operate handle.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_config(sc_sm2_t *sm2, sc_sm2_cipher_order_e co, sc_sm2_endian_mode_e endian);

/**
  \brief       Attach the callback handler to SM2
  \param[in]   sm2   Operate handle.
  \param[in]   cb    Callback function
  \param[in]   arg   User can define it by himself as callback's param
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_attach_callback(sc_sm2_t *sm2, sc_sm2_callback_t cb, void *arg);

/**
  \brief       Detach the callback handler
  \param[in]   sm2  Operate handle.
*/
void sc_sm2_detach_callback(sc_sm2_t *sm2);

/**
  \brief       sm2 get capability.
  \param[in]   sm2  Operate handle.
  \param[out]  cap  Pointer of sc_sm2_capabilities_t.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_get_capabilities(sc_sm2_t *sm2, sc_sm2_capabilities_t *cap);

/**
  \brief       check whether the public key and private key are a pair.
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   private   Pointer to the sm2 private key, alloc by caller.
  \param[in]   public    Pointer to the sm2 public key, alloc by caller.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_check_keypair(sc_sm2_t *sm2, uint8_t pubkey[65], uint8_t prikey[32]);

/**
  \brief       generate sm2 key.
  \param[in]   sm2       sm2 handle to operate.
  \param[out]  private   Pointer to the sm2 private key, alloc by caller.
  \param[out]  public    Pointer to the sm2 public key, alloc by caller.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_gen_key(sc_sm2_t *sm2, uint8_t pubkey[65], uint8_t prikey[32]);

/**
  \brief       sm2 sign
  \param[in]   sm2     sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s       Pointer to the signature
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_sign(sc_sm2_t *sm2, uint8_t d[32], uint8_t prikey[32], uint8_t s[64]);

/**
  \brief       sm2 sign
  \param[in]   sm2     sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s       Pointer to the signature
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_sign_async(sc_sm2_t *sm2, uint8_t d[32], uint8_t prikey[32], uint8_t s[64]);

/* TODO */
/**
  \brief       sm2 verify
  \param[in]   sm2     sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s       Pointer to the signature
  \return      verify result
*/
bool sc_sm2_verify(sc_sm2_t *sm2, uint8_t d[32], uint8_t pubkey[65], uint8_t s[64]);

/**
  \brief       sm2 verify
  \param[in]   sm2     sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s       Pointer to the signature
  \return      verify result
*/
bool sc_sm2_verify_async(sc_sm2_t *sm2, uint8_t d[32], uint8_t pubkey[65], uint8_t s[64]);

/**
  \brief       sm2 encrypto
  \param[in]   sm2              sm2 handle to operate.
  \param[in]   plain            Pointer to the plaintext.
  \param[in]   PlainByteLen     plaintext len
  \param[in]   pubKey           public key.
  \param[out]  cipher           Pointer to the chipher
  \param[out]  cipher_byte_len  Pointer to the chipher len.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_encrypt(sc_sm2_t *sm2, uint8_t *plain, uint32_t plain_len,
                        uint8_t pubKey[65], uint8_t *cipher, uint32_t *cipher_len);

/**
  \brief       sm2 encrypto
  \param[in]   sm2              sm2 handle to operate.
  \param[in]   cipher           Pointer to the chipher
  \param[in]   CipherByteLen    chipher len.
  \param[in]   prikey           private key.
  \param[out]  plain            Pointer to the plaintext.
  \param[out]  PlainByteLen     plaintext len
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_decrypt(sc_sm2_t *sm2, uint8_t *cipher, uint32_t cipher_len,
                        uint8_t prikey[32], uint8_t *plain,
                        uint32_t *plain_len);

/**
  \brief       sm2 key exchange
  \param[in]   sm2       sm2 handle to operate.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_exchangekey(sc_sm2_t *sm2, sc_sm2_exchange_role_e role,
                            uint8_t *da, uint8_t *pb, uint8_t *ra1, uint8_t *ra,
                            uint8_t *rb, uint8_t *za, uint8_t *zb,
                            uint32_t k_len, uint8_t *ka, uint8_t *s1,
                            uint8_t *sa);

/**
  \brief       sm2 key exchange get Z.
  \param[in]   sm2       sm2 handle to operate.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_getZ(sc_sm2_t *sm2, uint8_t *id, uint32_t id_len,
                     uint8_t pubkey[65], uint8_t z[32]);

/**
  \brief       sm2 key exchange get E
  \param[in]   sm2       sm2 handle to operate.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_getE(sc_sm2_t *sm2, uint8_t *m, uint32_t len, uint8_t z[32], uint8_t e[32]);

/**
  \brief       Get SM2 state.
  \param[in]   sm2      SM2 handle to operate.
  \param[out]  state    SM2 state \ref sc_sm2_state_t.
  \return      Error code \ref uint32_t
*/
uint32_t sc_sm2_get_state(sc_sm2_t *sm2, sc_sm2_state_t *state);

#ifdef __cplusplus
extern "C" {
#endif

#endif

#endif /* _SC_SM2_H_ */
