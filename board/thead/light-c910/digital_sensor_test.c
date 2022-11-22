/*
 * Copyright (C) 2017-2022 Alibaba Group Holding Limited
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
#include <thead/clock_config.h>

#define TEE_LIGHT_APCLK_ADDRBASE	((void __iomem *)0xffff011000)
#define REG_TEESYS_CLK_TEECFG		((void __iomem *)TEE_LIGHT_APCLK_ADDRBASE + 0x1cc)

/* VIDEO PLL */
#define	TEESYS_I1_HCLK_DIV_EN		BIT(12)
#define TEESYS_I1_HCLK_DIV_NUM_SHIFT	8
#define TEESYS_I1_HCLK_DIV_NUM_MASK	0xf

#define LIGHT_CPUFREQ_THRE              1500000
#define LIGHT_C910_BUS_CLK_SYNC         BIT(11)
#define LIGHT_C910_BUS_CLK_RATIO_MASK   0x700
#define LIGHT_C910_BUS_CLK_DIV_RATIO_2  0x100
#define LIGHT_C910_BUS_CLK_DIV_RATIO_3  0x200

extern int ds_init(void);

bool global_ds_init = false;

static int ds_cpu_alarm_clk_set(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	unsigned long new_freq;
	int ret = 0;
	u32 val;
	const struct clk_info *parent;

	if (argc != 2) {
		printf("invalid input parameters\n");
		return -EINVAL;
	}

	if (strict_strtoul(argv[1], 10, &new_freq) < 0)
		return CMD_RET_USAGE;

	val = readl(TEE_LIGHT_APCLK_ADDRBASE + 0x100);
	val &= ~LIGHT_C910_BUS_CLK_RATIO_MASK;
	val |= LIGHT_C910_BUS_CLK_DIV_RATIO_3;

	writel(val, TEE_LIGHT_APCLK_ADDRBASE + 0x100);
	val &= ~LIGHT_C910_BUS_CLK_SYNC;
	writel(val, TEE_LIGHT_APCLK_ADDRBASE + 0x100);
	udelay(1);
	val |= LIGHT_C910_BUS_CLK_SYNC;
	writel(val, TEE_LIGHT_APCLK_ADDRBASE + 0x100);
	udelay(1);

	printf("wait for cpu frequency alarm, rate: %ld\n", new_freq);

	parent = clk_light_get_parent("c910_cclk");
	if (!strcmp(parent->clk_name,  "cpu_pll1_foutpostdiv")) {
		ret = clk_light_set_rate("c910_cclk_i0", CLK_DEV_MUX, new_freq);
		if (ret) {
			printf("failed to set cpu frequency\n");
			ret = -EINVAL;
			goto out;
		}
		udelay(3);
		ret = clk_light_set_parent("c910_cclk", "c910_cclk_i0");
		if (ret) {
			printf("failed to set parent clock for cpu\n");
			ret = -EINVAL;
			goto out;
		}
	} else {
		ret = clk_light_set_rate("cpu_pll1_foutpostdiv", CLK_DEV_PLL, new_freq);
		if (ret) {
			printf("failed to set cpu frequency\n");
			ret = -EINVAL;
			goto out;
		}
		udelay(3);
		ret = clk_light_set_parent("c910_cclk", "cpu_pll1_foutpostdiv");
		if (ret) {
			printf("failed to set parent clock for cpu\n");
			ret = -EINVAL;
			goto out;
		}
	}

	printf("C910 CPU FREQ: %ldMHz\n", new_freq / 1000000);

out:
	return ret;
}

static int ds_3to6_alarm_clk_set(cmd_tbl_t *cmdtp, int flag, int argc,
				 char * const argv[])
{
	unsigned long div;
	int ret = 0;
	u32 cfg;

	if (argc != 2) {
		printf("invalid input parameters\n");
		return -EINVAL;
	}

	if (strict_strtoul(argv[1], 10, &div) < 0)
		return CMD_RET_USAGE;

	if (div < 2 || div > 15) {
		printf("invalid teesys clock divider number(%ld)\n", div);
		return -EINVAL;
	}

	cfg = readl(REG_TEESYS_CLK_TEECFG);
	cfg &= ~TEESYS_I1_HCLK_DIV_EN;
	writel(cfg, REG_TEESYS_CLK_TEECFG);

	cfg &= ~(TEESYS_I1_HCLK_DIV_NUM_MASK << TEESYS_I1_HCLK_DIV_NUM_SHIFT);
	cfg |= (div & TEESYS_I1_HCLK_DIV_NUM_MASK) << TEESYS_I1_HCLK_DIV_NUM_SHIFT;
	cfg |= TEESYS_I1_HCLK_DIV_EN;
	writel(cfg, REG_TEESYS_CLK_TEECFG);

	return ret;
}

static int ds_7_alarm_clk_set(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	return ds_3to6_alarm_clk_set(cmdtp, flag, argc, argv);
}

static int ds_init_cfg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (!global_ds_init) {
		global_ds_init = true;
		return ds_init();
	}

	return 0;
}

U_BOOT_CMD(ds_init, 1, 0, ds_init_cfg, "ds_init", "Initalize the digital sensor controller");
U_BOOT_CMD(ds_cpu_alarm, 2, 0, ds_cpu_alarm_clk_set, "ds_cpu_alarm 1500000000", "digital sensor cpu0~cpu3 alarm test");
U_BOOT_CMD(ds_3to6_alarm, 2, 0, ds_3to6_alarm_clk_set, "ds_3to6_alarm 3", "digital sensor for digital3~digital6 alarm test");
U_BOOT_CMD(ds_7_alarm, 2, 0, ds_7_alarm_clk_set, "ds_7_alarm 3", "digital sensor for digital7 alarm test");

