// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Liuw  <lw312886@alibaba-inc.com>
 *
 * U-Boot syscon driver for Thead's Platform Level Interrupt Controller (PLIC)
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/csr.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <asm/global_data.h>
#include <cpu.h>
#include <linux/err.h>
#include <asm/arch-thead/light-plic.h>

DECLARE_GLOBAL_DATA_PTR;

#define PLIC_BASE_GET(void)						\
	do {								\
		long *ret;						\
									\
		if (!gd->arch.plic) {					\
			ret = syscon_get_first_range(RISCV_SYSCON_PLIC); \
			gd->arch.plic = ret;				\
		}							\
	} while (0)

irq_handler_t irq_table[MAX_IRQ_NUM];

void __iomem *plic_base = NULL;

void external_interrupt(struct pt_regs *regs)
{
	void __iomem *claim;
	irq_handler_t handler;
	u32 irq_num;

	debug("[%s,%d]\n", __func__, __LINE__);
	if (!plic_base)
		return;

	debug("[%s,%d]\n", __func__, __LINE__);
	claim = PLIC_CLAIM_REG(plic_base, 0);

	while ((irq_num = readl(claim))) {
		if (irq_num >= MAX_IRQ_NUM)
			debug("Cannot find irq:%d\n", irq_num);
		else {
			handler = irq_table[irq_num];
			if (handler)
				handler();
			writel(irq_num, claim);
		}
	}
	debug("[%s,%d]\n", __func__, __LINE__);
}

static void plic_toggle(void __iomem *enable_base, int hwirq, int enable)
{
	u32 __iomem *reg = enable_base + (hwirq / 32) * sizeof(u32);
	u32 hwirq_mask = 1 << (hwirq % 32);

	if (enable)
		writel(readl(reg) | hwirq_mask, reg);
	else
		writel(readl(reg) & ~hwirq_mask, reg);

	debug("[%s,%d][0x%lx] = 0x%x\n", __func__, __LINE__,
			(unsigned long)reg, readl(reg));
}

static void plic_set_threshold(void __iomem *thre_base, u32 threshold)
{
	writel(threshold, thre_base);
	debug("[%s,%d][0x%lx] = 0x%x\n", __func__, __LINE__,
			(unsigned long)thre_base, readl(thre_base));
}

static void plic_set_irq_priority(void __iomem *prio_base, int prio)
{
	writel(prio, prio_base);
}

int irq_handler_register(int irq, irq_handler_t handler)
{
	if (irq < 0 || irq >= MAX_IRQ_NUM) {
		debug("invalid irq number to register\n");
		return -EINVAL;
	}

	irq_table[irq] = handler;

	return 0;
}

void arch_local_irq_enable(void)
{
	csr_set(CSR_MIE, MIE_MEIE);
	csr_set(CSR_MSTATUS, SR_MIE);
}

void arch_local_irq_disable(void)
{
	csr_clear(CSR_MIE, MIE_MEIE);
	csr_clear(CSR_MSTATUS, SR_MIE);
}

void irq_priority_set(int irq_id)
{
	plic_set_irq_priority(PLIC_PRIO_REG(gd->arch.plic, irq_id), 4);
}

void irq_enable(int hwirq)
{
	plic_toggle(PLIC_ENABLE_REG(gd->arch.plic, 0), hwirq, 1);
}

void irq_disable(int hwirq)
{
	plic_toggle(PLIC_ENABLE_REG(gd->arch.plic, 0), hwirq, 0);
}

int plic_init()
{
	PLIC_BASE_GET();
	if (IS_ERR(gd->arch.plic))
		return PTR_ERR(gd->arch.plic);

	plic_base = gd->arch.plic;
	debug("THEAD PLIC BASE: 0x%lx\n", (unsigned long)gd->arch.plic);

	plic_set_threshold(PLIC_THRESHOLD_REG(gd->arch.plic, 0), 0);

	arch_local_irq_enable(); //enale the global interrupt

	return 0;
}

static const struct udevice_id thead_plic_ids[] = {
	{ .compatible = "riscv,plic0", .data = RISCV_SYSCON_PLIC},
	{ }
};

U_BOOT_DRIVER(thead_plic) = {
	.name		= "thead_light_plic",
	.id		= UCLASS_SYSCON,
	.of_match	= thead_plic_ids,
};
