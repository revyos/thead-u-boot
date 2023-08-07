/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     seccrypt_sha.h
 * @brief    Header File for SHA
 * @version  V1.0
 * @date     20. Jul 2020
 * @model    sha
 ******************************************************************************/
#ifndef _SC_SHA_H_
#define _SC_SHA_H_
#include "sec_include_config.h"

#include <stdint.h>
#ifdef CONFIG_SYSTEM_SECURE
#ifdef SEC_LIB_VERSION
#include "drv/sha.h"
#else
#include "sha.h"
#endif
#include "soc.h"
#endif
#ifdef CONFIG_SEC_CRYPTO_SM3
#ifdef SEC_LIB_VERSION
#include "drv/sm3.h"
#else
#include "sm3.h"
#endif
#endif

#include "sec_crypto_errcode.h"


#ifdef CONFIG_SEC_CRYPTO_SHA_SW
#include "crypto_sha1.h"
#include "crypto_sha256.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
\brief SHA data transfer mode config
*/
typedef enum {
    SC_SHA_SLAVE_MODE = 0U,         /*slave mode*/
    SC_SHA_DMA_MODE,                /*dma mode*/
} sc_sha_trans_mode_t;

/*----- SHA Control Codes: Mode -----*/
typedef enum {
    SC_SHA_MODE_SHA1 = 1U,  /*SHA_1 mode*/
    SC_SHA_MODE_256,        /*SHA_256 mode*/
    SC_SHA_MODE_224,        /*SHA_224 mode*/
    SC_SHA_MODE_512,        /*SHA_512 mode*/
    SC_SHA_MODE_384,        /*SHA_384 mode*/
    SC_SHA_MODE_512_256,    /*SHA_512_256 mode*/
    SC_SHA_MODE_512_224,    /*SHA_512_224 mode*/
    SC_SHA_MODE_MD5,        /*MD5 mode*/
    SC_SM3_MODE,
} sc_sha_mode_t;

/**
\brief SHA State
*/
typedef struct {
    uint32_t busy : 1;  /*calculate busy flag*/
    uint32_t error : 1; /*calculate error flag*/
} sc_sha_state_t;

typedef struct {
#ifdef CONFIG_SYSTEM_SECURE
#ifdef CONFIG_CSI_V1
uint8_t ctx[SHA_CONTEXT_SIZE];
#endif /* CONFIG_CSI_V1 */
#ifdef CONFIG_CSI_V2
  csi_sha_context_t ctx;
  csi_sm3_context_t sm3ctx;
  csi_sha_state_t   state;
  csi_sm3_state_t   sm3state;
#endif
#endif
#if defined(CONFIG_TEE_CA)
    uint8_t ctx[224+8];
#endif
#if defined(CONFIG_SEC_CRYPTO_SHA_SW)
  sc_mbedtls_sha1_context sha1_ctx;
  sc_mbedtls_sha256_context sha2_ctx;
#endif
  sc_sha_mode_t mode;        /*sha mode*/
} sc_sha_context_t;

/****** SHA Event *****/
typedef enum {
    SC_SHA_EVENT_COMPLETE = 0U, /*calculate completed*/
    SC_SHA_EVENT_ERROR          /*calculate error*/
} sc_sha_event_t;

typedef struct sc_sha {
#ifdef CONFIG_SYSTEM_SECURE
#ifdef CONFIG_CSI_V1
  sha_handle_t handle;
  sc_sha_context_t ctx;
  sc_sha_mode_t mode;        /*sha mode*/
#endif /* CONFIG_CSI_V1 */
#ifdef CONFIG_CSI_V2
  csi_sha_t csi_sha;
  csi_sm3_t csi_sm3;
#endif
#endif
#if defined(CONFIG_TEE_CA)
#endif
} sc_sha_t;

// Function documentation

/**
  \brief       Initialize SHA Interface. Initializes the resources needed for the SHA interface
  \param[in]   sha  operate handle.
  \param[in]   idx index of sha
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_init(sc_sha_t *sha, uint32_t idx);

/**
  \brief       De-initialize SHA Interface. stops operation and releases the software resources used by the interface
  \param[in]   sha  sha handle to operate.
  \return      none
*/
void sc_sha_uninit(sc_sha_t *sha);

/**
  \brief       attach the callback handler to SHA
  \param[in]   sha      operate handle.
  \param[in]   callback callback function
  \param[in]   arg      callback's param
  \return      error code
*/
uint32_t sc_sha_attach_callback(sc_sha_t *sha, void *callback, void *arg);

/**
  \brief       detach the callback handler
  \param[in]   sha  operate handle.
*/
void sc_sha_detach_callback(sc_sha_t *sha);

/**
  \brief       start the engine
  \param[in]   sha     sha handle to operate.
  \param[in]   context Pointer to the sha context \ref sc_sha_context_t
  \param[in]   mode    sha mode \ref sc_sha_mode_t
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_start(sc_sha_t *sha, sc_sha_context_t *context, sc_sha_mode_t mode);

/**
  \brief       update the engine
  \param[in]   sha     sha handle to operate.
  \param[in]   context Pointer to the sha context \ref sc_sha_context_t
  \param[in]   input   Pointer to the Source data
  \param[in]   size    the data size
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_update(sc_sha_t *sha, sc_sha_context_t *context, const void *input, uint32_t size);

/**
  \brief       accumulate the engine (async mode)
  \param[in]   sha     sha handle to operate.
  \param[in]   context Pointer to the sha context \ref sc_sha_context_t
  \param[in]   input   Pointer to the Source data
  \param[in]   size    the data size
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_update_async(sc_sha_t *sha, sc_sha_context_t *context, const void *input, uint32_t size);

/**
  \brief       finish the engine
  \param[in]   sha      sha handle to operate.
  \param[in]   context  Pointer to the sha context \ref sc_sha_context_t
  \param[out]  output   Pointer to the result data
  \param[out]  out_size Pointer to the result data size(bytes)
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_finish(sc_sha_t *sha, sc_sha_context_t *context, void *output, uint32_t *out_size);

/**
  \brief       calculate the digest
  \param[in]   sha      sha handle to operate.
  \param[in]   idx      index of sha
  \param[in]   context  Pointer to the sha context \ref sc_sha_context_t
  \param[in]   mode     sha mode \ref sc_sha_mode_t
  \param[in]   input    Pointer to the Source data
  \param[in]   size     the data size
  \param[out]  output   Pointer to the result data
  \param[out]  out_size Pointer to the result data size(bytes)
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_digest(sc_sha_t *sha, uint32_t idx, sc_sha_context_t *context, sc_sha_mode_t mode,
                        const void *input, uint32_t size, void *output, uint32_t out_size);            

/**
  \brief       finish the engine
  \param[in]   sha      sha handle to operate.
  \param[in]   context  Pointer to the sha context \ref sc_sha_context_t
  \return      error code \ref uint32_t
*/
uint32_t sc_sha_get_state(sc_sha_t *sha,sc_sha_context_t *context);

/**
  \brief       Sha data transfer config
*/
uint32_t sc_sha_trans_config(sc_sha_t *sha, sc_sha_context_t *context, sc_sha_trans_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* _sc_SHA_H_ */
