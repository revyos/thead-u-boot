#ifndef __SEC_ECIES_CRYPTO_H__
#define __SEC_ECIES_CRYPTO_H__
#ifdef _WIN32
#include "win_ctypes.h"
#else
#include <linux/types.h>
#endif
/**
  \brief       Ecies malloc
  \param[in]   size
  \return      error code \ref int
*/
void *csi_ecies_malloc(uint32_t size);

/**
  \brief       Ecies free
  \param[in]   buffer
  \return      error code \ref int
*/
void csi_ecies_free(void *buffer);

/**
  \brief       Ecies transmit apdu command message
  \param[in]   ss             session handle to operate
  \param[in]   apdu           Pointer to the apdu buf
  \param[in]   apduLen        Length of apdu buffer
  \param[out]  apduResponse   Pointer to the apduResponse buf
  \param[out]  apduResponseLen  Length of apduResponse buffer
  \return      error code \ref int
*/
int csi_ecies_transmit_apdu(void *ss, const uint8_t *apdu,
                                    uint32_t apduLen, uint8_t *apduResponse, uint32_t *apduResponseLen);

/**
  \brief       Ecies generate ec key pair
  \param[in]   ss             session handle to operate
  \param[out]  privateKey     Pointer to the private key buf
  \param[out]  privateKeyLen  Length of private key
  \param[out]  publicKey      Pointer to the public key buf
  \param[out]  publicKeyLen   Length of public key
  \return      error code \ref int
*/
int csi_ecies_generate_keypair(void *ss, uint8_t *privateKey, uint32_t *privateKeyLen,
                                uint8_t *publicKey, uint32_t *publicKeyLen);

/**
  \brief       Ecies generate ec share key
  \param[in]   ss             session handle to operate
  \param[in]   privateKey     Pointer to the private key buf
  \param[in]   privateKeyLen  Length of private key
  \param[in]   publicKey      Pointer to the public key buf
  \param[in]   publicKeyLen   Length of public key
  \param[out]  shareKey       Pointer to the share key buf
  \param[out]  shareKeyLen    Length of share key
  \return      error code \ref int
*/
int csi_ecies_generate_share_key(void *ss, const uint8_t *privateKey, uint32_t privateKeyLen,
                            const uint8_t *publicKey, uint32_t publicKeyLen, uint8_t *shareKey, uint32_t *shareKeyLen);

/**
  \brief       Ecies kdf ec share key
  \param[in]   ss           session handle to operate
  \param[in]   shareKey     Pointer to the share key buf
  \param[in]   shareKeyLen  Length of share key
  \param[in]   encKeyLen    Length of encrypt key
  \param[out]  encKey       Pointer to the encrypt key buf
  \param[in]   macKeyLen    Length of mac key
  \param[out]  macKey       Pointer to the mac key buf
  \return      error code \ref int
*/
int csi_ecies_kdf_key(void *ss, const uint8_t *shareKey, uint32_t shareKeyLen,
                            uint32_t encKeyLen, uint8_t *encKey, uint32_t macKeyLen, uint8_t *macKey);

/**
  \brief       Ecies encrypt and generate tag
  \param[in]   ss           session handle to operate
  \param[in]   length       Length of input buffer
  \param[in]   input        Pointer to the input buf
  \param[out]  output       Pointer to the output buf
  \param[in]   tagLen       Length of tag
  \param[out]  tag          Pointer to the tag buf
  \return      error code \ref int
*/
int csi_ecies_encrypt_and_tag(void *ss, uint32_t length, const uint8_t *input, uint8_t *output,
                                uint32_t tagLen, uint8_t *tag);

/**
  \brief       Ecies auth tag and decrypt
  \param[in]   ss           session handle to operate
  \param[in]   length       Length of input buffer
  \param[in]   tag          Pointer to the tag buf
  \param[in]   tagLen       Length of tag
  \param[in]   input        Pointer to the input buf
  \param[out]  output       Pointer to the output buf
  \return      error code \ref int
*/
int csi_ecies_auth_decrypt(void *ss, uint32_t length, const uint8_t *tag, uint32_t tagLen,
                                    const uint8_t *input, uint8_t *output);

#endif /* __SEC_ECIES_CRYPTO_H__ */
