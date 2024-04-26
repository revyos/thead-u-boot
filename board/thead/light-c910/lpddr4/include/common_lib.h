#ifndef _COMMON_LIB_H_
#define _COMMON_LIB_H_

#include <stddef.h>
#include <stdio.h>

#include "ddr_reg_define.h"
#include "ddr_sysreg_registers_struct.h"
#include "ddr_sysreg_registers.h"
#include "aonsys_reg_define.h"
#include "aonsys_rstget_reg_define.h"
#include "define_ddr.h"
#include "DWC_ddr_umctl2_c_struct.h"
#include "DWC_ddr_umctl2_header.h"
#include "app_lib.h"
//#include "ddr_common_func.h"


void wr(u64 addr, u32 data); 
u32 rd(u64 addr);
void wr8(u64 addr, u8 data);
u8 rd8(u64 addr);
void wr16(u64 addr, u16 data);
u16 rd16(u64 addr);
//int putchar(int c);

#endif
