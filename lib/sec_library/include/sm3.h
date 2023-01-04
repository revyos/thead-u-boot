/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       drv/sm3.h
 * @brief      Header File for SM3 Driver
 * @version    V2.0
 * @date       9. DEC 2020
 * @model      SM3
 ******************************************************************************/

#ifndef _DRV_SM3_H_
#define _DRV_SM3_H_

#include <stdint.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SM3_DATAIN_BLOCK_SIZE 64
#define SM3_DIGEST_OUT_SIZE   32

typedef struct {
    uint32_t total[2];   ///< Number of bytes processed
    uint32_t state[16];  ///< Intermediate digest state
    uint8_t  buffer[SM3_DATAIN_BLOCK_SIZE]; ///< Data block  beingprocessed
    uint8_t  result[SM3_DIGEST_OUT_SIZE]; ///< Data block has processed
} csi_sm3_context_t;

/****** SM3 State ******/
typedef struct {
    uint32_t busy             : 1;        ///< Calculate busy flag
    uint32_t error            : 1;        ///< Calculate error flag
} csi_sm3_state_t;


/****** SM3 Event ******/
typedef enum {
    SM3_EVENT_COMPLETE = 0U, ///< Calculate completed
    SM3_EVENT_UPDATE,
    SM3_EVENT_START,
    SM3_EVENT_ERROR          ///< Calculate error
} csi_sm3_event_t;

typedef struct csi_sm3_t csi_sm3_t;

struct csi_sm3_t {
    csi_dev_t dev; ///< SM3 hw-device info
    void (*callback)(csi_sm3_t *sm3, csi_sm3_event_t event,
                        void *arg); ///< SM3 event callback for user
    void *          arg;    ///< SM3 custom designed param passed to evt_cb
    csi_sm3_state_t state;  ///< SM3 state
    void *          priv;
};

// Function documentation

/**
  \brief       Initialize SM3 Interface. Initializes the resources needed for the SM3 interface
  \param[in]   sm3  operate handle.
  \param[in]   idx index of sm3
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_init(csi_sm3_t *sm3, uint32_t idx);

/**
  \brief       De-initialize SM3 Interface. stops operation and releases the software resources used by the interface
  \param[in]   sm3  sm3 handle to operate.
  \return      none
*/
void csi_sm3_uninit(csi_sm3_t *sm3);

/**
  \brief       Attach the callback handler to SM3
  \param[in]   sm3         Handle to operate
  \param[in]   callback    Callback function
  \param[in]   arg         Callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_attach_callback(csi_sm3_t *sm3, void *callback, void *arg);

/**
  \brief       Detach the callback handler
  \param[in]   sm3    Handle to operate
  \return      None
*/
void csi_sm3_detach_callback(csi_sm3_t *sm3);

/**
  \brief       start the engine
  \param[in]   sm3     sm3 handle to .operate
  \param[in]   context Pointer to the sm3 context \ref csi_sm3_context_t
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_start(csi_sm3_t *sm3, csi_sm3_context_t *context);

/**
  \brief       update the engine
  \param[in]   sm3     sm3 handle to operate.
  \param[in]   context Pointer to the sm3 context \ref csi_sm3_context_t
  \param[in]   input   Pointer to the Source data
  \param[in]   size    the data size
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_update(csi_sm3_t *sm3, csi_sm3_context_t *context,
                           const uint8_t *input, uint32_t size);

/**
  \brief       Accumulate the engine (async mode)
  \param[in]   sm3        Handle to operate
  \param[in]   context    Pointer to the SM3 context \ref csi_sm3_context_t
  \param[in]   input      Pointer to the Source data
  \param[in]   size       The data size
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_update_async(csi_sm3_t *sm3, csi_sm3_context_t *context,
                                 const uint8_t *input, uint32_t size);

/**
  \brief       finish the engine
  \param[in]   sm3      sm3 handle to operate.
  \param[in]   context  Pointer to the sm3 context \ref csi_sm3_context_t
  \param[out]  output   Pointer to the result data
  \param[out]  out_size Pointer to the result data size(bytes)
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_finish(csi_sm3_t *sm3, csi_sm3_context_t *context,
                           uint8_t *output, uint32_t *out_size);

/**
  \brief       Get SM3 state
  \param[in]   sm3      Handle to operate
  \param[out]  state    SM3 state \ref csi_sm3_state_t
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_get_state(csi_sm3_t *sm3, csi_sm3_state_t *state);

/**
  \brief       Enable SM3 power manage
  \param[in]   sm3     Handle to operate
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_enable_pm(csi_sm3_t *sm3);

/**
  \brief       Disable SM3 power manage
  \param[in]   sm3    Handle to operate
  \return      None
*/
void csi_sm3_disable_pm(csi_sm3_t *sm3);

#ifdef __cplusplus
extern "C" {
#endif

#endif //_DRV_SM3_H