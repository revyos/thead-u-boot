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

#define LIGHT_TEE_SYSREG_BASE	((void __iomem*)0xffff200000)
#define REG_SECURITY_ERR_3	(LIGHT_TEE_SYSREG_BASE + 0x30)

/* id: 0~8 */
#define DS_C910_BASE(id)	(0xffffc57000 + (id) * 0x40)
#define DS_APB_BASE(id)		(0xffff270000 + ((id) - 4) * 0x1000)

#define DS_BASE(id)		(((id) > 3) ? (DS_APB_BASE(id)) : (DS_C910_BASE(id)))

#define REG_HW_VERSION(id)			((void __iomem *)DS_BASE(id) + 0x00)	//RO
#define REG_SENSOR_ALARM(id)			((void __iomem *)DS_BASE(id) + 0x04)	//RO
#define REG_HEALTH_TEST_ALARM(id)		((void __iomem *)DS_BASE(id) + 0x08)	//RO
#define REG_HEALTH_TEST_STATUS(id)		((void __iomem *)DS_BASE(id) + 0x0C)	//RO
#define REG_CONTROL(id)				((void __iomem *)DS_BASE(id) + 0x10)	//WO
#define REG_IRQ_STATUS(id)			((void __iomem *)DS_BASE(id) + 0x14)	//RO
#define REG_IRQ_CLEAR(id)			((void __iomem *)DS_BASE(id) + 0x18)	//WO
#define REG_IRQ_CONFIG(id)			((void __iomem *)DS_BASE(id) + 0x1C)	//RW
#define REG_STATUS_DSX(id, x)			((void __iomem *)DS_BASE(id) + 0x20 + 0x04 * x)	//RO

#define LIGHT_SEC_IRQ_NUM			91

#define MAX_DIGITAL_SENSOR_NUM			9
#define MAX_HW_MACRO_NUM_PER_DS			16

#define DS_HW_MACRO_TL_MASK			0xffff

#define DS_INVALID_MACRO_ID			MAX_HW_MACRO_NUM_PER_DS

#define DS_1_ERR				BIT(16)	//digital_sensor_C910_0 ~ digital_sensor_C910_3
#define DS_3_ERR				BIT(18) //digital_sensor_apb_3
#define DS_4_ERR				BIT(19) //digital_sensor_apb_4
#define DS_5_ERR				BIT(20) //digital_sensor_apb_5
#define DS_6_ERR				BIT(21) //digital_sensor_apb_6
#define DS_7_ERR				BIT(22) //digital_sensor_apb_7
#define DS_ERR					(DS_1_ERR | DS_3_ERR | DS_4_ERR | DS_5_ERR | DS_6_ERR | DS_7_ERR)

#define CFG_DS_CPU_THREAT_LEVEL			0
#define CFG_APB0_THREAT_LEVEL			0
#define CFG_APB1_THREAT_LEVEL			0

#define AONSYS_RSTGEN_BASE			((void __iomem *)0xFFFFF44000UL)
#define REG_SYS_RST_CFG				(AONSYS_RSTGEN_BASE + 0x10)
#define SW_SYS_RST_REQ				(1 << 0)
#define REG_RST_REQ_EN_0			(AONSYS_RSTGEN_BASE + 0x140)
#define SW_GLB_RST_EN				(1 << 0)

struct ds_data {
	int id;
	u32 hw_macro_num;
};

struct ds_data ds_array[MAX_DIGITAL_SENSOR_NUM] = {
	{0, 4},		//digital_sensor_C910_0
	{1, 4},		//digital_sensor_C910_1
	{2, 4},		//digital_sensor_C910_2
	{3, 4},		//digital_sensor_C910_3
	{4, 16},	//digital_sensor_apb_3
	{5, 11},	//digital_sensor_apb_4
	{6, 4},		//digital_sensor_apb_5
	{7, 16},	//digital_sensor_apb_6
	{8, 8},		//digital_sensor_apb_7
};

static void system_reset(void)
{
	u32 data = readl(REG_RST_REQ_EN_0);

	/* global reset enable */
	data |= SW_GLB_RST_EN;
	writel(data, REG_RST_REQ_EN_0);

	/* global reset request */
	writel(SW_SYS_RST_REQ, REG_SYS_RST_CFG);
	mdelay(1000);
}

static u32 ds_hw_macro_threat_level_get(int ds_id, u32 macro_id)
{
	return readl(REG_STATUS_DSX(ds_id, macro_id)) & DS_HW_MACRO_TL_MASK;
}

static __maybe_unused int ds_health_test_done_status(int ds_id, u32 macro_id)
{
	return readl(REG_HEALTH_TEST_STATUS(ds_id)) & (1 << macro_id) ? 1 : 0;
}

static __maybe_unused int ds_health_test_alarm_status(int ds_id, u32 macro_id)
{
	return readl(REG_HEALTH_TEST_ALARM(ds_id)) & (1 << macro_id) ? 1 : 0;
}

static __maybe_unused bool ds_sensor_alarm_event_hw_macro_get(int ds_id, u32 *event_macro)
{
	u32 alarm_status = readl(REG_SENSOR_ALARM(ds_id));
	int num = ds_array[ds_id].hw_macro_num;
	int s_alarm_bit = 0, j = 0;
	bool sensor_alarm_event_occured = false;

	for (int i = 0; i < num; i++) {
		s_alarm_bit = 1 << i;
		if (alarm_status & s_alarm_bit) {
			event_macro[j] = i;
			j++;
			sensor_alarm_event_occured = true;
		}
	}

	return sensor_alarm_event_occured;
}

static __maybe_unused bool ds_health_alarm_event_hw_macro_get(int ds_id, u32 *event_macro)
{
	u32 h_alarm_status = readl(REG_HEALTH_TEST_ALARM(ds_id));
	int num = ds_array[ds_id].hw_macro_num;
	int h_alarm_bit = 0, j = 0;
	bool health_alarm_event_occured = false;

	for (int i = 0; i < num; i++) {
		h_alarm_bit = 1 << i;
		if (h_alarm_status & h_alarm_bit) {
			event_macro[j] = i;
			j++;
			health_alarm_event_occured = true;
		}
	}

	return health_alarm_event_occured;
}

static bool ds_sensor_irq_hw_macro_get(int ds_id, u32 *irq_macro)
{
	u32 irq_status = readl(REG_IRQ_STATUS(ds_id));
	int num = ds_array[ds_id].hw_macro_num;
	int s_alarm_bit = 0, j = 0;
	bool sensor_alarm_irq_occured = false;

	for (int i = 0; i < num; i++) {
		s_alarm_bit = 1 << ((i << 1) + 1);
		if (irq_status & s_alarm_bit) {
			irq_macro[j] = i;
			debug("[%s,%d]irq_macro[%d] = %d\n", __func__, __LINE__,
							j, irq_macro[j]);
			j++;
			sensor_alarm_irq_occured = true;
		}
	}

	debug("[%s,%d] sensor_alarm_irq_occured = %d\n", __func__, __LINE__,
							sensor_alarm_irq_occured);
	return sensor_alarm_irq_occured;
}

static __maybe_unused bool ds_health_irq_hw_macro_get(int ds_id, u32 *irq_macro)
{
	u32 irq_status = readl(REG_IRQ_STATUS(ds_id));
	int num = ds_array[ds_id].hw_macro_num;
	int h_alarm_bit = 0, j = 0;
	bool health_alarm_irq_occured = false;

	for (int i = 0; i < num; i++) {
		h_alarm_bit = 1 << (i << 1);
		if (irq_status & h_alarm_bit) {
			irq_macro[j] = i;
			j++;
			health_alarm_irq_occured = true;
		}
	}

	return health_alarm_irq_occured;
}

static void ds_sensor_irq_clear(int ds_id, u32 macro_id)
{
	int s_alarm_bit = 1 << ((macro_id << 1) + 1);

	debug("[%s,%d]reg: 0x%lx, s_alarm_bit = 0x%x\n", __func__, __LINE__,
							(unsigned long)REG_IRQ_CLEAR(ds_id), s_alarm_bit);

	writel(s_alarm_bit, REG_IRQ_CLEAR(ds_id));
}

static __maybe_unused void ds_health_irq_clear(int ds_id, u32 macro_id)
{
	int h_alarm_bit = 1 << (macro_id << 1);

	writel(h_alarm_bit, REG_IRQ_CLEAR(ds_id));
}

static void ds_hw_macro_sensor_alarm_endisable(int ds_id, u32 macro_id, bool en)
{
	int s_alarm_bit = 1 << ((macro_id << 1) + 1);
	u32 bitmap = readl(REG_IRQ_CONFIG(ds_id));

	if (en)
		bitmap |= s_alarm_bit;
	else
		bitmap &= ~s_alarm_bit;

	writel(bitmap, REG_IRQ_CONFIG(ds_id));
}

static __maybe_unused void ds_hw_macro_health_alarm_endisable(int ds_id, u32 macro_id, bool en)
{
	int h_alarm_bit = 1 <<  (macro_id << 1);
	u32 bitmap = readl(REG_IRQ_CONFIG(ds_id));

	if (en)
		bitmap |= h_alarm_bit;
	else
		bitmap &= ~h_alarm_bit;

	writel(bitmap, REG_IRQ_CONFIG(ds_id));
}

static void ds_hw_macro_rearm(int ds_id, u32 macro_id, int macro_size)
{
	int rearm_bit = 1 << (macro_id + macro_size);

	debug("[%s,%d]rearm_bit = 0x%x\n", __func__, __LINE__, rearm_bit);
	writel(rearm_bit, REG_CONTROL(ds_id));
}

static __maybe_unused void ds_hw_macro_health_test_start(int ds_id, u32 macro_id)
{
	int start_bit = 1 << macro_id;

	writel(start_bit, REG_CONTROL(ds_id));
}

static void light_sec_irq_handler(void)
{
	u32 id, i, j = 0, t = 0, size;
	u32 tl;
	u32 irq_macro[MAX_HW_MACRO_NUM_PER_DS] = {0};
	u32 apb_ds[8] = {0};
	u32 status = readl(REG_SECURITY_ERR_3);

	if (!status)
		return;

	if (!(status & DS_ERR)) {
		printf("Unexpected security interrupt but not ditital sensor alarm occured\n");
		return; //drop other security interrupts, how to clear ?
	}

	if (status & DS_1_ERR) {
		printf("C910 cpu digital sensor detect error event\n");

		for (id = 0; id < 4; id++) {
			for (i = 0; i < ARRAY_SIZE(irq_macro); i++)
				irq_macro[i] = DS_INVALID_MACRO_ID;

			if (!ds_sensor_irq_hw_macro_get(id, irq_macro))
				continue;

			size = ds_array[id].hw_macro_num;

			i = 0;
			while (i < MAX_HW_MACRO_NUM_PER_DS && irq_macro[i] != DS_INVALID_MACRO_ID) {

				tl = ds_hw_macro_threat_level_get(id, irq_macro[i]);

				printf("DS%d-MACRO%d Threat Level: 0x%x\n", id, irq_macro[i], tl);

				if (tl >= CFG_DS_CPU_THREAT_LEVEL) {
#if 0
					run_command("ds_cpu_alarm 1500000000", 0);
#else
					system_reset();
#endif
				} else {
#if 0
					run_command("ds_cpu_alarm 1500000000", 0);
#else
					system_reset();
#endif
				}

				//Fixmed: before irq clear !!!
				ds_hw_macro_rearm(id, irq_macro[i], size);

				ds_sensor_irq_clear(id, irq_macro[i]);

				i++;
			}
		}
	}

	if (status & DS_3_ERR) {
		printf("apb digital sensor3 detect error event\n");
		apb_ds[j++] = 4;
	}

	if (status & DS_4_ERR) {
		printf("apb digital sensor4 detect error event\n");
		apb_ds[j++] = 5;
	}

	if (status & DS_5_ERR) {
		printf("apb digital sensor5 detect error event\n");
		apb_ds[j++] = 6;
	}

	if (status & DS_6_ERR) {
		printf("apb digital sensor6 detect error event\n");
		apb_ds[j++] = 7;
	}

	if (status & DS_7_ERR) {
		printf("apb digital sensor7 detect error event\n");
		apb_ds[j++] = 8;
	}


	while (apb_ds[t]) {
		if (apb_ds[t] < 4 || apb_ds[t] > 8) {
			printf("invalid digial sensor id(%d)\n", apb_ds[t]);
			return;
		}

		for (i = 0; i < ARRAY_SIZE(irq_macro); i++)
			irq_macro[i] = DS_INVALID_MACRO_ID;

		size = ds_array[apb_ds[t]].hw_macro_num;

		if (ds_sensor_irq_hw_macro_get(apb_ds[t], irq_macro)) {

			i = 0;
			while (i < MAX_HW_MACRO_NUM_PER_DS && irq_macro[i] != DS_INVALID_MACRO_ID) {//hardware macro

				debug("[%s,%d]irq_status = 0x%x, alarm_status = 0x%x\n", __func__, __LINE__,
						readl(REG_IRQ_STATUS(apb_ds[t])), readl(REG_SENSOR_ALARM(apb_ds[t])));

				tl = ds_hw_macro_threat_level_get(apb_ds[t], irq_macro[i]);
				if (tl)
					printf("DS%d-MACRO%d Threat Level: 0x%x\n", apb_ds[t], irq_macro[i], tl);

				if (apb_ds[t] == 8 && tl >= CFG_APB1_THREAT_LEVEL) {
					//handle1
					run_command("ds_3to6_alarm 4", 0);
				} else if (tl >= CFG_APB0_THREAT_LEVEL) {
					//handle2
					run_command("ds_3to6_alarm 4", 0);
				} else {
					//handle3
					run_command("ds_3to6_alarm 4", 0);
				}

				//Fixmed: before irq clear !!!
				ds_hw_macro_rearm(apb_ds[t], irq_macro[i], size);

				ds_sensor_irq_clear(apb_ds[t], irq_macro[i]);

				i++;
			}
		}

		t++; //digital number
	}
}

static int ds_platform_init(void)
{
	int ds_id, macro_id, ret = 0;

	for (ds_id = 0; ds_id < MAX_DIGITAL_SENSOR_NUM; ds_id++) {

		printf("Digital Sensor%d HW Version: 0x%x\n", ds_id, readl(REG_HW_VERSION(ds_id)));

		for (macro_id = 0; macro_id < ds_array[ds_id].hw_macro_num; macro_id++) {
			ds_hw_macro_sensor_alarm_endisable(ds_id,
					                   macro_id, true);
			ds_hw_macro_health_test_start(ds_id, macro_id);
			udelay(5);
			ret = ds_health_test_done_status(ds_id, macro_id);
			if (!ret) {
				printf("health test failed for DS%d-Macro%d\n", ds_id, macro_id);
				ds_hw_macro_sensor_alarm_endisable(ds_id,
								   macro_id, false);
			} else {
				ret = ds_health_test_alarm_status(ds_id, macro_id);
				if (ret) {
					ds_hw_macro_sensor_alarm_endisable(ds_id,
								   macro_id, false);
					printf("health test failed for DS%d-Macro%d\n", ds_id, macro_id);
				}
			}
		}
	}

	return 0;
}

static int ds_sec_interrupt_init(void)
{
	int ret;

	ret = irq_handler_register(LIGHT_SEC_IRQ_NUM, light_sec_irq_handler);
	if (ret) {
		printf("failed to register security interrupt handler\n");
		return ret;
	}

	irq_priority_set(LIGHT_SEC_IRQ_NUM);
	irq_enable(LIGHT_SEC_IRQ_NUM);

	return 0;
}

void ds_uninit(void)
{
	int ds_id, macro_id;

	irq_disable(LIGHT_SEC_IRQ_NUM);

	for (ds_id = 0; ds_id < MAX_DIGITAL_SENSOR_NUM; ds_id++) {
		for (macro_id = 0; macro_id < ds_array[ds_id].hw_macro_num; macro_id++)
			ds_hw_macro_sensor_alarm_endisable(ds_id, macro_id, false);
	}

	irq_handler_register(LIGHT_SEC_IRQ_NUM, NULL);
}

int ds_init(void)
{
	int ret;

	ds_platform_init();

	ret = ds_sec_interrupt_init();
	if (ret)
		return ret;

	return 0;
}
