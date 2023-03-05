// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#define AONSYS_RSTGEN_BASE			((void __iomem *)0xFFFFF44000UL)
#define REG_RST_REQ_EN_0			(AONSYS_RSTGEN_BASE + 0x140)
#define WDT0_SYS_RST_REQ			(1 << 8)

static __attribute__((naked))void sys_wdt_reset(void)
{
    uint32_t data;

	/* wdt0 reset enable */
    data = readl(REG_RST_REQ_EN_0);
	data |= WDT0_SYS_RST_REQ;
	writel(data, REG_RST_REQ_EN_0);

    asm volatile (
    "1: \n\r"
        "li      a0, 0xFFEFC30000 \n\r"
        "li      a1, 1          \n\r"
        "sw      a1, 0(a0)      \n\r"
        "sw      a1, 4(a0)      \n\r"
        "j      1b              \n\r"
        "ret                    \n\r"
    );
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    printf("resetting ...\n");

	sys_wdt_reset();
	hang();

	return 0;
}
