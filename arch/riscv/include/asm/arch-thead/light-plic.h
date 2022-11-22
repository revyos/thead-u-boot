// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022, Liuw  <lw312886@alibaba-inc.com>
 *
 * U-Boot syscon driver for Thead's Platform Level Interrupt Controller (PLIC)
 */

#ifndef _LIGHT_PLIC_H
#define _LIGHT_PLIC_H

/*
 * M-mode
 *	hart id: 0, 2, 4, 6
 * S-mode
 *	hart id: 1, 3, 5, 7
 */

/* interrupt priority register */
#define PLIC_PRIO_REG(base, id)			((void __iomem *)(base) + 0x00 + (id) * 4)

/* enable register */
#define PLIC_ENABLE_REG(base, hart)		((void __iomem *)(base) + 0x2000 + (hart) * 0x80)

/* pending registr */
#define PLIC_PENDING_REG(base, hart)		((void __iomem *)(base) + 0x1000 + ((hart) / 4) * 4)

/* threshold register */
#define PLIC_THRESHOLD_REG(base, hart)		((void __iomem *)(base) + 0x200000 + (hart) * 0x1000 + 0x00)

/* claim/complete register */
#define PLIC_CLAIM_REG(base, hart)		((void __iomem *)(base) + 0x200000 + (hart) * 0x1000 + 0x04)

#define MAX_IRQ_NUM	256
typedef void (*irq_handler_t)(void);


int irq_handler_register(int irq, irq_handler_t handler);

void arch_local_irq_enable(void);

void arch_local_irq_disable(void);

void irq_enable(int hwirq);

void irq_disable(int hwirq);

void irq_priority_set(int prio);

int plic_init(void);

#endif
