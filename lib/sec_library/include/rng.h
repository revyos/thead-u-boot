/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     drv/tng.h
 * @brief    Header File for RNG Driver
 * @version  V1.0
 * @date     22. Apr 2020
 * @model    tng
 ******************************************************************************/
#ifndef _DRV_TNG_H_
#define _DRV_TNG_H_

#include "common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  \brief       Get data from the TNG engine
  \param[out]  Data  Pointer to buffer with data get from TNG
  \param[in]   Size   Size of data items,uinit in uint32
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_rng_get_random_bytes(uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* _DRV_TNG_H_ */
