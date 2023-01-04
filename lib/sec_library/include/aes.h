/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/aes.h
 * @brief      Header File for AES Driver
 * @version    V1.0
 * @date       9. Oct 2020
 * @model      aes
 ******************************************************************************/

#ifndef _DRV_AES_H_
#define _DRV_AES_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----- Encrypt & Decrypt: Config key length -----*/
typedef enum {
    AES_KEY_LEN_BITS_128        = 0,       ///< 128 Data bits
    AES_KEY_LEN_BITS_192,                  ///< 192 Data bits
    AES_KEY_LEN_BITS_256                   ///< 256 Data bits
} csi_aes_key_bits_t;

typedef enum{
    AES_MODE_ECB = 0,
    AES_MODE_CBC = 0x20000020,
    AES_MODE_CTR = 0x200001c0,
    AES_MODE_CFB = 0x20000400,
    AES_MODE_GCM = 0x20030040,
    AES_MODE_CCM = 0x21D40040,
    AES_MODE_OFB = 0x24000000,
} aes_mode_t;

#define AES_KEY_LEN_BYTES_32 32
#define AES_KEY_LEN_BYTES_24 24
#define AES_KEY_LEN_BYTES_16 16

#define AES_CRYPTO_CTRL_CBC_256 0x20000038
#define AES_CRYPTO_CTRL_CBC_192 0x20000030
#define AES_CRYPTO_CTRL_CBC_128 0x20000028
#define AES_CRYPTO_CTRL_ECB_256 0x00000018
#define AES_CRYPTO_CTRL_ECB_192 0x00000010
#define AES_CRYPTO_CTRL_ECB_128 0x00000008

#define AES_BLOCK_IV_SIZE  16
#define AES_BLOCK_TAG_SIZE  16
#define AES_BLOCK_CRYPTO_SIZE  16

#define AES_DIR_ENCRYPT    1
#define AES_DIR_DECRYPT    0

#define KEY_128_BITS 0x8
#define KEY_192_BITS 0x10
#define KEY_256_BITS 0x18

#define AES_DMA_ENABLE  1
#define AES_DMA_DISABLE 0


typedef enum{
    AES_CRYPTO_ECB_256_MODE = 0,
    AES_CRYPTO_ECB_192_MODE,
    AES_CRYPTO_ECB_128_MODE,
    AES_CRYPTO_CBC_256_MODE,
    AES_CRYPTO_CBC_192_MODE,
    AES_CRYPTO_CBC_128_MODE,
} csi_aes_mode_t;

typedef struct {
    uint32_t busy             : 1;        ///< Calculate busy flag
    uint32_t error            : 1;        ///< Calculate error flag
} csi_aes_state_t;

typedef struct {
    uint32_t            key_len_byte;
    uint8_t             key[32];          ///< Data block being processed
    uint32_t            sca;
    uint32_t            is_kdf;
    uint32_t            is_dma;
} csi_aes_context_t;

/**
\brief AES Ctrl Block
*/
typedef struct {
    csi_aes_state_t         state;
    csi_aes_context_t       context;
    csi_dev_t               dev;
    void                    *priv;
} csi_aes_t;

/**
  \brief       Initialize AES interface. Initializes the resources needed for the AES interface
  \param[in]   aes    Handle to operate
  \param[in]   idx    Device id
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_init(csi_aes_t *aes, uint32_t idx);

/**
  \brief       De-initialize AES interface. Stops operation and releases the software resources used by the interface
  \param[in]   aes    Dandle to operate
  \return      None
*/
void csi_aes_uninit(csi_aes_t *aes);

/**
  \brief       Set encrypt key
  \param[in]   aes        Handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_aes_key_bits_t
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_set_encrypt_key(csi_aes_t *aes, void *key, csi_aes_key_bits_t key_len);

/**
  \brief       Set decrypt key
  \param[in]   aes        Handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_aes_key_bits_t
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_set_decrypt_key(csi_aes_t *aes, void *key, csi_aes_key_bits_t key_len);

/**
  \brief       AES ecb encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_ecb_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size);

/**
  \brief       AES ecb decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_ecb_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size);

/**
  \brief       AES cbc encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cbc_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv) ;

/**
  \brief       AES cbc decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cbc_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       AES cfb1 encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb1_encrypt(csi_aes_t *aes, void *in, void *out,  uint32_t size, void *iv);

/**
  \brief       AES cfb1 decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb1_decrypt(csi_aes_t *aes, void *in, void *out,  uint32_t size, void *iv);

/**
  \brief       AES cfb8 encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb8_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       AES cfb8 decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb8_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       AES cfb128 decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb128_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       AES cfb128 encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb128_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       AES ofb encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \param[in]  key_len key bits
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ofb_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Aes ofb decrypt
  \param[in]   dev_aes     dev_aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[in]  key_len key bits
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ofb_decrypt(csi_aes_t *aes, void *in, void *out,uint32_t size, void *iv);

/**
  \brief       AES ctr encrypt
  \param[in]   aes              Handle to operate
  \param[in]   in               Pointer to the source data
  \param[out]  out              Pointer to the result data
  \param[in]   size             The source data size
  \param[in]   iv               Init vector
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ctr_encrypt(csi_aes_t *aes,void *in,void *out,uint32_t size,void *iv);

/**
  \brief       AES ctr decrypt
  \param[in]   aes              Handle to operate
  \param[in]   in               Pointer to the source data
  \param[out]  out              Pointer to the result data
  \param[in]   size             The source data size
  \param[in]   iv               Init vecotr
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ctr_decrypt(csi_aes_t *aes,void *in,void *out,uint32_t size,void *iv);

/**
  \brief       Aes gcm encrypt
  \param[in]   dev_aes              dev_aes handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vector
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_gcm_encrypt(csi_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv);

/**
  \brief       Aes gcm decrypt
  \param[in]   dev_aes              dev_aes handle to operate
  \param[in]   in               Pointer to the Source data.
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vecotr
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_gcm_decrypt(csi_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv);

/**
  \brief       Aes ccm encrypt
  \param[in]   dev_aes              dev_aes handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vector
  \param[in]   tag_out          tag output
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ccm_encrypt(csi_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv, uint8_t *tag_out);

/**
  \brief       Aes ccm decrypt
  \param[in]   dev_aes              dev_aes handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   iv               init vecotr
  \param[in]   tag_out          tag output
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ccm_decrypt(csi_aes_t *aes, void *in, void *out,uint32_t size, uint32_t add_len, void *iv, uint8_t *tag_out);

/**
  \brief       Enable AES power manage
  \param[in]   aes    Handle to operate
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_enable_pm(csi_aes_t *aes);

/**
  \brief       Disable AES power manage
  \param[in]   aes    Handle to operate
  \return      None
*/
void csi_aes_disable_pm(csi_aes_t *aes);

/**
  \brief       Config AES mode dma or slave
  \param[in]   dam_en    zero disable dma, not zero enable dma 
  \return      None
*/
void csi_aes_dma_enable(csi_aes_t *aes, uint8_t dma_en);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_AES_H_ */
