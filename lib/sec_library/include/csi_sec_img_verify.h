/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#ifndef __CSI_SEC_IMG_VERIFY_H__
#define __CSI_SEC_IMG_VERIFY_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	BOOT_STAGE_BL1 = 0,
	BOOT_STAGE_BL2,
	BOOT_STAGE_ERR,
} boot_stage_t;

int csi_sec_custom_image_verify(unsigned long img_src_addr, unsigned long cur_hdr_addr);

int csi_sec_uboot_image_verify(unsigned long img_src_addr, unsigned long cur_hdr_addr);

int csi_sec_set_boot_stage(boot_stage_t boot_stage);

int csi_sec_get_lib_version(char ** p_version);

#ifdef __cplusplus
}
#endif

#endif	/* __CSI_SEC_IMG_VERIFY_H__ */