/*
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _CLOCK_CONFIG_H
#define _CLOCK_CONFIG_H

enum clk_device_type {
	CLK_DEV_ROOT,
	CLK_DEV_FIXED,
	CLK_DEV_MUX,
	CLK_DEV_DIV,
	CLK_DEV_PLL,
	CLK_DEV_FACTOR,
	CLK_DEV_GATE,
};

enum light_pll_clktype {
	LIGHT_AUDIO_PLL,
	LIGHT_SYS_PLL,
	LIGHT_CPU_PLL0,
	LIGHT_CPU_PLL1,
	LIGHT_GMAC_PLL,
	LIGHT_VIDEO_PLL,
	LIGHT_DDR_PLL,
	LIGHT_DPU0_PLL,
	LIGHT_DPU1_PLL,
};

struct clk_info {
	const char *clk_name;
	enum clk_device_type clk_dev_type;
};

int clk_config(void);
int clk_light_set_rate(const char *clk_name, enum clk_device_type clk_dev_type, unsigned long rate);
unsigned long clk_light_get_rate(const char *clk_name, enum clk_device_type clk_dev_type);
int clk_light_set_parent(const char *clk_name, const char *parent);
const struct clk_info *clk_light_get_parent(const char *clk_name);

void ap_dpu_clk_endisable(bool en);
void ap_hdmi_clk_endisable(bool en);
void ap_mipi_dsi0_clk_endisable(bool en);
void ap_mipi_dsi1_clk_endisable(bool en);
void ap_peri_clk_disable(void);

#endif
