/*
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/types.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <div64.h>
#include <thead/clock_config.h>

#define ap_base		((void __iomem *)0xffff011000)
#define aon_base         ((void __iomem *)0xfffff46000)

#define AP_REE_CLKGEN_BASE	(0xffef010000)
#define AP_DPU0_PLL_CFG1	(AP_REE_CLKGEN_BASE + 0x44)
#define AP_DPU1_PLL_CFG1	(AP_REE_CLKGEN_BASE + 0x54)
#define AP_PERI_CLK_CFG		(AP_REE_CLKGEN_BASE + 0x204)
#define AP_CTRL_CLK_CFG		(AP_REE_CLKGEN_BASE + 0x208)

#define MISCSYS_TEE_REG_BASE		(0xfffc02d000)
#define MISCSYS_TEE_CLK_CTRL_TEE	(MISCSYS_TEE_REG_BASE + 0x120)

/* VOSYS_SYREG_R */
#define VOSYS_SYSREG_BASE	(0xffef528000)
#define VOSYS_CLK_GATE_REG	(VOSYS_SYSREG_BASE + 0x50)
#define VOSYS_CLK_GATE1_REG	(VOSYS_SYSREG_BASE + 0x54)
#define VOSYS_DPU_CCLK_CFG	(VOSYS_SYSREG_BASE + 0x64)

/* VISYS_SYSREG_R */
#define VISYS_SYSREG_BASE		(0xffe4040000)
#define VISYS_MIPI_CSI0_PIXELCLK	(VISYS_SYSREG_BASE + 0x30)
#define VISYS_ISP0_CLK_CFG		(VISYS_SYSREG_BASE + 0x24)
#define VISYS_ISP1_CLK_CFG              (VISYS_SYSREG_BASE + 0x28)
#define VISYS_ISP_RY_CLK_CFG            (VISYS_SYSREG_BASE + 0x2c)

/* APSYS_SYSREG_R */
#define APSYS_CLKGEN_BASE		(0xffef010000)
#define APSYS_DPU0_PLL_DIV_CFG		(APSYS_CLKGEN_BASE + 0x1e8)
#define APSYS_DPU1_PLL_DIV_CFG          (APSYS_CLKGEN_BASE + 0x1ec)

/* AP_DPU0_PLL_CFG1 */
#define AP_DPU0_PLL_RST		BIT(29)

/* AP_DPU1_PLL_CFG1 */
#define AP_DPU1_PLL_RST		BIT(29)

/* AP_PERI_CLK_CFG */
#define GMAC1_CLK_EN		BIT(26)
#define PADCTRL1_APSYS_PCLK_EN	BIT(24)
#define DSMART_CLK_EN		BIT(23)
#define PADCTRL0_APSYS_PCLK_EN  BIT(22)
#define GMAC_AXI_CLK_EN		BIT(21)
#define GPIO3_CLK_EN		BIT(20)
#define GMAC0_CLK_EN            BIT(19)
#define PWM_CLK_EN		BIT(18)
#define QSPI0_CLK_EN		BIT(17)
#define QSPI1_CLK_EN		BIT(16)
#define SPI_CLK_EN		BIT(15)
#define UART0_CLK_EN		BIT(14)
#define UART1_CLK_EN            BIT(13)
#define UART2_CLK_EN            BIT(12)
#define UART3_CLK_EN            BIT(11)
#define UART4_CLK_EN            BIT(10)
#define UART5_CLK_EN            BIT(9)
#define GPIO0_CLK_EN            BIT(8)
#define GPIO1_CLK_EN            BIT(7)
#define GPIO2_CLK_EN            BIT(6)
#define I2C0_CLK_EN		BIT(5)
#define I2C1_CLK_EN             BIT(4)
#define I2C2_CLK_EN             BIT(3)
#define I2C3_CLK_EN             BIT(2)
#define I2C4_CLK_EN             BIT(1)
#define I2C5_CLK_EN             BIT(0)

/* AP_CTRL_CLK_CFG */
#define SPINLOCK_CLK_EN		BIT(10)
#define CHIP_DBG_CLK_EN		BIT(9)
#define DMAC_CPUSYS_CLK_EN	BIT(8)
#define MBOX0_PCLK_EN		BIT(7)
#define MBOX1_PCLK_EN           BIT(6)
#define MBOX2_PCLK_EN           BIT(5)
#define MBOX3_PCLK_EN           BIT(4)
#define WDT0_CLK_EN		BIT(3)
#define WDT1_CLK_EN		BIT(2)
#define TIMER0_CLK_EN		BIT(1)
#define TIMER1_CLK_EN		BIT(0)

/* MISCSYS_TEE_CLK_CTRL_TEE */
#define TEE_DMAC_CLK_EN		BIT(6)

/* VOSYS_CLK_GATE_REG */
#define CLKCTRL_DPU_PIXELCLK0_EN	BIT(5)
#define CLKCTRL_DPU_PIXELCLK1_EN        BIT(6)
#define CLKCTRL_DPU_HCLK_EN		BIT(7)
#define CLKCTRL_DPU_ACLK_EN             BIT(8)
#define CLKCTRL_DPU_CCLK_EN             BIT(9)

#define CLKCTRL_HDMI_SFR_CLK_EN		BIT(10)
#define CLKCTRL_HDMI_PCLK_EN		BIT(11)
#define CLKCTRL_HDMI_CEC_CLK_EN		BIT(12)
#define CLKCTRL_HDMI_I2S_CLK_EN		BIT(19)

#define CLKCTRL_MIPI_DSI0_PCLK_EN	BIT(13)
#define CLKCTRL_MIPI_DSI0_CFG_CLK_EN	BIT(15)
#define CLKCTRL_MIPI_DSI0_REFCLK_EN     BIT(17)
#define CLKCTRL_MIPIDSI0_PIXCLK_EN      BIT(30)

#define CLKCTRL_MIPI_DSI1_PCLK_EN	BIT(14)
#define CLKCTRL_MIPI_DSI1_CFG_CLK_EN	BIT(16)
#define CLKCTRL_MIPI_DSI1_REFCLK_EN     BIT(18)
#define CLKCTRL_MIPIDSI1_PIXCLK_EN      BIT(31)

/* VOSYS_CLK_GATE1_REG */
#define CLKCTRL_HDMI_PIXCLK_EN		BIT(0)

/* VOSYS_DPU_CCLK_CFG */
#define VOSYS_DPU_CCLK_DIV_NUM_MASK	0xf
#define VOSYS_DPU_CCLK_DIV_NUM_SHIFT	0
#define VOSYS_DPU_CCLK_DIV_EN		BIT(4)

/* VISYS_MIPI_CSI0_PIXELCLK  */
#define VISYS_MIPI_CSI0_PIXELCLK_DIV_NUM_SHIFT	0
#define VISYS_MIPI_CSI0_PIXELCLK_DIV_NUM_MASK	0xf
#define VISYS_MIPI_CSI0_PIXELCLK_DIV_EN		BIT(4)

/* VISYS_ISP0_CLK_CFG */
#define VISYS_ISP0_CLK_DIV_EN			BIT(4)
#define VISYS_ISP0_CLK_DIV_NUM_SHIFT		(0)
#define VISYS_ISP0_CLK_DIV_NUM_MASK		0xf

/* VISYS_ISP1_CLK_CFG */
#define VISYS_ISP1_CLK_DIV_EN			BIT(4)
#define VISYS_ISP1_CLK_DIV_NUM_SHIFT		(0)
#define VISYS_ISP1_CLK_DIV_NUM_MASK		0xf

/* VISYS_ISP_RY_CLK_CFG */
#define VISYS_ISP_RY_CLK_DIV_EN			BIT(4)
#define VISYS_ISP_RY_CLK_DIV_NUM_SHIFT		(0)
#define VISYS_ISP_RY_CLK_DIV_NUM_MASK		0xf

/* APSYS_DPU0_PLL_DIV_CFG  */
#define APSYS_DPU0_PLL_DIV_CLK_DIV_EN		BIT(8)
#define APSYS_DPU0_PLL_DIV_CLK_DIV_NUM_MASK	0xff
#define APSYS_DPU0_PLL_DIV_CLK_DIV_NUM_SHIFT	0

/* APSYS_DPU1_PLL_DIV_CFG  */
#define APSYS_DPU1_PLL_DIV_CLK_DIV_EN		BIT(8)
#define APSYS_DPU1_PLL_DIV_CLK_DIV_NUM_MASK	0xff
#define APSYS_DPU1_PLL_DIV_CLK_DIV_NUM_SHIFT	0

enum multimedia_div_type {
	VI_MIPI_CSI0_DIV,
	VI_ISP0_CORE_DIV,
	VI_ISP1_CORE_DIV,
	VI_ISP_RY_CORE_DIV,
	VO_DPU_CORE_DIV,
	VO_DPU_PLL0_DIV,
	VO_DPU_PLL1_DIV,
};

#define C910_CCLK	0
#define C910_CCLK_I0	1
#define CLK_END		16

#define LIGHT_PLL_CFG1	0x4

#define LIGHT_REFDIV_MASK       GENMASK(5, 0)
#define LIGHT_REFDIV_SHIFT      0
#define LIGHT_FBDIV_MASK        GENMASK(19, 8)
#define LIGHT_FBDIV_SHIFT       8
#define LIGHT_POSTDIV1_MASK     GENMASK(22, 20)
#define LIGHT_POSTDIV1_SHIFT    20
#define LIGHT_POSTDIV2_MASK     GENMASK(26, 24)
#define LIGHT_POSTDIV2_SHIFT    24
#define LIGHT_FRAC_MASK         GENMASK(23, 0)
#define LIGHT_FRAC_SHIFT        0
#define LIGHT_FRAC_DIV          BIT(24)

#define LIGHT_DSMPD_MASK        BIT(24)
#define LIGHT_DACPD_MASK        BIT(25)

#define LIGHT_PLL_RATE(_vco, _rate, _r, _b, _f, _p, _k) \
         {                                               \
                 .vco_rate       =       (_vco),         \
                 .rate           =       (_rate),        \
                 .refdiv         =       (_r),           \
                 .fbdiv          =       (_b),           \
                 .frac           =       (_f),           \
                 .postdiv1       =       (_p),           \
                 .postdiv2       =       (_k),           \
         }

enum light_pll_mode {
	PLL_MODE_FRAC,
	PLL_MODE_INT,
};

enum light_pll_outtype {
	LIGHT_PLL_VCO,
	LIGHT_PLL_DIV,
};

struct clk_fixed {
	const char *clk_name;
	unsigned long rate;
};

struct light_pll_rate_table {
        unsigned long vco_rate;
        unsigned long rate;
        unsigned int refdiv;
        unsigned int fbdiv;
        unsigned int frac;
        unsigned int postdiv1;
        unsigned int postdiv2;
};

struct clk_lightpll {
	const char *clk_name;
	void __iomem			*base;
	enum light_pll_clktype          clk_type;
	enum light_pll_outtype          out_type;
	enum light_pll_mode             pll_mode;
	const struct light_pll_rate_table *rate_table;
	int rate_count;
	u32 cfg0_reg_off;
	u32 pll_sts_off;
	int pll_lock_bit;
	int pll_rst_bit;
	int pll_bypass_bit;
	enum clk_device_type clk_dev_type;
};

struct clk_fixed clks_fixed[] = {
	{"video_pll_foutvco", 2376000000U},
	{"gmac_pll_foutpostdiv", 1000000000U},
};

static const struct light_pll_rate_table light_cpupll_tbl[] = {
	/* vco_rate postdiv_rate refdiv fbdiv frac postdiv1 postdiv2 */
	LIGHT_PLL_RATE(3000000000U, 125000000U, 1, 125,  0, 6, 4),
	LIGHT_PLL_RATE(3000000000U, 1000000000U, 1, 125, 0, 3, 1),
	LIGHT_PLL_RATE(3000000000U, 1500000000U, 1, 125, 0, 2, 1),
	LIGHT_PLL_RATE(1800000000U, 1800000000U, 1, 75,  0, 1, 1),
	LIGHT_PLL_RATE(2256000000U, 752000000U,  1, 94,  0, 3, 1),
	LIGHT_PLL_RATE(3000000000U, 300000000U,  1, 125, 0, 5, 2),
	LIGHT_PLL_RATE(1848000000U, 1848000000U, 1, 77,  0, 1, 1),
	LIGHT_PLL_RATE(1872000000U, 1872000000U, 1, 78,  0, 1, 1),
};

static const struct light_pll_rate_table light_audio_pll_tbl[] = {
	LIGHT_PLL_RATE(864000000U, 288000000U, 1, 36, 0, 3, 1),		/* INT MODE */
	LIGHT_PLL_RATE(884736000U, 294912000U, 1, 36, 14495600U, 3, 1),	/* FRAC MODE */
	LIGHT_PLL_RATE(2064384000U, 294912000U, 1, 86, 268500U, 7, 1),  /* FRAC MODE */
};

static const struct light_pll_rate_table light_sys_pll_tbl[] = {
	LIGHT_PLL_RATE(2424000000U, 134667000U, 1, 101, 0, 6, 3),	/* INT MODE */
	LIGHT_PLL_RATE(884736000U, 294912000U, 1, 36, 14495600, 3, 1),	/* FRAC MODE */
	LIGHT_PLL_RATE(2438553600U, 135475200U, 1, 101, 10173704, 6, 3), /* FRAC MODE */
};

static const struct light_pll_rate_table light_dpu_pll_tbl[] = {
	LIGHT_PLL_RATE(1188000000U, 1188000000U, 1, 0x31, 0x800000, 1, 1),	/* FRAC MODE */
	LIGHT_PLL_RATE(1611200000U, 805600000U, 200, 0x43, 0x222222, 2, 1),	/* FRAC MODE */
};

static const struct light_pll_rate_table light_gmac_pll_tbl[] = {
	LIGHT_PLL_RATE(3000000000U, 1000000000U, 1, 125, 0, 3, 1),
};

struct clk_lightpll light_cpu_pll0div = {
	.clk_name = "cpu_pll0_foutpostdiv",
	.base = ap_base,
	.out_type = LIGHT_PLL_DIV,
	.clk_type = LIGHT_CPU_PLL0,
	.rate_table = light_cpupll_tbl,
	.rate_count = ARRAY_SIZE(light_cpupll_tbl),
	.cfg0_reg_off = 0x0,
	.pll_sts_off = 0x80,
	.pll_lock_bit = BIT(1),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
	.pll_mode = PLL_MODE_INT,
	.clk_dev_type = CLK_DEV_PLL,
};

struct clk_lightpll light_cpu_pll1div = {
	.clk_name = "cpu_pll1_foutpostdiv",
	.base = ap_base,
        .out_type = LIGHT_PLL_DIV,
        .clk_type = LIGHT_CPU_PLL1,
        .rate_table = light_cpupll_tbl,
        .rate_count = ARRAY_SIZE(light_cpupll_tbl),
	.cfg0_reg_off = 0x10,
	.pll_sts_off = 0x80,
	.pll_lock_bit = BIT(4),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
	.pll_mode = PLL_MODE_INT,
	.clk_dev_type = CLK_DEV_PLL,
};

#define AUDIO_PLL_INT_MODE
struct clk_lightpll light_audio_plldiv = {
	.clk_name = "audio_pll_foutpostdiv",
	.base = aon_base,
        .out_type = LIGHT_PLL_DIV,
        .clk_type = LIGHT_AUDIO_PLL,
        .rate_table = light_audio_pll_tbl,
        .rate_count = ARRAY_SIZE(light_audio_pll_tbl),
	.cfg0_reg_off = 0x0,
	.pll_sts_off = 0x90,
	.pll_lock_bit = BIT(0),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
#ifndef AUDIO_PLL_INT_MODE
	.pll_mode = PLL_MODE_FRAC,
#else
	.pll_mode = PLL_MODE_INT,
#endif
	.clk_dev_type = CLK_DEV_PLL,
};

struct clk_lightpll light_audio_pllvco = {
	.clk_name = "audio_pll_foutvco",
	.base = aon_base,
        .out_type = LIGHT_PLL_VCO,
        .clk_type = LIGHT_AUDIO_PLL,
        .rate_table = light_audio_pll_tbl,
        .rate_count = ARRAY_SIZE(light_audio_pll_tbl),
	.cfg0_reg_off = 0x0,
	.pll_sts_off = 0x90,
	.pll_lock_bit = BIT(0),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
#ifndef AUDIO_PLL_INT_MODE
	.pll_mode = PLL_MODE_FRAC,
#else
	.pll_mode = PLL_MODE_INT,
#endif
	.clk_dev_type = CLK_DEV_PLL,
};

#define SYS_PLL_INT_MODE
struct clk_lightpll light_sys_plldiv = {
	.clk_name = "sys_pll_foutpostdiv",
	.base = aon_base,
        .out_type = LIGHT_PLL_DIV,
        .clk_type = LIGHT_SYS_PLL,
        .rate_table = light_sys_pll_tbl,
        .rate_count = ARRAY_SIZE(light_sys_pll_tbl),
	.cfg0_reg_off = 0x10,
	.pll_sts_off = 0x90,
	.pll_lock_bit = BIT(1),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
#ifndef SYS_PLL_INT_MODE
	.pll_mode = PLL_MODE_FRAC,
#else
	.pll_mode = PLL_MODE_INT,
#endif
	.clk_dev_type = CLK_DEV_PLL,
};

struct clk_lightpll light_sys_pllvco = {
	.clk_name = "sys_pll_foutvco",
	.base = aon_base,
        .out_type = LIGHT_PLL_VCO,
        .clk_type = LIGHT_SYS_PLL,
        .rate_table = light_sys_pll_tbl,
        .rate_count = ARRAY_SIZE(light_sys_pll_tbl),
	.cfg0_reg_off = 0x10,
	.pll_sts_off = 0x90,
	.pll_lock_bit = BIT(1),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
#ifndef SYS_PLL_INT_MODE
	.pll_mode = PLL_MODE_FRAC,
#else
	.pll_mode = PLL_MODE_INT,
#endif
	.clk_dev_type = CLK_DEV_PLL,
};

#define DPU_PLL_INT_MODE
struct clk_lightpll light_dpu0_plldiv = {
	.clk_name = "dpu0_pll_foutpostdiv",
	.base = ap_base,
	.out_type = LIGHT_PLL_DIV,
	.clk_type = LIGHT_DPU0_PLL,
	.rate_table = light_dpu_pll_tbl,
	.rate_count = ARRAY_SIZE(light_dpu_pll_tbl),
	.cfg0_reg_off = 0x40,
	.pll_sts_off = 0x80,
	.pll_lock_bit = BIT(8),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
#ifndef DPU_PLL_INT_MODE
	.pll_mode = PLL_MODE_FRAC,
#else
	.pll_mode = PLL_MODE_INT,
#endif
	.clk_dev_type = CLK_DEV_PLL,
};

struct clk_lightpll light_dpu1_plldiv = {
	.clk_name = "dpu1_pll_foutpostdiv",
	.base = ap_base,
	.out_type = LIGHT_PLL_DIV,
	.clk_type = LIGHT_DPU1_PLL,
	.rate_table = light_dpu_pll_tbl,
	.rate_count = ARRAY_SIZE(light_dpu_pll_tbl),
	.cfg0_reg_off = 0x50,
	.pll_sts_off = 0x80,
	.pll_lock_bit = BIT(9),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
#ifndef DPU_PLL_INT_MODE
	.pll_mode = PLL_MODE_FRAC,
#else
	.pll_mode = PLL_MODE_INT,
#endif
	.clk_dev_type = CLK_DEV_PLL,
};

struct clk_lightpll light_gmac_plldiv = {
	.clk_name = "gmac_pll_foutpostdiv",
	.base = ap_base,
	.out_type = LIGHT_PLL_DIV,
	.clk_type = LIGHT_GMAC_PLL,
	.rate_table = light_gmac_pll_tbl,
	.rate_count = ARRAY_SIZE(light_gmac_pll_tbl),
	.cfg0_reg_off = 0x20,
	.pll_sts_off = 0x80,
	.pll_lock_bit = BIT(3),
	.pll_bypass_bit = BIT(30),
	.pll_rst_bit = BIT(29),
	.pll_mode = PLL_MODE_INT,
	.clk_dev_type = CLK_DEV_PLL,
};

struct clk_lightpll *clks_pll[] = {
	&light_cpu_pll0div,
	&light_cpu_pll1div,
	&light_audio_plldiv,
	&light_audio_pllvco,
	&light_sys_plldiv,
	&light_sys_pllvco,
	&light_dpu0_plldiv,
	&light_dpu1_plldiv,
	&light_gmac_plldiv,
};

static const struct light_pll_rate_table *light_get_pll_div_settings(
                struct clk_lightpll *pll, unsigned long rate)
{
        const struct light_pll_rate_table *rate_table = pll->rate_table;
        int i;

        for (i = 0; i < pll->rate_count; i++)
                if (rate == rate_table[i].rate)
                        return &rate_table[i];

        return NULL;
}

static const struct light_pll_rate_table *light_get_pll_vco_settings(
                struct clk_lightpll *pll, unsigned long rate)
{
        const struct light_pll_rate_table *rate_table = pll->rate_table;
        int i;

        for (i = 0; i < pll->rate_count; i++)
                if (rate == rate_table[i].vco_rate)
                        return &rate_table[i];

        return NULL;
}

static inline bool clk_light_pll_change(struct clk_lightpll *pll,
                                        const struct light_pll_rate_table *rate)
{
        u32 refdiv_old, fbdiv_old, postdiv1_old, postdiv2_old, frac_old;
        u32 cfg0, cfg1;
        bool pll_changed;

        cfg0 = readl(pll->base + pll->cfg0_reg_off);
        cfg1 = readl(pll->base + pll->cfg0_reg_off + LIGHT_PLL_CFG1);

        refdiv_old = (cfg0 & LIGHT_REFDIV_MASK) >> LIGHT_REFDIV_SHIFT;
        fbdiv_old = (cfg0 & LIGHT_FBDIV_MASK) >> LIGHT_FBDIV_SHIFT;
        postdiv1_old = (cfg0 & LIGHT_POSTDIV1_MASK) >> LIGHT_POSTDIV1_SHIFT;
        postdiv2_old = (cfg0 & LIGHT_POSTDIV2_MASK) >> LIGHT_POSTDIV2_SHIFT;
        frac_old = (cfg1 & LIGHT_FRAC_MASK) >> LIGHT_FRAC_SHIFT;

        pll_changed = rate->refdiv != refdiv_old || rate->fbdiv != fbdiv_old ||
                      rate->postdiv1 != postdiv1_old || rate->postdiv2 != postdiv2_old;
        if (pll->pll_mode == PLL_MODE_FRAC)
                pll_changed |= (rate->frac != frac_old);

        return pll_changed;
}

static int clk_light_pll_wait_lock(struct clk_lightpll *pll)
{
	u32 count = 0;
	bool lock_done = false;

	while (count++ < 1000) {
		if (readl(pll->base + pll->pll_sts_off) & pll->pll_lock_bit) {
			lock_done = true;
			break;
		}
		mdelay(1);
	}

	if (lock_done)
		return 0;

	return -EBUSY;
}

int clk_pll_set_rate(struct clk_lightpll *pll, unsigned long drate)
{
	const struct light_pll_rate_table *rate;
	void __iomem *cfg1_off;
	u32 tmp, div_val;
	int ret;

	if (pll->out_type == LIGHT_PLL_VCO) {
		rate = light_get_pll_vco_settings(pll, drate);
		if (!rate) {
			printf("%s: Invalid rate : %lu for pll clk\n", __func__, drate);
			return -EINVAL;
		}
	} else {
		rate = light_get_pll_div_settings(pll, drate);
		if (!rate) {
			printf("%s: Invalid rate : %lu for pll clk\n", __func__, drate);
			return -EINVAL;
		}
	}

	if (!clk_light_pll_change(pll, rate))
		return 0;

	/* Enable RST */
	cfg1_off = pll->base + pll->cfg0_reg_off + LIGHT_PLL_CFG1;
	tmp = readl(cfg1_off);
	tmp |= pll->pll_rst_bit;
	writel(tmp, cfg1_off);

	div_val = (rate->refdiv << LIGHT_REFDIV_SHIFT) |
                   (rate->fbdiv << LIGHT_FBDIV_SHIFT) |
                   (rate->postdiv1 << LIGHT_POSTDIV1_SHIFT) |
                   (rate->postdiv2 << LIGHT_POSTDIV2_SHIFT);

	writel(div_val, pll->base + pll->cfg0_reg_off);

	if (pll->pll_mode == PLL_MODE_FRAC) {
		tmp &= ~LIGHT_DSMPD_MASK;
		tmp |= LIGHT_DACPD_MASK;
		tmp &= ~(LIGHT_FRAC_MASK << LIGHT_FRAC_SHIFT);
		tmp |= rate->frac;
		writel(tmp, cfg1_off);
	}

	udelay(3);

	/* Disable RST */
	tmp &= ~pll->pll_rst_bit;
	writel(tmp, cfg1_off);

	/* Wait Lock, ~20us cost */
	ret = clk_light_pll_wait_lock(pll);
	if (ret)
		return ret;

	/* HW requires 30us for pll stable */
	udelay(30);

	return 0;
}

unsigned long clk_pll_round_rate(struct clk_lightpll *pll, unsigned long rate)
{
	const struct light_pll_rate_table *rate_table = pll->rate_table;
	unsigned long best = 0, now = 0;
	unsigned int i, best_i = 0;

	for (i = 0; i < pll->rate_count; i++) {
		if (pll->out_type == LIGHT_PLL_DIV)
			now = rate_table[i].rate;
		else if (pll->out_type == LIGHT_PLL_VCO)
			now = rate_table[i].vco_rate;
		else {
			printf("invalid pll mode\n");
			return 0;
		}

		if (rate == now) {
			if (pll->out_type == LIGHT_PLL_DIV)
				return rate_table[i].rate;
			else
				return rate_table[i].vco_rate;
		} else if (abs(now - rate) < abs(best - rate)) {
			best = now;
			best_i = i;
		}
	}

	/* return minimum supported value */
	if (pll->out_type == LIGHT_PLL_DIV)
		return rate_table[best_i].rate;

	return rate_table[best_i].vco_rate;
}

unsigned long clk_pll_recalc_rate(struct clk_lightpll *pll, unsigned long prate)
{
	u32 refdiv, fbdiv, postdiv1, postdiv2, frac;
	u32 pll_cfg0, pll_cfg1;
	u64 fvco = 0;

	pll_cfg0 = readl(pll->base + pll->cfg0_reg_off);
	pll_cfg1 = readl(pll->base + pll->cfg0_reg_off + LIGHT_PLL_CFG1);
	refdiv = (pll_cfg0 & LIGHT_REFDIV_MASK) >> LIGHT_REFDIV_SHIFT;
	fbdiv = (pll_cfg0 & LIGHT_FBDIV_MASK) >> LIGHT_FBDIV_SHIFT;
	postdiv1 = (pll_cfg0 & LIGHT_POSTDIV1_MASK) >> LIGHT_POSTDIV1_SHIFT;
	postdiv2 = (pll_cfg0 & LIGHT_POSTDIV2_MASK) >> LIGHT_POSTDIV2_SHIFT;
	frac = (pll_cfg1 & LIGHT_FRAC_MASK) >> LIGHT_FRAC_SHIFT;

	pr_debug("reg_pll_cfg0: 0x%p, reg_pll_cfg1: 0x%p\n", (pll->base + pll->cfg0_reg_off), (pll->base + pll->cfg0_reg_off + LIGHT_PLL_CFG1));
	pr_debug("refdiv = %d, fbdiv = %d, postdiv1 = %d, postdiv2 = %d, frac = %d, mode = %d\n",
			refdiv, fbdiv, postdiv1, postdiv2, frac, pll->pll_mode);

	/* rate calculation:
	 * INT mode: FOUTVCO = FREE * FBDIV / REFDIV
	 * FRAC mode:FOUTVCO = (FREE * FBDIV + FREE * FRAC/BIT(24)) / REFDIV
	 */
	if (pll->pll_mode == PLL_MODE_FRAC)
		fvco = (prate * frac) / LIGHT_FRAC_DIV;
	fvco += (prate * fbdiv);
	do_div(fvco, refdiv);

	if (pll->out_type == LIGHT_PLL_DIV)
		do_div(fvco, postdiv1 * postdiv2);

	pr_debug("fvco = %lluHZ\n", fvco);

	return fvco;
}

struct clk_lightmux {
	const char *clk_name;
	void __iomem *reg;
	u8 shift;
	u8 width;
	const struct clk_info * parents;
	int num_parents;
	enum clk_device_type clk_dev_type;
};

static const struct clk_info c910_cclk_sels[] = {
	{"c910_cclk_i0", CLK_DEV_MUX},
	{"cpu_pll1_foutpostdiv", CLK_DEV_PLL},
};

static const struct clk_info c910_cclk_i0_sels[] = {
	{ "cpu_pll0_foutpostdiv", CLK_DEV_PLL},
	{ "osc_24m", CLK_DEV_ROOT},
};

static const struct clk_info gmac_pll_bypass_sels[] = {
	{"gmac_pll_foutpostdiv", CLK_DEV_FIXED},
	{ "osc_24m", CLK_DEV_ROOT},
};

static const struct clk_info ahb2_cpusys_hclk_sels[] = {
	{"ahb2_cpusys_hclk_out_div", CLK_DEV_DIV},
	{"osc_24m", CLK_DEV_ROOT},
};

static const struct clk_info perisys_ahb_hclk_sels[] = {
	{"perisys_ahb_hclk_out_div", CLK_DEV_DIV},
	{"osc_24m", CLK_DEV_ROOT},
};

struct clk_lightmux clks_mux[] = {
	{"c910_cclk", ap_base + 0x100, 0, 1, c910_cclk_sels, ARRAY_SIZE(c910_cclk_sels), CLK_DEV_MUX},
	{"c910_cclk_i0", ap_base + 0x100, 1, 1, c910_cclk_i0_sels, ARRAY_SIZE(c910_cclk_i0_sels), CLK_DEV_MUX},
	{"gmac_pll_bypass", ap_base + 0x24, 30, 1, gmac_pll_bypass_sels, ARRAY_SIZE(gmac_pll_bypass_sels), CLK_DEV_MUX},
	{"ahb2_cpusys_hclk", ap_base + 0x120, 5, 1, ahb2_cpusys_hclk_sels, ARRAY_SIZE(ahb2_cpusys_hclk_sels), CLK_DEV_MUX},
	{"perisys_ahb_hclk", ap_base + 0x140, 5, 1, perisys_ahb_hclk_sels, ARRAY_SIZE(perisys_ahb_hclk_sels), CLK_DEV_MUX},
};

const struct clk_info *clk_light_get_parent(const char *clk_name)
{
	struct clk_lightmux *clk = NULL;
	int idx, i;

	for (i = 0; i < ARRAY_SIZE(clks_mux); i++) {
			if (strcmp(clk_name, clks_mux[i].clk_name))
				continue;
			clk = &clks_mux[i];
			break;
		}

	if (!clk) {
		printf("%s is not a clock mux\n", clk_name);
		return NULL;
	}

	idx = (readl(clk->reg) >> clk->shift) & clk->width;

	return &clk->parents[idx];
}

int clk_light_set_parent(const char *clk_name, const char *parent)
{
	struct clk_lightmux *clk = NULL;
	int i;
	u32 val;

	for (i = 0; i < ARRAY_SIZE(clks_mux); i++) {
			if (strcmp(clk_name, clks_mux[i].clk_name))
				continue;
			clk = &clks_mux[i];
			break;
		}

	if (!clk) {
		printf("%s is not a clock mux, can not switch parent clock\n", clk_name);
		return -EINVAL;
	}

	if (!strcmp(clk->clk_name, parent))
		return 0;

	debug("clk->num_parents = %d\n", clk->num_parents);

	for (i = 0; i < clk->num_parents; i++) {
		pr_debug("parent%d:%s\n", i, clk->parents[i].clk_name);
		if (!strcmp(parent, clk->parents[i].clk_name))
			break;
	}

	if (i >= clk->num_parents) {
		printf("No target parent clock found\n");
		return -EINVAL;
	}

	val = readl(clk->reg);
	val &= ~(clk->width << clk->shift);
	val |= (i & clk->width)<< clk->shift;
	writel(val, clk->reg);

	return 0;
}

#define NPU_CCLK_OUT_DIV	0
#define div_mask(d)     ((1 << (d->width)) - 1)

enum light_div_type {
	MUX_TYPE_DIV,
	MUX_TYPE_CDE,
};

struct clk_lightdiv {
	const char *clk_name;
	enum clk_device_type clk_dev_type;
	const char *parent;
	enum clk_device_type p_clk_dev_type;
	void __iomem    *reg;
	u8              shift;
	u8              width;
	u8 sync_en;
	enum light_div_type div_type;
	u16 min_div;
	u16 max_div;
};

struct clk_lightdiv clks_div[] = {
	{"npu_cclk_out_div", CLK_DEV_DIV, "video_pll_foutvco", CLK_DEV_FIXED, ap_base + 0x1c8, 0, 3, 3, MUX_TYPE_DIV, 3, 7},
	{"ahb2_cpusys_hclk_out_div", CLK_DEV_DIV, "gmac_pll_fout1ph0", CLK_DEV_FACTOR, ap_base + 0x120, 0, 3, 4, MUX_TYPE_DIV, 2, 7},
	{"apb3_cpusys_pclk", CLK_DEV_DIV, "ahb2_cpusys_hclk", CLK_DEV_MUX, ap_base + 0x130, 0, 3, 3, MUX_TYPE_CDE, 1, 7},
	{"perisys_ahb_hclk_out_div", CLK_DEV_DIV, "gmac_pll_fout1ph0", CLK_DEV_FACTOR, ap_base + 0x140, 0, 4, 4, MUX_TYPE_DIV, 2, 7},
	{"perisys_apb_pclk", CLK_DEV_DIV, "perisys_ahb_hclk", CLK_DEV_MUX, ap_base + 0x150, 0, 3, 3, MUX_TYPE_CDE, 3, 7},
};

int clk_lightdiv_set_rate(struct clk_lightdiv *lightdiv, unsigned long rate, unsigned long parent_rate)
{
	unsigned int divider, value;
	u32 val;

	divider = parent_rate / rate;

	/* DIV is zero based divider, but CDE is not */
	if (lightdiv->div_type == MUX_TYPE_DIV)
		value = divider;
	else
		value = divider - 1;

	/* handle the div valid range */
	if (value > lightdiv->max_div)
		value = lightdiv->max_div;
	if (value < lightdiv->min_div)
		value = lightdiv->min_div;

	val = readl(lightdiv->reg);
	val &= ~BIT(lightdiv->sync_en);
	writel(val, lightdiv->reg);

	udelay(1);

	val &= ~(div_mask(lightdiv) << lightdiv->shift);
	val |= value << lightdiv->shift;
	writel(val, lightdiv->reg);

	udelay(1);

	val |= BIT(lightdiv->sync_en);
	writel(val, lightdiv->reg);

	return 0;
}

unsigned long clk_lightdiv_recalc_rate(struct clk_lightdiv *lightdiv, unsigned long prate)
{
	unsigned int divider, value;
	u32 val;

	val = readl(lightdiv->reg);
	value = (val >> lightdiv->shift) & div_mask(lightdiv);

	if (lightdiv->div_type == MUX_TYPE_DIV)
		divider = value;
	else
		divider = value + 1;

	pr_debug("divider parent rate: %ld, divider: %d\n", prate, divider);

	return prate / divider;
}

struct clk_light_fixed_factor {
	const char *clk_name;
	enum clk_device_type clk_dev_type;
	const char *parent;
	enum clk_device_type p_clk_dev_type;
	unsigned int    mult;
	unsigned int    div;
};

struct clk_light_fixed_factor clks_factor[] = {
	{"gmac_pll_fout1ph0", CLK_DEV_FACTOR, "gmac_pll_bypass", CLK_DEV_MUX, 1, 2},
};

unsigned long clk_light_factor_recalc_rate(struct clk_light_fixed_factor *fix, unsigned long prate)
{
	unsigned long long int rate;

	rate = (unsigned long long int)prate * fix->mult;
	do_div(rate, fix->div);
	return (unsigned long)rate;
}

unsigned long clk_light_get_rate(const char *clk_name, enum clk_device_type clk_dev_type)
{
	unsigned long rate;
	int i;

	if (clk_dev_type == CLK_DEV_PLL) {
		struct clk_lightpll *pll = NULL;

		for (i = 0; i < ARRAY_SIZE(clks_pll); i++) {
			if (strcmp(clk_name, (clks_pll[i])->clk_name))
				continue;
			pll = clks_pll[i];
			break;
		}

		if (!pll) {
			printf("No pll clock '%s' registered\n", clk_name);
			return 0;
		}

		rate = clk_pll_recalc_rate(pll, 24000000U);

	} else if (clk_dev_type == CLK_DEV_FIXED) {
		struct clk_fixed *fixed = NULL;

		for (i = 0; i < ARRAY_SIZE(clks_fixed); i++) {
			if (strcmp(clk_name, clks_fixed[i].clk_name))
				continue;
			fixed = &clks_fixed[i];
		}

		if (!fixed) {
			printf("No fixed clock '%s' registered\n", clk_name);
			return 0;
		}

		rate = fixed->rate;

	} else if (clk_dev_type == CLK_DEV_MUX) {
		struct clk_lightmux *mux = NULL;
		const struct clk_info *mux_parent;

		for (i = 0; i < ARRAY_SIZE(clks_mux); i++) {
			if (strcmp(clk_name, clks_mux[i].clk_name))
				continue;
			mux = &clks_mux[i];
			break;
		}

		if (!mux) {
			printf("No mux clock '%s' registered\n", clk_name);
			return 0;
		}

		mux_parent = clk_light_get_parent(mux->clk_name);

		pr_debug("%s parent: %s\n", clk_name, mux_parent->clk_name);

		rate = clk_light_get_rate(mux_parent->clk_name, mux_parent->clk_dev_type);
		if (!rate)
			return rate;

	} else if (clk_dev_type == CLK_DEV_DIV) {
		struct clk_lightdiv *div = NULL;
		unsigned long prate;

		for (i = 0; i < ARRAY_SIZE(clks_div); i++) {
			if (strcmp(clk_name, clks_div[i].clk_name))
				continue;
			div = &clks_div[i];
			break;
		}

		if (!div) {
			printf("No div clock '%s' registered\n", clk_name);
			return -EINVAL;
		}

		prate = clk_light_get_rate(div->parent, div->p_clk_dev_type);
		if (!prate)
			return prate;

		rate = clk_lightdiv_recalc_rate(div, prate);

	} else if (clk_dev_type == CLK_DEV_FACTOR) {
		struct clk_light_fixed_factor *factor = NULL;
		unsigned long prate;

		for (i = 0; i < ARRAY_SIZE(clks_factor); i++) {
			if (strcmp(clk_name, clks_factor[i].clk_name))
				continue;
			factor = &clks_factor[i];
			break;
		}

		if (!factor) {
			printf("No factor clock '%s' registered\n", clk_name);
			return -EINVAL;
		}

		prate = clk_light_get_rate(factor->parent, factor->p_clk_dev_type);
		if (!prate)
			return prate;

		rate = clk_light_factor_recalc_rate(factor, prate);


	} else if (clk_dev_type == CLK_DEV_ROOT) {
		rate = 24000000U;
	} else {
		printf("Incorrect clock device type\n");
		rate = 0;
	}

	return rate;
}

int clk_light_set_rate(const char *clk_name, enum clk_device_type clk_dev_type, unsigned long rate)
{
	int i, ret = 0;

	if (clk_dev_type == CLK_DEV_MUX) {
		struct clk_lightmux *mux = NULL;
		const struct clk_info *mux_parent;

		for (i = 0; i < ARRAY_SIZE(clks_mux); i++) {
			if (strcmp(clk_name, clks_mux[i].clk_name))
				continue;
			mux = &clks_mux[i];
			break;
		}

		if (!mux) {
			printf("No mux clock '%s' registered\n", clk_name);
			return -EINVAL;
		}

		mux_parent = clk_light_get_parent(mux->clk_name);

		ret = clk_light_set_rate(mux_parent->clk_name, mux_parent->clk_dev_type, rate);

	} else if (clk_dev_type == CLK_DEV_PLL) {
		struct clk_lightpll *pll = NULL;

		for (i = 0; i < ARRAY_SIZE(clks_pll); i++) {
			if (strcmp(clk_name, (clks_pll[i])->clk_name))
				continue;
			pll = clks_pll[i];
			break;
		}

		if (!pll) {
			printf("No pll clock '%s' registered\n", clk_name);
			return -EINVAL;
		}

		ret = clk_pll_set_rate(pll, rate);

	} else if (clk_dev_type == CLK_DEV_DIV) {
		unsigned long prate;
		struct clk_lightdiv *div = NULL;

		for (i = 0; i < ARRAY_SIZE(clks_div); i++) {
			if (strcmp(clk_name, clks_div[i].clk_name))
				continue;
			div = &clks_div[i];
			break;
		}

		if (!div) {
			printf("No div clock '%s' registered\n", clk_name);
			return -EINVAL;
		}

		prate = clk_light_get_rate(div->parent, div->p_clk_dev_type);
		if (!prate)
			return -EINVAL;

		ret = clk_lightdiv_set_rate(div, rate, prate);

	} else {
		printf("Incorrect clock device type\n");
		return -EINVAL;
	}

	return ret;
}

/* disable some modules' clk that will not work in u-boot phase */
void ap_peri_clk_disable(void)
{
	unsigned int clk_cfg;

#if (defined CONFIG_TARGET_LIGHT_FM_C910_VAL_B) || (defined CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF) || (defined CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_DISCRETE) || (defined CONFIG_TARGET_LIGHT_FM_C910_B_POWER)
	clk_cfg = readl((void __iomem *)AP_PERI_CLK_CFG);
	clk_cfg &= ~(GMAC1_CLK_EN);
	writel(clk_cfg, (void __iomem *)AP_PERI_CLK_CFG);
#endif

	clk_cfg = readl((void __iomem *)MISCSYS_TEE_CLK_CTRL_TEE);
	clk_cfg &= ~(TEE_DMAC_CLK_EN);
	writel(clk_cfg, (void __iomem *)MISCSYS_TEE_CLK_CTRL_TEE);
}

void ap_dpu_clk_endisable(bool en)
{
	unsigned int cfg = readl((void __iomem *)VOSYS_CLK_GATE_REG);

	if (en)
		cfg |= (CLKCTRL_DPU_PIXELCLK0_EN | CLKCTRL_DPU_PIXELCLK1_EN | CLKCTRL_DPU_HCLK_EN |
				CLKCTRL_DPU_ACLK_EN | CLKCTRL_DPU_CCLK_EN);
	else
		cfg &= ~(CLKCTRL_DPU_PIXELCLK0_EN | CLKCTRL_DPU_PIXELCLK1_EN | CLKCTRL_DPU_HCLK_EN |
				CLKCTRL_DPU_ACLK_EN | CLKCTRL_DPU_CCLK_EN);

	writel(cfg, (void __iomem *)VOSYS_CLK_GATE_REG);
}

void ap_hdmi_clk_endisable(bool en)
{
	unsigned int cfg = readl((void __iomem *)VOSYS_CLK_GATE_REG);
	unsigned int cfg1 = readl((void __iomem *)VOSYS_CLK_GATE1_REG);

	if (en) {
		cfg |= (CLKCTRL_HDMI_SFR_CLK_EN | CLKCTRL_HDMI_PCLK_EN | CLKCTRL_HDMI_CEC_CLK_EN |
				CLKCTRL_HDMI_I2S_CLK_EN);
		cfg1 |= CLKCTRL_HDMI_PIXCLK_EN;
	} else {
		cfg &= ~(CLKCTRL_HDMI_SFR_CLK_EN | CLKCTRL_HDMI_PCLK_EN | CLKCTRL_HDMI_CEC_CLK_EN |
				CLKCTRL_HDMI_I2S_CLK_EN);
		cfg1 &= ~CLKCTRL_HDMI_PIXCLK_EN;
	}

	writel(cfg, (void __iomem *)VOSYS_CLK_GATE_REG);
	writel(cfg1, (void __iomem *)VOSYS_CLK_GATE1_REG);
}

void ap_mipi_dsi0_clk_endisable(bool en)
{
	unsigned int cfg = readl((void __iomem *)VOSYS_CLK_GATE_REG);
	unsigned int cfg1 = readl((void __iomem *)AP_DPU0_PLL_CFG1);

	if (en) {
		cfg |= (CLKCTRL_MIPI_DSI0_PCLK_EN | CLKCTRL_MIPI_DSI0_CFG_CLK_EN | CLKCTRL_MIPI_DSI0_REFCLK_EN |
				CLKCTRL_MIPIDSI0_PIXCLK_EN);
		cfg1 &= ~AP_DPU0_PLL_RST;
	} else {
		cfg &= ~(CLKCTRL_MIPI_DSI0_PCLK_EN | CLKCTRL_MIPI_DSI0_CFG_CLK_EN | CLKCTRL_MIPI_DSI0_REFCLK_EN |
				CLKCTRL_MIPIDSI0_PIXCLK_EN);
		cfg1 |= AP_DPU0_PLL_RST;
	}

	writel(cfg, (void __iomem *)VOSYS_CLK_GATE_REG);
	writel(cfg1, (void __iomem *)AP_DPU0_PLL_CFG1);
}

void ap_mipi_dsi1_clk_endisable(bool en)
{
	unsigned int cfg = readl((void __iomem *)VOSYS_CLK_GATE_REG);
	unsigned int cfg1 = readl((void __iomem *)AP_DPU1_PLL_CFG1);

	if (en) {
		cfg |= (CLKCTRL_MIPI_DSI1_PCLK_EN | CLKCTRL_MIPI_DSI1_CFG_CLK_EN | CLKCTRL_MIPI_DSI1_REFCLK_EN |
				CLKCTRL_MIPIDSI1_PIXCLK_EN);
		cfg1 &= ~AP_DPU1_PLL_RST;
	} else {
		cfg &= ~(CLKCTRL_MIPI_DSI1_PCLK_EN | CLKCTRL_MIPI_DSI1_CFG_CLK_EN | CLKCTRL_MIPI_DSI1_REFCLK_EN |
				CLKCTRL_MIPIDSI1_PIXCLK_EN);
		cfg1 |= AP_DPU1_PLL_RST;
	}

	writel(cfg, (void __iomem *)VOSYS_CLK_GATE_REG);
	writel(cfg1, (void __iomem *)AP_DPU1_PLL_CFG1);
}

static void ap_multimedia_div_num_set(enum multimedia_div_type type, unsigned int div_num)
{
	unsigned long div_reg;
	unsigned int div_num_shift, div_num_mask, div_en;
	unsigned int div_cfg;

	switch (type) {
	case VI_MIPI_CSI0_DIV:
		div_reg = VISYS_MIPI_CSI0_PIXELCLK;
		div_num_shift = VISYS_MIPI_CSI0_PIXELCLK_DIV_NUM_SHIFT;
		div_num_mask = VISYS_MIPI_CSI0_PIXELCLK_DIV_NUM_MASK;
		div_en = VISYS_MIPI_CSI0_PIXELCLK_DIV_EN;
		break;
	case VI_ISP0_CORE_DIV:
		div_reg = VISYS_ISP0_CLK_CFG;
		div_num_shift = VISYS_ISP0_CLK_DIV_NUM_SHIFT;
		div_num_mask = VISYS_ISP0_CLK_DIV_NUM_MASK;
		div_en = VISYS_ISP0_CLK_DIV_EN;
		break;
	case VI_ISP1_CORE_DIV:
		div_reg = VISYS_ISP1_CLK_CFG;
		div_num_shift = VISYS_ISP1_CLK_DIV_NUM_SHIFT;
		div_num_mask = VISYS_ISP1_CLK_DIV_NUM_MASK;
		div_en = VISYS_ISP1_CLK_DIV_EN;
		break;
	case VI_ISP_RY_CORE_DIV:
		div_reg = VISYS_ISP_RY_CLK_CFG;
		div_num_shift = VISYS_ISP_RY_CLK_DIV_NUM_SHIFT;
		div_num_mask = VISYS_ISP_RY_CLK_DIV_NUM_MASK;
		div_en = VISYS_ISP_RY_CLK_DIV_EN;
		break;
	case VO_DPU_CORE_DIV:
		div_reg = VOSYS_DPU_CCLK_CFG;
		div_num_shift = VOSYS_DPU_CCLK_DIV_NUM_SHIFT;
		div_num_mask = VOSYS_DPU_CCLK_DIV_NUM_MASK;
		div_en = VOSYS_DPU_CCLK_DIV_EN;
		break;
	case VO_DPU_PLL0_DIV:
		div_reg =APSYS_DPU0_PLL_DIV_CFG;
		div_num_shift = APSYS_DPU0_PLL_DIV_CLK_DIV_NUM_SHIFT;
		div_num_mask = APSYS_DPU0_PLL_DIV_CLK_DIV_NUM_MASK;
		div_en = APSYS_DPU0_PLL_DIV_CLK_DIV_EN;
		break;
	case VO_DPU_PLL1_DIV:
		div_reg =APSYS_DPU1_PLL_DIV_CFG;
		div_num_shift = APSYS_DPU1_PLL_DIV_CLK_DIV_NUM_SHIFT;
		div_num_mask = APSYS_DPU1_PLL_DIV_CLK_DIV_NUM_MASK;
		div_en = APSYS_DPU1_PLL_DIV_CLK_DIV_EN;
		break;
	default:
		printf("invalid ap multimedia divider type\n");
		return;
	}

	div_cfg = readl((void __iomem *)div_reg);
	div_cfg &= ~div_en;
	writel(div_cfg, (void __iomem *)div_reg);

	div_cfg &= ~(div_num_mask << div_num_shift);
	div_cfg |= (div_num & div_num_mask) << div_num_shift;
	div_cfg |= div_en;
	writel(div_cfg, (void __iomem *)div_reg);
}

int clk_config(void)
{
	unsigned long rate = clk_light_get_rate("c910_cclk", CLK_DEV_MUX);

	if (!rate)
		return -EINVAL;

	printf("C910 CPU FREQ: %ldMHz\n", rate / 1000000);

	rate = clk_light_get_rate("ahb2_cpusys_hclk", CLK_DEV_MUX);
	if (!rate)
		return -EINVAL;

	printf("AHB2_CPUSYS_HCLK FREQ: %ldMHz\n", rate / 1000000);

	rate = clk_light_get_rate("apb3_cpusys_pclk", CLK_DEV_DIV);
	if (!rate)
		return -EINVAL;

	printf("AHB3_CPUSYS_PCLK FREQ: %ldMHz\n", rate / 1000000);

	rate = clk_light_get_rate("perisys_ahb_hclk", CLK_DEV_MUX);
	if (!rate)
		return -EINVAL;

	printf("PERISYS_AHB_HCLK FREQ: %ldMHz\n", rate / 1000000);

	rate = clk_light_get_rate("perisys_apb_pclk", CLK_DEV_DIV);
	if (!rate)
		return -EINVAL;

	printf("PERISYS_APB_PCLK FREQ: %ldMHz\n", rate / 1000000);

	rate = clk_light_get_rate("gmac_pll_foutpostdiv", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("GMAC PLL POSTDIV FREQ: %ldMHZ\n", rate / 1000000);

	rate = clk_light_get_rate("dpu0_pll_foutpostdiv", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("DPU0 PLL POSTDIV FREQ: %ldMHZ\n", rate / 1000000);

	rate = clk_light_get_rate("dpu1_pll_foutpostdiv", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("DPU1 PLL POSTDIV FREQ: %ldMHZ\n", rate / 1000000);

#ifdef AUDIO_PLL_FREQ_PRINT
	rate = clk_light_get_rate("audio_pll_foutpostdiv", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("AUDIO PLL POSTDIV FREQ: %ldMHZ\n", rate / 1000000);

	rate = clk_light_get_rate("audio_pll_foutvco", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("AUDIO PLL FOUVCO FREQ: %ldMHZ\n", rate / 1000000);
#endif

#ifdef SYS_PLL_FREQ_PRINT
	rate = clk_light_get_rate("sys_pll_foutpostdiv", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("SYS PLL POSTDIV FREQ: %ldMHZ\n", rate / 1000000);

	rate = clk_light_get_rate("sys_pll_foutvco", CLK_DEV_PLL);
	if (!rate)
		return -EINVAL;

	printf("SYS PLL FOUVCO FREQ: %ldMHZ\n", rate / 1000000);

#endif

#ifdef CPU_PLL_FREQ_SET
	ret = clk_light_set_rate("cpu_pll1_foutpostdiv", CLK_DEV_PLL, 1500000000U);
	if (ret)
		return ret;

	ret = clk_light_set_parent("c910_cclk", "cpu_pll1_foutpostdiv");
	if (ret)
		return ret;
#endif

	/* The boards other than the LightA board perform the bus down-speed operation */

#if defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_DISCRETE) || defined (CONFIG_TARGET_LIGHT_FM_C910_BEAGLE) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_POWER) || defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_B)
	ap_multimedia_div_num_set(VI_MIPI_CSI0_DIV, 12); /* Input frquency: 2376MHZ */
	ap_multimedia_div_num_set(VI_ISP0_CORE_DIV, 15); /* Input frquency: 2376MHZ */
	ap_multimedia_div_num_set(VI_ISP1_CORE_DIV, 12); /* Input frquency: 2376MHZ */
	ap_multimedia_div_num_set(VI_ISP_RY_CORE_DIV, 12); /* Input frquency: 2376MHZ */
	ap_multimedia_div_num_set(VO_DPU_CORE_DIV, 4);	/* Input frquency: 2376MHZ */
	ap_multimedia_div_num_set(VO_DPU_PLL0_DIV, 16);
	ap_multimedia_div_num_set(VO_DPU_PLL1_DIV, 4);
#endif
	ap_hdmi_clk_endisable(false);
	ap_mipi_dsi1_clk_endisable(false);

	return 0;
}
