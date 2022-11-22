/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/types.h>
#include <thead/clock_config.h>
#include <linux/bitops.h>
#include <asm/arch-thead/light-iopmp.h>
#include <asm/arch-thead/light-plic.h>

#define DW_TIMER0_BASE		0xffefc32000
#define DW_TIMER0_TLC_REG	(DW_TIMER0_BASE + 0x00)		/* Offset: 0x000 (R/W) TimerLoadCount */
#define DW_TIMER0_TCV_REG	(DW_TIMER0_BASE + 0X04)		/* Offset: 0x004 (R/ ) TimerCurrentValue */
#define DW_TIMER0_TCR_REG       (DW_TIMER0_BASE + 0X08)         /* Offset: 0x008 (R/W) TimerControlReg */
#define DW_TIMER0_TEOI_REG      (DW_TIMER0_BASE + 0X0C)         /* Offset: 0x00c (R/ ) TimerEOI */
#define DW_TIMER0_TIS_REG       (DW_TIMER0_BASE + 0X10)         /* Offset: 0x010 (R/ ) TimerIntStatus */

/*! Timer Int Status,     offset: 0x10 */
#define DW_TIMER_INT_STATUS_Pos                                        (0U)
#define DW_TIMER_INT_STATUS_Msk                                        (0x1U << DW_TIMER_INT_STATUS_Pos)
#define DW_TIMER_INT_STATUS_EN                                         DW_TIMER_INT_STATUS_Msk

/*! Timer1 Control Reg,	       offset: 0x08 */
#define DW_TIMER_CTL_ENABLE_SEL_Pos                                    (0U)
#define DW_TIMER_CTL_ENABLE_SEL_Msk                                    (0x1U << DW_TIMER_CTL_ENABLE_SEL_Pos)
#define DW_TIMER_CTL_ENABLE_SEL_EN                                     DW_TIMER_CTL_ENABLE_SEL_Msk

#define DW_TIMER_CTL_MODE_SEL_Pos                                      (1U)
#define DW_TIMER_CTL_MODE_SEL_Msk                                      (0x1U << DW_TIMER_CTL_MODE_SEL_Pos)
#define DW_TIMER_CTL_MODE_SEL_EN                                       DW_TIMER_CTL_MODE_SEL_Msk

#define DW_TIMER_CTL_INT_MASK_Pos                                      (2U)
#define DW_TIMER_CTL_INT_MASK_Msk                                      (0x1U << DW_TIMER_CTL_INT_MASK_Pos)
#define DW_TIMER_CTL_INT_MAKS_EN                                       DW_TIMER_CTL_INT_MASK_Msk

#define DW_TIMER_CTL_HARD_TRIG_Pos                                     (4U)
#define DW_TIMER_CTL_HARD_TRIG_Msk                                     (0x1U << DW_TIMER_CTL_HARD_TRIG_Pos)
#define DW_TIMER_CTL_HARD_TRIG_EN                                      DW_TIMER_CTL_HARD_TRIG_Msk

/*! Timer EOI,            offset: 0x0c */
#define DW_TIMER_EOI_REG_Pos                                           (0U)
#define DW_TIMER_EOI_REG_Msk                                           (0x1U << DW_TIMER_EOI_REG_Pos)
#define DW_TIMER_EOI_REG_EN                                            DW_TIMER_EOI_REG_Msk

#define TIMER0_IRQ_NUM	16
#define TIMER0_FREQ_HZ	125000000U
#define DW_TIMER_GET_RELOAD_VAL(_tim_, _frq_)      ((_tim_ < 25000U) ? ((_frq_ * _tim_) / 1000U) : (_frq_ * (_tim_ / 1000U)))

static int time_user_defined_flag = 0;

static void csi_timer_stop(void);

static inline u32 dw_timer_get_int_status(void)
{
	return (readl((void __iomem *)DW_TIMER0_TIS_REG) & DW_TIMER_INT_STATUS_EN) ?  1 : 0;
}

static inline void dw_timer_clear_irq(void)
{
	readl((void __iomem *)DW_TIMER0_TEOI_REG);
}

static inline void dw_timer_write_load(uint32_t value)
{
	writel(value, (void __iomem *)DW_TIMER0_TLC_REG);
}

static inline void dw_timer_set_mode_load(void)
{
	writel((readl((void __iomem *)DW_TIMER0_TCR_REG) | DW_TIMER_CTL_MODE_SEL_EN), (void __iomem *)DW_TIMER0_TCR_REG);
}

static inline void dw_timer_set_disable(void)
{
	u32 data = readl((void __iomem *)DW_TIMER0_TCR_REG);

	data &= ~DW_TIMER_CTL_ENABLE_SEL_EN;
	writel(data, (void __iomem *)DW_TIMER0_TCR_REG);
}

static inline void dw_timer_set_enable(void)
{
	u32 data = readl((void __iomem *)DW_TIMER0_TCR_REG);

	data |= DW_TIMER_CTL_ENABLE_SEL_EN;
	writel(data, (void __iomem *)DW_TIMER0_TCR_REG);
}

static inline void dw_timer_set_unmask(void)
{
	u32 data = readl((void __iomem *)DW_TIMER0_TCR_REG);

	data &= ~DW_TIMER_CTL_INT_MAKS_EN;
	writel(data, (void __iomem *)DW_TIMER0_TCR_REG);
}

static inline void dw_timer_set_mask(void)
{
	u32 data = readl((void __iomem *)DW_TIMER0_TCR_REG);

	data |= DW_TIMER_CTL_INT_MAKS_EN;
	writel(data, (void __iomem *)DW_TIMER0_TCR_REG);
}

static void dw_timer_irq_handler(void)
{
	debug("[%s,%d]\n", __func__, __LINE__);
	if (dw_timer_get_int_status()) {
		dw_timer_clear_irq();
		csi_timer_stop();
		debug("[%s,%d]\n", __func__, __LINE__);
		time_user_defined_flag = 1;
	}
}

static inline void dw_timer_reset_register(void)
{
	writel(0, (void __iomem *)DW_TIMER0_TLC_REG);
	writel(0, (void __iomem *)DW_TIMER0_TCV_REG);
}

static int csi_timer_start(u32 timeout_us)
{
	u32 timer_freq = TIMER0_FREQ_HZ;
	u32 tmp_load = DW_TIMER_GET_RELOAD_VAL(timeout_us, timer_freq);

	dw_timer_set_mode_load();

	//FIXME: no less than 10
	if (tmp_load < 10)
		tmp_load = 10;

	dw_timer_write_load(tmp_load);

	dw_timer_set_disable();
	dw_timer_set_enable();
	dw_timer_set_unmask();

	return 0;
}

static void csi_timer_stop(void)
{
	dw_timer_set_mask();
	dw_timer_set_disable();
}

static void timer_interrupt_init(void)
{
	irq_handler_register(TIMER0_IRQ_NUM, dw_timer_irq_handler);
	irq_priority_set(TIMER0_IRQ_NUM);
	irq_enable(TIMER0_IRQ_NUM);
	arch_local_irq_enable();
}

static void timer_interrupt_uninit(void)
{
	arch_local_irq_disable();
	irq_disable(TIMER0_IRQ_NUM);
}

static int csi_timer_init(void)
{
	dw_timer_reset_register();
	timer_interrupt_init();
	return 0;
}

static void csi_timer_uinit(void)
{
	timer_interrupt_uninit();
	dw_timer_reset_register();
}

int timer_alarm_set(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned long time_us;
	int ret, state;
	u32 timeout = 0;

	if (argc != 2) {
		printf("invalid input parameters\n");
		return -EINVAL;
	}

	if (strict_strtoul(argv[1], 10, &time_us) < 0)
		return CMD_RET_USAGE;

	time_us = time_us * 1000000;
	ret = csi_timer_init();
	if(ret) {
		printf("failed to initialize the timer\n");
		return -EINVAL;
	}

	time_user_defined_flag = 0;
	state = csi_timer_start(time_us);
	if (state) {
		printf("failed to start the timer0\n");
		return ret;
	}

	do {

		timeout++;
		//if (!timeout)
		//	break;
		mdelay(1000);
		printf("[%s,%d]wait for timer interrupt, %d seconds elapsed\n",
				__func__, __LINE__, timeout);

	} while (!time_user_defined_flag);

	csi_timer_uinit();

	return 0;
}

U_BOOT_CMD(timer_alarm, 2, 0, timer_alarm_set, "timer_alarm 10", "timer interrupt test");
