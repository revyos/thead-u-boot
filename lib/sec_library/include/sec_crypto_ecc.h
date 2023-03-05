/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     sec_crypt_ecc.h
 * @brief    Header File for ECC
 * @version  V3.3
 * @date     30. May 2022
 * @model    ecc
 ******************************************************************************/
#ifndef _SC_ECC_H_
#define _SC_ECC_H_
#include "sec_include_config.h"

#define CONFIG_SEC_CRYPTO_ECC

#ifdef CONFIG_SEC_CRYPTO_ECC

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SEC_LIB_VERSION
#include "drv/ecc.h"
#else
#include "ecc.h"
#endif

typedef enum {
    SC_ECC_PRIME256V1 = 0,
} sc_ecc_curve_type;

/**
\brief ECC ciphertext order
*/
typedef enum {
        SC_ECC_C1C3C2 = 0,
        SC_ECC_C1C2C3,
} sc_ecc_cipher_order_e;

typedef enum {
        SC_ECC_ENDIAN_LITTLE = 0, ///< Little Endian
        SC_ECC_ENDIAN_BIG         ///< Big Endian
} sc_ecc_endian_mode_e;

/**
\brief ECC key exchange role
*/
typedef enum { SC_ECC_Role_Sponsor = 0, SC_ECC_Role_Responsor } sc_ecc_exchange_role_e;

/****** ECC Event *****/
typedef enum {
        SC_ECC_EVENT_MAKE_KEY_COMPLETE = 0, ///< Make key completed
        SC_ECC_EVENT_ENCRYPT_COMPLETE,      ///< Encrypt completed
        SC_ECC_EVENT_DECRYPT_COMPLETE,      ///< Decrypt completed
        SC_ECC_EVENT_SIGN_COMPLETE,         ///< Sign completed
        SC_ECC_EVENT_VERIFY_COMPLETE,       ///< Verify completed
        SC_ECC_EVENT_EXCHANGE_KEY_COMPLETE, ///< Exchange key completed
} sc_ecc_event_e;

typedef struct {
        uint32_t ecc_curve : 1; ///< supports 256bits curve
} sc_ecc_capabilities_t;

/**
\brief ECC status
*/
typedef struct {
        uint32_t busy : 1; ///< Calculate busy flag
} sc_ecc_state_t;

typedef struct {
#ifdef CONFIG_CSI_V2
        csi_ecc_t ecc;
#endif
} sc_ecc_t;

///< Pointer to \ref sc_ecc_callback_t : ECC Event call back.
typedef void (*sc_ecc_callback_t)(sc_ecc_event_e event);

/**
  \brief       Initialize ECC.
  \param[in]   ecc  ecc handle to operate.
  \param[in]   idx  device id
  \return      \ref uint32_t
*/
uint32_t sc_ecc_init(sc_ecc_t *ecc, uint32_t idx);

/**
  \brief       De-initialize ECC Interface. stops operation and releases the
  software resources used by the interface \param[in]   ecc  ecc handle to
  operate. \return      none
*/
void sc_ecc_uninit(sc_ecc_t *ecc);

/**
  \brief       ecc get capability.
  \param[in]   ecc  Operate handle.
  \return      \ref uint32_t
*/
uint32_t sc_ecc_config(sc_ecc_t *ecc, sc_ecc_cipher_order_e co,
                       sc_ecc_endian_mode_e endian);

/**
  \brief       Attach the callback handler to ECC
  \param[in]   ecc  Operate handle.
  \param[in]   cb    Callback function
  \param[in]   arg   User can define it by himself as callback's param
  \return      Error code \ref uint32_t
*/
uint32_t sc_ecc_attach_callback(sc_ecc_t *ecc, sc_ecc_callback_t cb, void *arg);

/**
  \brief       Detach the callback handler
  \param[in]   ecc  Operate handle.
*/
uint32_t sc_ecc_detach_callback(sc_ecc_t *ecc);

/**
  \brief       ecc get capability.
  \param[in]   ecc  Operate handle.
  \param[out]   cap  Pointer of sc_ecc_capabilities_t.
  \return      \ref uint32_t
*/
uint32_t sc_ecc_get_capabilities(sc_ecc_t *ecc, sc_ecc_capabilities_t *cap);

uint32_t sc_ecc_check_keypair(sc_ecc_t *ecc, uint8_t pubkey[65],
                              uint8_t prikey[32]);

/**
  \brief       generate ecc key.
  \param[in]   ecc       ecc handle to operate.
  \param[out]  private   Pointer to the ecc private key, alloc by caller.
  \param[out]  public   Pointer to the ecc public key, alloc by caller.
  \return      \ref uint32_t
*/
uint32_t sc_ecc_gen_key(sc_ecc_t *ecc, uint8_t pubkey[65], uint8_t prikey[32]);


/**
  \brief       generate ecc pubkey.
  \param[in]   ecc      ecc handle to operate.
  \param[in]   prikey   Pointer to the ecc private key, alloc by caller.
  \param[out]  pubkey   Pointer to the ecc public key, alloc by caller.
  \return      \ref uint32_t
*/
uint32_t sc_ecc_gen_pubkey(sc_ecc_t *ecc, uint8_t pubkey[65], 
                    uint8_t prikey[32], sc_ecc_curve_type type);

/**
  \brief       ecc sign
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
uint32_t sc_ecc_sign(sc_ecc_t *ecc, uint8_t d[32], uint8_t prikey[32],
                     uint8_t s[64], sc_ecc_curve_type type);

/**
  \brief       ecc sign
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
uint32_t sc_ecc_sign_async(sc_ecc_t *ecc, uint8_t d[32], uint8_t prikey[32],
                           uint8_t s[64], sc_ecc_curve_type type);

/* TODO */
/**
  \brief       ecc verify
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool sc_ecc_verify(sc_ecc_t *ecc, uint8_t d[32], uint8_t pubkey[65],
                   uint8_t s[64], sc_ecc_curve_type type);

/**
  \brief       ecc verify
  \param[in]   ecc       ecc handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool sc_ecc_verify_async(sc_ecc_t *ecc, uint8_t d[32], uint8_t pubkey[65],
                         uint8_t s[64], sc_ecc_curve_type type);

/**
  \brief       ecc encrypto
  \param[in]   ecc       ecc handle to operate.
  \param[in]   plain       Pointer to the plaintext.
  \param[in]  PlainByteLen plaintext len
  \param[in]  pubKey public key.
  \param[out]  cipher Pointer to the chipher
  \param[out]  cipher_byte_len Pointer to the chipher len.
  \return      uint32_t
*/
uint32_t sc_ecc_encrypt(sc_ecc_t *ecc, uint8_t *plain, uint32_t plain_len,
                        uint8_t pubKey[65], uint8_t *cipher,
                        uint32_t *cipher_len);

/**
  \brief       ecc encrypto
  \param[in]   ecc       ecc handle to operate.
  \param[in]  cipher Pointer to the chipher
  \param[in]  CipherByteLen chipher len.
  \param[in]  prikey private key.
  \param[out]   plain       Pointer to the plaintext.
  \param[out]  PlainByteLen plaintext len
  \return      uint32_t
*/
uint32_t sc_ecc_decrypt(sc_ecc_t *ecc, uint8_t *cipher, uint32_t cipher_len,
                        uint8_t prikey[32], uint8_t *plain,
                        uint32_t *plain_len);

/**
  \brief       ecc key exchange
  \param[in]   ecc       ecc handle to operate.
  \return      uint32_t
*/
uint32_t sc_ecc_exchangekey(sc_ecc_t *ecc, sc_ecc_exchange_role_e role,
                            uint8_t *da, uint8_t *pb, uint8_t *ra1, uint8_t *ra,
                            uint8_t *rb, uint8_t *za, uint8_t *zb,
                            uint32_t k_len, uint8_t *ka, uint8_t *s1,
                            uint8_t *sa);

/**
  \brief       ecc key exchange get Z.
  \param[in]   ecc       ecc handle to operate.
  \return      uint32_t
*/
uint32_t sc_ecc_getZ(sc_ecc_t *ecc, uint8_t *id, uint32_t id_len,
                     uint8_t pubkey[65], uint8_t z[32]);

/**
  \brief       ecc key exchange get E
  \param[in]   ecc       ecc handle to operate.
  \return      uint32_t
*/
uint32_t sc_ecc_getE(sc_ecc_t *ecc, uint8_t *m, uint32_t len, uint8_t z[32],
                     uint8_t e[32]);

/**
  \brief       Get ECC state.
  \param[in]   ecc      ECC handle to operate.
  \param[out]  state    ECC state \ref sc_ecc_state_t.
  \return      Error code \ref uint32_t
*/
uint32_t sc_ecc_get_state(sc_ecc_t *ecc, sc_ecc_state_t *state);

#ifdef __cplusplus
extern "C" {
#endif

#endif

#endif /* _SC_ECC_H_ */

