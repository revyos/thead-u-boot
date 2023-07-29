/*
 * Copyright (C) 2017-2021 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

#define LIGHT_APCLK_ADDRBASE	0xffff011000
#define LIGHT_AONCLK_ADDRBASE	0xfffff46000
#define LIGHT_DDRCLK_ADDRBASE	0xffff005000
#define LIGHT_MISC_SUBSYS_ADDRBASE	0xfffc02d000
#define LIGHT_VI_SUBSYS_ADDRBASE	0xfff4041000
#define LIGHT_VO_SUBSYS_ADDRBASE	0xffff401000
#define LIGHT_VO_SUBSYS_R_ADDRBASE	0xffef528000
#define LIGHT_VP_SUBSYS_ADDRBASE	0xfffcc01000
#define LIGHT_DSP_SUBSYS_ADDRBASE	0xffff041000
#define LIGHT_AUDIO_SUBSYS_ADDRBASE	0xffcb000000
#define LIGHT_APSYS_RSTGEN_ADDRBASE	0xffff015000
#define LIGHT_DPU_CLOCK_GATING_CTRL0	0xffef601A28
#define LIGHT_DPU_CLOCK_GATING_CTRL1    0xffef601A2C

void show_sys_clk(void)
{
	/* Do nothing for FPGA */
}

void cpu_clk_config(uint32_t cpu_freq)
{
#ifndef CONFIG_TARGET_LIGHT_FPGA_FM_C910 /* for sillicon */
	unsigned int tmp;
	/* 4. update c910_cclk to 750Mhz */
	/* set cpu_pll1_foutpostdiv to 750Mhz */
	writel(0x20000000, (void *)LIGHT_APCLK_ADDRBASE + 0x14);
	writel(0x01407d01, (void *)LIGHT_APCLK_ADDRBASE + 0x10);
	writel(0x23000000, (void *)LIGHT_APCLK_ADDRBASE + 0x14);
	udelay(3);
	writel(0x03000000, (void *)LIGHT_APCLK_ADDRBASE + 0x14);
	readl((void *)LIGHT_APCLK_ADDRBASE + 0x80);
	readl((void *)LIGHT_APCLK_ADDRBASE + 0x80);
	while(!(readl((void *)LIGHT_APCLK_ADDRBASE + 0x80) & 0x10));
	udelay(11);

	/* config bus: cpu clk ratio to 1:1 */
	writel((readl(LIGHT_APCLK_ADDRBASE + 0x100) & (~(0x7<<8))) | (0x0<<8), (void *)(LIGHT_APCLK_ADDRBASE + 0x100)); // ratio=0
	writel(readl(LIGHT_APCLK_ADDRBASE + 0x100) & (~(0x1<<11)), (void *)(LIGHT_APCLK_ADDRBASE + 0x100)); // sync=0
	writel(readl(LIGHT_APCLK_ADDRBASE + 0x100) | (0x1<<11), (void *)(LIGHT_APCLK_ADDRBASE + 0x100)); // sync=1

	/* switch c910_cclk to cpu_pll1_foutpostdiv */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x100);
	tmp |= 0x1;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x100);
	udelay(1);
#endif
}

void sys_clk_config(void)
{
#ifndef CONFIG_TARGET_LIGHT_FPGA_FM_C910 /* for sillicon */
	unsigned int tmp;

	/* 1. double check all pll lock */
	udelay(60);
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x80);
	if (!((tmp & 0x3fe) == 0x3fe))
		return;

	/* 2. update sys_pll to frac mode, 2438.5536MHz */
	/* switch share_sram_clk to audio_pll_foutvco */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x104);
	tmp |= 0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x104);

	/* switch audio_c906_cclk to audio_pll_foutvco */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x110);
	tmp |= 0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x110);

	/* switch audio_subsys_aclk to audio_pll_foutvco */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x108);
	tmp |= 0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x108);

	/* disable audio_i2s_src_clk */
	tmp = readl((void *)LIGHT_AUDIO_SUBSYS_ADDRBASE + 0x4);
	tmp &= ~0x20200;
	writel(tmp, (void *)LIGHT_AUDIO_SUBSYS_ADDRBASE + 0x4);

	/* disable peri_i2s_src_clk */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1f0);
	tmp &= ~0x2;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1f0);

	udelay(1);

	/* set sys_pll_foutvco to 2438.5536MHz */
	writel(0x20000000, (void *)LIGHT_AONCLK_ADDRBASE + 0x14);
	writel(0x03606501, (void *)LIGHT_AONCLK_ADDRBASE + 0x10);
	writel(0x209b3d08, (void *)LIGHT_AONCLK_ADDRBASE + 0x14);
	udelay(3);
	writel(0x009b3d08, (void *)LIGHT_AONCLK_ADDRBASE + 0x14);
	readl((void *)LIGHT_AONCLK_ADDRBASE + 0x90);
	readl((void *)LIGHT_AONCLK_ADDRBASE + 0x90);
	while(!(readl((void *)LIGHT_AONCLK_ADDRBASE + 0x90) & 0x2));
	udelay(11);

	/* switch share_sram_clk to sys_pll_foutvco */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x104);
	tmp &= ~0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x104);

	/* switch audio_c906_cclk to sys_pll_foutvco */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x110);
	tmp &= ~0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x110);

	/* swith audio_subsys_aclk to sys_pll_foutvco */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x108);
	tmp &= ~0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x108);

	/* 3. update audio_pll, to frac mode, 884.736MHz */
	/* switch aonsys_clk to pad_osc_clk */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x100);
	tmp |= 0x10;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x100);

	/* set audio_pll_foutvco to frac mode, 884.736MHz */
	writel(0x20000000, (void *)LIGHT_AONCLK_ADDRBASE + 0x04);
	writel(0x01302401, (void *)LIGHT_AONCLK_ADDRBASE + 0x00);
	writel(0x20dd2f70, (void *)LIGHT_AONCLK_ADDRBASE + 0x04);
	udelay(3);
	writel(0x00dd2f70, (void *)LIGHT_AONCLK_ADDRBASE + 0x04);
	readl((void *)LIGHT_AONCLK_ADDRBASE + 0x90);
	readl((void *)LIGHT_AONCLK_ADDRBASE + 0x90);
	while(!(readl((void *)LIGHT_AONCLK_ADDRBASE + 0x90) & 0x1));
	udelay(11);

	/* switch aonsys_clk to audio_pll_foutpostdiv */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x100);
	tmp &= ~0x10;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x100);

	/* switch aoi2c_ic_clk to audio_pll_fout3 */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x11c);
	tmp &= ~0x1;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x11c);

	/* enable audio_i2s_src_clk */
	tmp = readl((void *)LIGHT_AUDIO_SUBSYS_ADDRBASE + 0x4);
	tmp |= 0x20200;
	writel(tmp, (void *)LIGHT_AUDIO_SUBSYS_ADDRBASE + 0x4);

	/* enable peri_i2s_src_clk */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1f0);
	tmp |= 0x2;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1f0);

	/* set apb3_cpusys_pclk to ahb2_cpusys_hclk/2 */
	/* CPU AHB 125MHz  CPU pclk 125MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x130);
	tmp &= ~0x8;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x130);
	udelay(1);
	tmp &= ~0x7;
	tmp |= 0x1;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x130);
	udelay(1);
	tmp |= 0x8;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x130);
	udelay(1);
	/* CPU AHB 125MHz  CPU pclk 62.5MHz */

	/* set ahb2_cpusys_hclk to 250Mhz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x120);
	tmp |= 0x20;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x120);
	udelay(1);
	tmp &= ~0x10;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x120);
	udelay(1);
	tmp &= ~0x7;
	tmp |= 0x2;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x120);
	udelay(1);
	tmp |= 0x10;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x120);
	udelay(1);
	tmp &= ~0x20;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x120);
	udelay(1);
	/* CPU AHB 250MHz  CPU pclk 125MHz */

	/* perisys_apb_pclk to perisys_ahb_hclk/4 */
	/* perisys_ahb_hclk 62.5MHz  perisys_apb_pclk 62.5MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x150);
	tmp &= ~0x8;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x150);
	udelay(1);
	tmp &= ~0x7;
	tmp |= 0x3;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x150);
	udelay(1);
	tmp |= 0x8;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x150);
	udelay(1);
	/* perisys_ahb_hclk 62.5MHz  perisys_apb_pclk 15.625MHz */

	/* perisys_ahb_hclk to 250MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x140);
	tmp |= 0x20;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x140);
	udelay(1);
	/* perisys_ahb_hclk 24MHz  perisys_apb_pclk 6MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x140);
	tmp &= ~0x10;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x140);
	udelay(1);
	tmp &= ~0xf;
	tmp |= 0x2;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x140);
	/* perisys_ahb_hclk 24MHz  perisys_apb_pclk 6MHz */
	udelay(1);
	tmp |= 0x10;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x140);
	udelay(1);
	tmp &= ~0x20;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x140);
	/* perisys_ahb_hclk 250MHz  perisys_apb_pclk 62.5MHz */

	/* set dpu0_pll_div_clk to dpu0_pll_foutpostdiv/16 as 74.25MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1e8);
	tmp &= ~0x100;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1e8);
	udelay(1);
	tmp &= ~0xff;
	tmp |= 0x10;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1e8);
	udelay(1);
	tmp |= 0x100;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1e8);
	udelay(1);

	/* set dpu1_pll_div_clk to dpu1_pll_foutpostdiv/16 as 74.25MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1ec);
	tmp &= ~0x100;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1ec);
	udelay(1);
	tmp &= ~0xff;
	tmp |= 0x10;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1ec);
	udelay(1);
	tmp |= 0x100;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1ec);
	udelay(1);

	/*5. enable necessary gates */
	/* enable dsp_subsys, vi_subsys, vo_subsys all clocls */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x220);
	tmp |= 0x7;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x220);

	/* AP rst_gen: VP/VO/VI/DSP */
	writel(0xf, (void *)LIGHT_APSYS_RSTGEN_ADDRBASE + 0x220);

	/* enable dsp0/1_cclk, dsp0/1_pclk */
	tmp = readl((void *)LIGHT_DSP_SUBSYS_ADDRBASE + 0x24);
	tmp |= 0xf;
	writel(tmp, (void *)LIGHT_DSP_SUBSYS_ADDRBASE + 0x24);

	/* enable gpu_core_clk, gpu_cfg_aclk */
	tmp = readl((void *)LIGHT_VO_SUBSYS_ADDRBASE + 0x50);
	tmp |= 0x18;
	writel(tmp, (void *)LIGHT_VO_SUBSYS_ADDRBASE + 0x50);

	tmp = readl((void *)LIGHT_VO_SUBSYS_R_ADDRBASE + 0x50);
	tmp |= 0x3ff;
	writel(tmp, (void *)LIGHT_VO_SUBSYS_R_ADDRBASE + 0x50);

	/* enable dpu_pixelclk0/1, dpu_hclk, dpu_aclk, dpu_cclk */
	tmp = readl((void *)LIGHT_VO_SUBSYS_ADDRBASE + 0x50);
	tmp |= 0x3e0;
	writel(tmp, (void *)LIGHT_VO_SUBSYS_ADDRBASE + 0x50);

	/* enable npu_axi_aclk, npu_core_clk */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1c8);
	tmp |= 0x30;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1c8);

	/* The boards other than the LightA board perform the bus down-speed operation */

#if defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_DISCRETE) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_POWER) || defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_B)
	/* axi_sram_clk: 812.8512MHz -> 688.128MHz */
	tmp = readl((void *)LIGHT_AONCLK_ADDRBASE + 0x104);
	tmp |= 0x2000;
	writel(tmp, (void *)LIGHT_AONCLK_ADDRBASE + 0x104);

	/* visys_aclk_m decrease frequency 792MHZ->594MHZ */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1d0);
	tmp &= ~0x00100000;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1d0);

	tmp &= ~0x000f0000;
	tmp |= 0x00140000;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1d0);
	/* vosys_aclk_m：792MHz->594MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1dc);
	tmp &= ~0x00000020;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1dc);

	tmp &= ~0x0000000f;
	tmp |= 0x00000024;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1dc);

	/* vpsys_axi_aclk：792MHz->594MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1e0);
	tmp &= ~0x00001000;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1e0);

	tmp &= ~0x00000f00;
	tmp |= 0x00001400;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1e0);

	/* npu_cclk：1000MHz->792MHz */
	tmp = readl((void *)LIGHT_APCLK_ADDRBASE + 0x1c8);
	tmp |= 0x00000040;
	writel(tmp, (void *)LIGHT_APCLK_ADDRBASE + 0x1c8);


	/* Enable dpu auto clock gating */
	writel(0, (void __iomem *)LIGHT_DPU_CLOCK_GATING_CTRL0);
	writel(0, (void __iomem *)LIGHT_DPU_CLOCK_GATING_CTRL1);
#endif

#endif
}

void ddr_clk_config(int ddr_freq)
{
	/* Do nothing for FPGA */
}
