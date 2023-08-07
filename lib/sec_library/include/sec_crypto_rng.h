/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     seccrypt_rng.h
 * @brief    Header File for RNG
 * @version  V1.0
 * @date     20. Jul 2020
 * @model    rng
 ******************************************************************************/
#ifndef _SC_RNG_H_
#define _SC_RNG_H_


#include <stdint.h>
#include "sec_crypto_errcode.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
  \brief       Get data from the TRNG engine
  \param[out]  data  Pointer to buffer with data get from TRNG
  \param[in]   size   Size of data items in bytes
  \return      error code
*/
uint32_t sc_rng_get_random_bytes(uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_TRNG_H_ */
