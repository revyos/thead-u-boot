/*
* Copyright (C) 2017-2020 Alibaba Group Holding Limited
*
* SPDX-License-Identifier: GPL-2.0+
*/

#include <asm/asm.h>
#include <asm/io.h>
#include <common.h>

void init_ddr(void)
{
	writel(0x1ff << 4, (void *)0xffff005000);
}

int fixup_ddr_addrmap(unsigned long size)
{
	return 0;
}

int query_ddr_boundary(unsigned long size)
{
	return 0;
}
unsigned long get_ddr_density(void)
{
	return 0x100000000;
}
