/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/des.h
 * @brief      Header File for DES Driver
 * @version    V1.0
 * @date       24. Oct 2022
 * @model      des
 ******************************************************************************/

#ifndef _DRV_DES_H_
#define _DRV_DES_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----- Encrypt & Decrypt: Config key length -----*/
/**
\brief DES data transfer mode config
*/
typedef enum {
    DES_SLAVE_MODE = 0U,         /*slave mode*/
    DES_DMA_MODE,                /*dma mode*/
} csi_des_trans_mode_t;

/**
\brief DES key-len-bits type
*/
typedef enum {
    DES_KEY_LEN_BITS_64        = 0,        /*64 Data bits*/
    DES_KEY_LEN_BITS_128,                  /*128 Data bits*/
    DES_KEY_LEN_BITS_192,                  /*192 Data bits*/
} csi_des_key_bits_t;

typedef enum{
    DES_MODE_ECB    = 0x00000000,
    DES_MODE_CBC    = 0x20000020,
    TDES_MODE_ECB   = 0x00000008,
    TDES_MODE_CBC   = 0x20000028,
} des_mode_t;


#define DES_KEY_LEN_BYTES_32    (32)
#define DES_KEY_LEN_BYTES_16    (16)
#define DES_KEY_LEN_BYTES_24    (24)
#define DES_KEY_LEN_BYTES_8     (8)

#define DES_BLOCK_IV_SIZE       (8)
#define DES_BLOCK_CRYPTO_SIZE   (8)
#define TDES_BLOCK_CRYPTO_SIZE  (16)

#define DES_DIR_ENCRYPT         (1)
#define DES_DIR_DECRYPT         (0)

#define DES_KEY_128_BITS        (0x8)
#define DES_KEY_192_BITS        (0x10)

/**
\brief DES State
*/
typedef struct {
    uint32_t busy             : 1;        /*Calculate busy flag*/
    uint32_t error            : 1;        /*Calculate error flag*/
} csi_des_state_t;

/**
\brief DES Context
*/
typedef struct {
    uint32_t            key_len_byte;
    uint8_t             key[32];          /*Data block being processed*/
    uint32_t            sca;
    uint32_t            is_kdf;
    uint32_t            is_dma;
} csi_des_context_t;

/**
\brief DES Ctrl Block
*/
typedef struct {
    csi_des_state_t         state;
    csi_des_context_t       context;
    csi_dev_t               dev;
    void                    *priv;
} csi_des_t;

/**
  \brief       Initialize DES interface. Initializes the resources needed for the DES interface
  \param[in]   des    Handle to operate
  \param[in]   idx    Device id
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_des_init(csi_des_t *des, uint32_t idx);

/**
  \brief       De-initialize DES interface. Stops operation and releases the software resources used by the interface
  \param[in]   des    Dandle to operate
  \return      None
*/
void csi_des_uninit(csi_des_t *des);

/**
  \brief       Set encrypt key
  \param[in]   des        Handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_des_key_bits_t
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_des_set_encrypt_key(csi_des_t *des, void *key, csi_des_key_bits_t key_len);

/**
  \brief       Set decrypt key
  \param[in]   des        Handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_des_key_bits_t
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_des_set_decrypt_key(csi_des_t *des, void *key, csi_des_key_bits_t key_len);

/**
  \brief       DES ecb encrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_des_ecb_encrypt(csi_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       DES ecb decrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_des_ecb_decrypt(csi_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       DES cbc encrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_des_cbc_encrypt(csi_des_t *des, void *in, void *out, uint32_t size, void *iv) ;

/**
  \brief       DES cbc decrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_des_cbc_decrypt(csi_des_t *des, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       TDES ecb encrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_tdes_ecb_encrypt(csi_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       TDES ecb decrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_tdes_ecb_decrypt(csi_des_t *des, void *in, void *out, uint32_t size);

/**
  \brief       TDES cbc encrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_tdes_cbc_encrypt(csi_des_t *des, void *in, void *out, uint32_t size, void *iv) ;

/**
  \brief       TDES cbc decrypt
  \param[in]   des     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_tdes_cbc_decrypt(csi_des_t *des, void *in, void *out, uint32_t size, void *iv);

/**
  \brief       Config DES mode dma or slave
  \param[in]   mode    \ref csi_des_trans_mode_t 
  \return      None
*/
csi_error_t csi_des_trans_config(csi_des_t *des, csi_des_trans_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_AES_H_ */
