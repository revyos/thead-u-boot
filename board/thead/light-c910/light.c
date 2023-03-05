/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/types.h>
#include <thead/clock_config.h>
#include <linux/bitops.h>
#include <asm/arch-thead/light-iopmp.h>

#define SOC_PIN_AP_RIGHT_TOP          (0x0)
#define SOC_PIN_AP_LEFT_TOP           (0x1)
#define SOC_PIN_AON                   (0x2)

#define PAD_GRP_BASE_SET(x)           (x << 12)
#define PAD_GRP_IDX_GET(x)            ( (x >> 12) & 0xF)
#define PAD_INDEX(x)                  (x & 0xFFF)

#define LIGHT_GPIO1_BADDR	0xffec006000
#define LIGHT_GPIO2_BADDR	0xffe7f34000
#define LIGHT_GPIO3_BADDR	0xffe7f38000
#define LIGHT_GPIO1_13		(0x1 << 13)
#define LIGHT_GPIO3_21		(0x1 << 21)
#define LIGHT_GPIO2_26		(0x1 << 26)
#define LIGHT_GPIO2_28		(0x1 << 28)

#define GMAC0_APB3S_BADDR	0xffec003000
#define GMAC1_APB3S_BADDR	0xffec004000
static uint64_t apb3s_baddr;

typedef enum {
	UART0_TXD = PAD_GRP_BASE_SET(SOC_PIN_AP_RIGHT_TOP),
	UART0_RXD,
	QSPI0_SCLK,
	QSPI0_CSN0,
	QSPI0_CSN1,
	QSPI0_D0_MOSI,
	QSPI0_D1_MISO,
	QSPI0_D2_WP,
	QSPI0_D3_HOLD,
	I2C2_SCL,
	I2C2_SDA,
	I2C3_SCL,
	I2C3_SDA,
	GPIO2_13,
	SPI_SCLK,
	SPI_CSN,
	SPI_MOSI,
	SPI_MISO,
	GPIO2_18,
	GPIO2_19,
	GPIO2_20,
	GPIO2_21,
	GPIO2_22,
	GPIO2_23,
	GPIO2_24,
	GPIO2_25,
	SDIO0_WPRTN,
	SDIO0_DETN,
	SDIO1_WPRTN,
	SDIO1_DETN,
	GPIO2_30,
	GPIO2_31,
	GPIO3_0,
	GPIO3_1,
	GPIO3_2,
	GPIO3_3,
	HDMI_SCL,
	HDMI_SDA,
	HDMI_CEC,
	GMAC0_TX_CLK,
	GMAC0_RX_CLK,
	GMAC0_TXEN,
	GMAC0_TXD0,
	GMAC0_TXD1,
	GMAC0_TXD2,
	GMAC0_TXD3,
	GMAC0_RXDV,
	GMAC0_RXD0,
	GMAC0_RXD1,
	GMAC0_RXD2,
	GMAC0_RXD3,
	GMAC0_MDC,
	GMAC0_MDIO,
	GMAC0_COL,
	GMAC0_CRS,

	QSPI1_SCLK = PAD_GRP_BASE_SET(SOC_PIN_AP_LEFT_TOP),
	QSPI1_CSN0,
	QSPI1_D0_MOSI,
	QSPI1_D1_MISO,
	QSPI1_D2_WP,
	QSPI1_D3_HOLD,
	I2C0_SCL,
	I2C0_SDA,
	I2C1_SCL,
	I2C1_SDA,
	UART1_TXD,
	UART1_RXD,
	UART4_TXD,
	UART4_RXD,
	UART4_CTSN,
	UART4_RTSN,
	UART3_TXD,
	UART3_RXD,
	GPIO0_18,
	GPIO0_19,
	GPIO0_20,
	GPIO0_21,
	GPIO0_22,
	GPIO0_23,
	GPIO0_24,
	GPIO0_25,
	GPIO0_26,
	GPIO0_27,
	GPIO0_28,
	GPIO0_29,
	GPIO0_30,
	GPIO0_31,
	GPIO1_0,
	GPIO1_1,
	GPIO1_2,
	GPIO1_3,
	GPIO1_4,
	GPIO1_5,
	GPIO1_6,
	GPIO1_7,
	GPIO1_8,
	GPIO1_9,
	GPIO1_10,
	GPIO1_11,
	GPIO1_12,
	GPIO1_13,
	GPIO1_14,
	GPIO1_15,
	GPIO1_16,
	CLK_OUT_0,
	CLK_OUT_1,
	CLK_OUT_2,
	CLK_OUT_3,
	GPIO1_21,
	GPIO1_22,
	GPIO1_23,
	GPIO1_24,
	GPIO1_25,
	GPIO1_26,
	GPIO1_27,
	GPIO1_28,
	GPIO1_29,
	GPIO1_30,

	OSC_CLK_IN= PAD_GRP_BASE_SET(SOC_PIN_AON),
	OSC_CLK_OUT,
	SYS_RST_N,
	RTC_CLK_IN,
	RTC_CLK_OUT,
	TEST_MODE,
	DFT_PROT_DIS_0,
	DEBUG_MODE,
	POR_SEL,
	I2C_AON_SCL,
	I2C_AON_SDA,
	CPU_JTG_TCLK,
	CPU_JTG_TMS,
	CPU_JTG_TDI,
	CPU_JTG_TDO,
	CPU_JTG_TRST,
	AOGPIO_7,
	AOGPIO_8,
	AOGPIO_9,
	AOGPIO_10,
	AOGPIO_11,
	AOGPIO_12,
	AOGPIO_13,
	AOGPIO_14,
	AOGPIO_15,
	AUDIO_PA0,
	AUDIO_PA1,
	AUDIO_PA2,
	AUDIO_PA3,
	AUDIO_PA4,
	AUDIO_PA5,
	AUDIO_PA6,
	AUDIO_PA7,
	AUDIO_PA8,
	AUDIO_PA9,
	AUDIO_PA10,
	AUDIO_PA11,
	AUDIO_PA12,
	AUDIO_PA13,
	AUDIO_PA14,
	AUDIO_PA15,
	AUDIO_PA16,
	AUDIO_PA17,
	AUDIO_PA27,
	AUDIO_PA28,
	AUDIO_PA29,
	AUDIO_PA30,
} pin_name_t;

enum {
	PIN_PD,
	PIN_PU,
	PIN_PN,
};

enum {
	PIN_SPEED_NORMAL,
	PIN_SPEED_FAST
};

#define PIN_DRV_MAX             (15)

#define USBPHY_TEST_CTRL2   0xFFEC03F02C
#define USBPHY_TEST_CTRL3   0xFFEC03F030

#define SYSCLK_USB_CTRL		0xFFFC02D104
#define REF_SSP_EN		0xFFEC03F034
#define USB3_DRD_SWRST		0xFFEC02C014
#define SOC_OM_ADDRBASE		0xFFEF018010

#define ISO7816_CARD_ADDRBASE	0xFFF7F30000
#define REG_CONFIG_OFF		0x10
#define MIE			(1<<7)

#define LIGHT_IOPMP_DEFAULT_ATTR	0xffffffff
#define LIGHT_IOPMP_DEFAULT_OFF		0xc0
#ifndef CONFIG_TARGET_LIGHT_FPGA_FM_C910

#define PADMUX_REG_ADDR(base,index)  ((void*)(base) + ((index >> 3) << 2))
#define PADMUX_REG_BIT_POS(index)    ((index & 0x7) << 2)
#define PADMUX_REG_MASK(index)       (0xF << PADMUX_REG_BIT_POS(index))

#define PADCFG_REG_ADDR(base,index)  ((void*)(base) + ((index >> 1) << 2))
#define PADCFG_REG_BIT_POS(index)    ((index & 0x1) << 4)
#define PADCFG_REG_PULL_MASK(index)  (0x70 << PADCFG_REG_BIT_POS(index))
#define PADCFG_REG_DRV_MASK(index)   (0xF << PADCFG_REG_BIT_POS(index))
#define PADCFG_REG_SLEW_MASK(index) (0x100 << PADCFG_REG_BIT_POS(index))


#define PADMUX_CFG(grp_addr_base,index,pin_func)  \
        { \
		uint32_t val; \
		val = readl(PADMUX_REG_ADDR(grp_addr_base,index)); \
		val &=~PADMUX_REG_MASK(index); \
		val |=	pin_func << PADMUX_REG_BIT_POS(index); \
		writel(val,PADMUX_REG_ADDR(grp_addr_base,index)); \
	}

#define PADCFG_PULL(grp_addr_base,index,bpull,bpullup)  \
        { \
		uint32_t val; \
		val = readl(PADCFG_REG_ADDR(grp_addr_base,index)); \
		val &=~PADCFG_REG_PULL_MASK(index); \
		val |=	((bpull<<4)|(bpullup<<5)) << PADCFG_REG_BIT_POS(index); \
		writel(val,PADCFG_REG_ADDR(grp_addr_base,index)); \
	}

#define PADCFG_SLEW(grp_addr_base,index,bfast)  \
        { \
		uint32_t val; \
		val = readl(PADCFG_REG_ADDR(grp_addr_base,index)); \
		val &=~PADCFG_REG_SLEW_MASK(index); \
		val |=	(bfast<<8) << PADCFG_REG_BIT_POS(index); \
		writel(val,PADCFG_REG_ADDR(grp_addr_base,index)); \
	}

#define PADCFG_DRV(grp_addr_base,index,strength)  \
        { \
		uint32_t val; \
		val = readl(PADCFG_REG_ADDR(grp_addr_base,index)); \
		val &=~PADCFG_REG_DRV_MASK(index); \
		val |=	(strength) << PADCFG_REG_BIT_POS(index); \
		writel(val,PADCFG_REG_ADDR(grp_addr_base,index)); \
	}

struct pinmux_addr_t {
	uint32_t pin_grp_id;
	uint32_t *pin_grp_cfg_base;
	uint32_t *pin_grp_mux_base;
};

#define PIN_GRP_ADDR_DEF(grp_idx,cfg_base,mux_base) \
{ \
	.pin_grp_id = grp_idx, \
	.pin_grp_cfg_base =(uint32_t*) cfg_base, \
	.pin_grp_mux_base =(uint32_t*) mux_base, \
}

struct pinmux_addr_t g_soc_pin_grp_addr[] = {
	PIN_GRP_ADDR_DEF(SOC_PIN_AP_RIGHT_TOP,      0xFFEC007000,0xFFEC007400),
	PIN_GRP_ADDR_DEF(SOC_PIN_AP_LEFT_TOP,       0xFFE7F3C000,0xFFE7F3C400),
	PIN_GRP_ADDR_DEF(SOC_PIN_AON,               0xFFFFF4A000,0xFFFFF4A400),
};

static void iso7816_card_glb_interrupt_disable(void)
{
	writel(readl((void *)(ISO7816_CARD_ADDRBASE + REG_CONFIG_OFF))  & (~MIE), (void *)(ISO7816_CARD_ADDRBASE + REG_CONFIG_OFF));
}

static int  light_pinmx_get_cfg_base(pin_name_t pin_name, uint32_t** cfg_base)
{
	uint32_t grp_num = ARRAY_SIZE(g_soc_pin_grp_addr);
	uint32_t grp_idx = PAD_GRP_IDX_GET(pin_name);
	uint32_t i;

	for(i = 0; i < grp_num; i++) {
		if (grp_idx == g_soc_pin_grp_addr[i].pin_grp_id)
			break;
	}

	if (i == grp_num)
		return -1;

	*cfg_base = g_soc_pin_grp_addr[i].pin_grp_cfg_base;
	return 0;
}

static int  light_pinmx_get_mux_base(pin_name_t pin_name, uint32_t** cfg_base)
{
	uint32_t grp_num = ARRAY_SIZE(g_soc_pin_grp_addr);
	uint32_t grp_idx = PAD_GRP_IDX_GET(pin_name);
	uint32_t i;

	for(i = 0; i < grp_num; i++) {
		if (grp_idx == g_soc_pin_grp_addr[i].pin_grp_id)
			break;
	}

	if (i == grp_num)
		return -1;

	*cfg_base = g_soc_pin_grp_addr[i].pin_grp_mux_base;
	return 0;
}

/*******************************************************************************
 * function: danica_ioreuse_inital
 *
 * description:
 *   initial danica_pinmux
 *******************************************************************************/
static inline int light_pin_pllmode(pin_name_t pin_name, uint32_t mode)
{
	uint32_t bpull,bpullup;
	uint32_t *reg_addr;
	int      ret;

	ret = light_pinmx_get_cfg_base(pin_name,&reg_addr);
	if (ret)
		return ret;

	switch (mode)
	{
	case PIN_PD:
		bpull =1;
		bpullup = 0;
		/* code */
		break;
	case PIN_PU:
		bpull =1;
		bpullup = 1;
		/* code */
		break;
	case PIN_PN:
		bpull =0;
		bpullup = 0;
		/* code */
		break;
	default:
		return -1;
		break;
	}
	PADCFG_PULL(reg_addr,PAD_INDEX(pin_name),bpull,bpullup);

    return 0;
}

/**
  \brief       set pin speed
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   speed    io speed
  \return      error code
*/
static inline int light_pin_speed(pin_name_t pin_name, uint32_t speed)
{
	uint32_t bfast;
	uint32_t *reg_addr;
	int      ret;

	ret = light_pinmx_get_cfg_base(pin_name,&reg_addr);
	if (ret)
		return ret;

	switch (speed)
	{
	case PIN_SPEED_NORMAL:
		bfast = 0;
		break;
	case PIN_SPEED_FAST:
		bfast = 1;
		break;
	default:
		return -1;
	}

	PADCFG_SLEW(reg_addr,PAD_INDEX(pin_name),bfast);
    return 0;
}

/**
  \brief       set pin drive
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   drive    io drive
  \return      error code
*/
static inline int light_pin_drv_strength(pin_name_t pin_name, uint32_t strength)
{
	uint32_t *reg_addr;
	int      ret;

	ret = light_pinmx_get_cfg_base(pin_name,&reg_addr);
	if (ret)
		return ret;

	if(strength > PIN_DRV_MAX) {
		return -1;
	}

	PADCFG_DRV(reg_addr,PAD_INDEX(pin_name),strength);
	return 0;
}

int light_pin_mux(pin_name_t pin_name, uint32_t pin_func)
{
	uint32_t *reg_addr;
	int      ret;

	ret = light_pinmx_get_mux_base(pin_name,&reg_addr);
	if (ret)
		return ret;

	PADMUX_CFG(reg_addr,PAD_INDEX(pin_name),pin_func);
    return 0;
}

int light_pin_cfg(pin_name_t pin_name,uint32_t slew_rate, uint32_t pullmode,uint32_t drvstrength)
{
	int ret;

	ret = light_pin_speed(pin_name,slew_rate);
	if (ret)
		return ret;
	ret = light_pin_pllmode(pin_name,pullmode);
	if (ret )
		return ret;
	ret = light_pin_drv_strength(pin_name,drvstrength);

	return ret;
}
#endif

static struct light_iopmp_list {
        int iopmp_type;
        u64 reg;
} light_iopmp_lists[] = {
        {IOPMP_EMMC, 0xFFFC028000},
        {IOPMP_SDIO0, 0xFFFC029000},
        {IOPMP_SDIO1, 0xFFFC02A000},
        {IOPMP_USB0, 0xFFFC02E000},
        {IOPMP_AO, 0xFFFFC21000},
        {IOPMP_AUD, 0xFFFFC22000},
        {IOPMP_CHIP_DBG, 0xFFFFC37000},
        {IOPMP_EIP120I, 0xFFFF220000},
        {IOPMP_EIP120II, 0xFFFF230000},
        {IOPMP_EIP120III, 0xFFFF240000},
        {IOPMP_ISP0, 0xFFF4080000},
        {IOPMP_ISP1, 0xFFF4081000},
        {IOPMP_DW200, 0xFFF4082000},
        {IOPMP_VIPRE, 0xFFF4083000},
        {IOPMP_VENC, 0xFFFCC60000},
        {IOPMP_VDEC, 0xFFFCC61000},
        {IOPMP_G2D, 0xFFFCC62000},
        {IOPMP_FCE, 0xFFFCC63000},
        {IOPMP_NPU, 0xFFFF01C000},
        {IOPMP0_DPU, 0xFFFF520000},
        {IOPMP1_DPU, 0xFFFF521000},
        {IOPMP_GPU, 0xFFFF522000},
        {IOPMP_GMAC1, 0xFFFC001000},
        {IOPMP_GMAC2, 0xFFFC002000},
        {IOPMP_DMAC, 0xFFFFC20000},
        {IOPMP_TEE_DMAC, 0xFFFF250000},
	{IOPMP_DSP0, 0xFFFF058000},
	{IOPMP_DSP1, 0xFFFF059000},
	{IOPMP_AUDIO, 0xFFFFC22000},
	{IOPMP_AUDIO0, 0xFFCB02E000},
	{IOPMP_AUDIO1, 0xFFCB02F000},
};

static void light_iopmp_config(void)
{
	int i = 0;
	int entry_size;

	entry_size = ARRAY_SIZE(light_iopmp_lists);

	while (i < entry_size) {
		writel(LIGHT_IOPMP_DEFAULT_ATTR, (void *)(light_iopmp_lists[i].reg) + LIGHT_IOPMP_DEFAULT_OFF);
		i++;
	}
}

void board_lmb_reserve(struct lmb *lmb)
{
	lmb_add(lmb, (phys_addr_t)0x0, (phys_addr_t)0x20000000);
	lmb_add(lmb, (phys_addr_t)0xffffef8000, (phys_addr_t)(64*1024));
	lmb_add(lmb, (phys_addr_t)0xffc0000000, (phys_addr_t)(128 * 1024));
}

int dw_txclk_set_rate(u32 rate)
{
        switch (rate) {
        case 125000000:
		writel(0x4, (void *)(apb3s_baddr + 0xc));
		writel(0x80000004, (void *)(apb3s_baddr + 0xc));
                break;
        case 25000000:
		writel(0x14, (void *)(apb3s_baddr + 0xc));
		writel(0x80000014, (void *)(apb3s_baddr + 0xc));
                break;
        case 2500000:
		writel(0xc8, (void *)(apb3s_baddr + 0xc));
		writel(0x800000c8, (void *)(apb3s_baddr + 0xc));
                break;
        default:
                return -EINVAL;
        }

        return 0;
}

static void gmac_phy_rst(void)
{
	//GPIO reset
	writel(readl((void *)(LIGHT_GPIO3_BADDR + 0x4)) | LIGHT_GPIO3_21,
	       (void *)(LIGHT_GPIO3_BADDR + 0x4));
	writel(readl((void *)(LIGHT_GPIO1_BADDR + 0x4)) | LIGHT_GPIO1_13,
	       (void *)(LIGHT_GPIO1_BADDR + 0x4));

	writel(readl((void *)LIGHT_GPIO3_BADDR) & ~LIGHT_GPIO3_21,
	       (void *)LIGHT_GPIO3_BADDR);
	writel(readl((void *)LIGHT_GPIO1_BADDR) & ~LIGHT_GPIO1_13,
	       (void *)LIGHT_GPIO1_BADDR);
	/* At least 10ms */
	mdelay(12);
	writel(readl((void *)LIGHT_GPIO3_BADDR) | LIGHT_GPIO3_21,
	       (void *)LIGHT_GPIO3_BADDR);
	writel(readl((void *)LIGHT_GPIO1_BADDR) | LIGHT_GPIO1_13,
	       (void *)LIGHT_GPIO1_BADDR);
}

static void gmac_glue_init(uint64_t apb3s_baddr)
{
	/* Interface select mii:0   rgmii:1 */
	//writel(0x0, (void *)(apb3s_baddr + 0x1c));
	writel(0x1, (void *)(apb3s_baddr + 0x1c));
	writel(0x7e, (void *)(apb3s_baddr + 0x0));

	writel(0x4, (void *)(apb3s_baddr + 0xc));
	writel(0x80000004, (void *)(apb3s_baddr + 0xc));

	/* Clock source pad:0   pll:1 */
	//writel(0x0, (void *)(apb3s_baddr + 0x18));
	writel(0x1, (void *)(apb3s_baddr + 0x18));

	/* tx clk direction output:0 input:1 */
	//writel(0x1, (void *)(apb3s_baddr + 0x20));
	writel(0x0, (void *)(apb3s_baddr + 0x20));
}

void gmac_hw_init(void)
{
	gmac_phy_rst();
	apb3s_baddr = GMAC1_APB3S_BADDR;
	gmac_glue_init(apb3s_baddr);
	apb3s_baddr = GMAC0_APB3S_BADDR;
	gmac_glue_init(apb3s_baddr);
}

static void wifi_en(void)
{
	uint32_t val;

	writel(readl((void *)(LIGHT_GPIO2_BADDR + 0x4)) | LIGHT_GPIO2_26 | LIGHT_GPIO2_28,
	       (void *)(LIGHT_GPIO2_BADDR + 0x4));

	/* kernel driver will reset wifi module */
	val = readl((void *)LIGHT_GPIO2_BADDR);
	val |= LIGHT_GPIO2_26 | LIGHT_GPIO2_28;
	writel(val, (void *)LIGHT_GPIO2_BADDR);
}

static void usb_phy_test_config(void)
{
	writel(0x77F, (void *)USBPHY_TEST_CTRL3);
	writel(0x15150F0, (void *)USBPHY_TEST_CTRL2);
}

static void usb_clk_config(void)
{
	writel(readl((void *)SYSCLK_USB_CTRL) | 0xf, (void *)SYSCLK_USB_CTRL);
	writel(readl((void *)REF_SSP_EN) | 0x1, (void *)REF_SSP_EN);

	udelay(10);

	writel(0x0, (void *)USB3_DRD_SWRST);
	udelay(1000);
	writel(0x7, (void *)USB3_DRD_SWRST);
}

#if defined (CONFIG_TARGET_LIGHT_FPGA_FM_C910)
static void light_iopin_init(void)
{
	return;
}
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_A)
static void light_iopin_init(void)
{
	/*AON-PADMUX*/
	light_pin_mux(AOGPIO_7,3);
	light_pin_mux(AOGPIO_8,3);
	light_pin_mux(AOGPIO_9,3);
	light_pin_mux(AOGPIO_10,3);
	light_pin_mux(AOGPIO_12,1);
	light_pin_mux(AOGPIO_13,1);
	light_pin_mux(AUDIO_PA9,3);
	light_pin_mux(AUDIO_PA10,3);
	light_pin_mux(AUDIO_PA11,3);
	light_pin_mux(AUDIO_PA12,3);
	light_pin_mux(AUDIO_PA27,3);
	light_pin_mux(AUDIO_PA28,3);
	light_pin_mux(AUDIO_PA29,3);
	light_pin_mux(AUDIO_PA30,3);

	light_pin_cfg(I2C_AON_SCL,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(I2C_AON_SDA,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(AOGPIO_7,PIN_SPEED_NORMAL,PIN_PD,2);
	light_pin_cfg(AOGPIO_8,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_10,PIN_SPEED_NORMAL,PIN_PD,2);
	light_pin_cfg(AOGPIO_14,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(AUDIO_PA9,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(AUDIO_PA10,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AUDIO_PA11,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AUDIO_PA12,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AUDIO_PA27,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AUDIO_PA28,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AUDIO_PA29,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(AUDIO_PA30,PIN_SPEED_NORMAL,PIN_PU,2);

	/*AP-LEFT-TOP PADMUX*/
	light_pin_mux(QSPI1_CSN0,4);
	light_pin_mux(QSPI1_SCLK,4);
	light_pin_mux(QSPI1_D0_MOSI,4);
	light_pin_mux(QSPI1_D1_MISO,4);
	light_pin_mux(QSPI1_D2_WP,2);
	light_pin_mux(QSPI1_D3_HOLD,2);
	light_pin_cfg(QSPI1_CSN0,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_SCLK,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_D0_MOSI,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D1_MISO,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D2_WP,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_D3_HOLD,PIN_SPEED_NORMAL,PIN_PN,8);

	light_pin_cfg(I2C0_SCL,PIN_SPEED_NORMAL,PIN_PN,5);
	light_pin_cfg(I2C0_SDA,PIN_SPEED_NORMAL,PIN_PN,5);
	light_pin_cfg(I2C1_SCL,PIN_SPEED_NORMAL,PIN_PN,5);
	light_pin_cfg(I2C1_SDA,PIN_SPEED_NORMAL,PIN_PN,5);

	light_pin_mux(UART1_TXD,3);
	light_pin_mux(UART1_RXD,3);
	light_pin_cfg(UART1_TXD,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(UART1_RXD,PIN_SPEED_NORMAL,PIN_PU,2);

	light_pin_mux(UART3_TXD,1);
	light_pin_mux(UART3_RXD,1);
	light_pin_cfg(UART3_TXD,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(UART3_RXD,PIN_SPEED_NORMAL,PIN_PN,8);

	light_pin_mux(GPIO0_18,1);
	light_pin_mux(GPIO0_19,1);
	light_pin_cfg(GPIO0_18,PIN_SPEED_NORMAL,PIN_PN,5);
	light_pin_cfg(GPIO0_19,PIN_SPEED_NORMAL,PIN_PN,5);

	light_pin_cfg(GPIO0_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_22,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_23,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_31,PIN_SPEED_NORMAL,PIN_PU,2);

	light_pin_mux(CLK_OUT_0,1);
	light_pin_mux(CLK_OUT_1,1);
	light_pin_mux(CLK_OUT_2,1);
	light_pin_mux(CLK_OUT_3,1);

	/*AP-RIGHT-TOP PADMUX*/
	light_pin_mux(QSPI0_CSN0,2);
	light_pin_mux(QSPI0_SCLK,2);
	light_pin_mux(QSPI0_D0_MOSI,2);
	light_pin_mux(QSPI0_D1_MISO,2);
	light_pin_mux(QSPI0_D2_WP,2);
	light_pin_mux(QSPI0_D3_HOLD,2);
	light_pin_cfg(QSPI0_CSN0,PIN_SPEED_NORMAL,PIN_PN,0x8);
	light_pin_cfg(QSPI0_SCLK,PIN_SPEED_NORMAL,PIN_PN,0x8);
	light_pin_cfg(QSPI0_D0_MOSI,PIN_SPEED_NORMAL,PIN_PN,0x8);
	light_pin_cfg(QSPI0_D1_MISO,PIN_SPEED_NORMAL,PIN_PN,0x8);
	light_pin_cfg(QSPI0_D2_WP,PIN_SPEED_NORMAL,PIN_PN,0x8);
	light_pin_cfg(QSPI0_D3_HOLD,PIN_SPEED_NORMAL,PIN_PN,0x8);
	light_pin_cfg(I2C2_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C2_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(SPI_CSN,3);
	light_pin_mux(SPI_SCLK,3);
	light_pin_mux(SPI_MISO,3);
	light_pin_mux(SPI_MOSI,3);
	light_pin_mux(GPIO1_21,3);
	light_pin_mux(GPIO1_22,3);
	light_pin_mux(GPIO1_23,3);
	light_pin_mux(GPIO1_24,3);
	light_pin_mux(GPIO1_25,3);
	light_pin_cfg(SPI_CSN,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(SPI_SCLK,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(SPI_MISO,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(SPI_MOSI,PIN_SPEED_NORMAL,PIN_PU,2);

	light_pin_cfg(GPIO1_7, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_8, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_9, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_21, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_22, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_23, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_24, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_cfg(GPIO1_25, PIN_SPEED_NORMAL, PIN_PU, 0x2);
	light_pin_mux(GPIO2_18,1);
	light_pin_mux(GPIO2_19,1);
	light_pin_mux(GPIO2_20,1);
	light_pin_mux(GPIO2_21,1);
	light_pin_mux(GPIO2_22,1);
	light_pin_mux(GPIO2_23,1);
	light_pin_mux(GPIO2_24,1);
	light_pin_mux(GPIO2_25,1);

	light_pin_cfg(GPIO2_13, PIN_SPEED_NORMAL, PIN_PD, 0x2);
	light_pin_cfg(GPIO2_18, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_19, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_20, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_21, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_22, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_23, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_24, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_25, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(SDIO0_WPRTN,3);
	light_pin_mux(SDIO0_DETN,3);
	light_pin_mux(SDIO1_WPRTN,3);
	light_pin_cfg(SDIO0_WPRTN,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(SDIO0_DETN,PIN_SPEED_NORMAL,PIN_PU,2);

	light_pin_mux(GPIO2_30,1);
	light_pin_mux(GPIO2_31,1);
	light_pin_mux(GPIO3_0,1);
	light_pin_mux(GPIO3_1,1);
	light_pin_cfg(GPIO2_30, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_31, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_1, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(GPIO3_3,1);
	light_pin_cfg(GPIO3_3, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(GMAC0_COL,3);
	light_pin_mux(GMAC0_CRS,3);
	light_pin_cfg(GMAC0_COL,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(GMAC0_CRS,PIN_SPEED_NORMAL,PIN_PU,2);



	/* GMAC0 pad drive strength configurate to 0xF */
	light_pin_cfg(GMAC0_TX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXEN, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXDV, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);

}
#elif defined ( CONFIG_TARGET_LIGHT_FM_C910_VAL_B) || defined ( CONFIG_TARGET_LIGHT_FM_C910_B_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_POWER)
static void light_iopin_init(void)
{
	/* aon-padmux config */
	light_pin_cfg(I2C_AON_SCL,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(I2C_AON_SDA,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(AOGPIO_7,PIN_SPEED_NORMAL,PIN_PU,2);//soc_aud_3v3_en_reg
	light_pin_cfg(AOGPIO_8,PIN_SPEED_NORMAL,PIN_PU,2);//soc_aud_1v8_en_reg
	light_pin_cfg(AOGPIO_9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_10,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_11,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_14,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_15,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AOGPIO_7,3);
	light_pin_mux(AOGPIO_8,3);
	light_pin_mux(AOGPIO_9,3);
	light_pin_mux(AOGPIO_10,3);
	light_pin_mux(AOGPIO_11,0);
	light_pin_mux(AOGPIO_12,1);
	light_pin_mux(AOGPIO_13,1);
	light_pin_mux(AOGPIO_14,0);
	light_pin_cfg(AOGPIO_14,PIN_SPEED_NORMAL,PIN_PU,2);  ///WL_DEV_WAKE_HOST
	light_pin_mux(AOGPIO_15,0);
	light_pin_cfg(AOGPIO_15,PIN_SPEED_NORMAL,PIN_PU,2);  ///BT_DEV_WAKE_HOST

	light_pin_mux(AUDIO_PA4,3);                           ///NC
	light_pin_cfg(AUDIO_PA4,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AUDIO_PA5,3);                           ///NC
	light_pin_cfg(AUDIO_PA5,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AUDIO_PA11,3);                          ///NC
	light_pin_cfg(AUDIO_PA11,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AUDIO_PA16,3);                          ///NC
	light_pin_cfg(AUDIO_PA16,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(AUDIO_PA9,3);
	light_pin_cfg(AUDIO_PA9,PIN_SPEED_NORMAL,PIN_PU,2);// reset signal for audio-pa
	light_pin_mux(AUDIO_PA10,3);                          ///NC
	light_pin_cfg(AUDIO_PA10,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AUDIO_PA12,3);                          ///NC
	light_pin_cfg(AUDIO_PA12,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AUDIO_PA13,0);

	/*ap-padmux on left/top */
	light_pin_mux(QSPI1_CSN0,3);
	light_pin_cfg(QSPI1_CSN0,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_SCLK,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_D0_MOSI,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D1_MISO,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D2_WP,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_D3_HOLD,PIN_SPEED_NORMAL,PIN_PN,8);

	light_pin_cfg(I2C0_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C0_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(UART3_TXD,1);
	light_pin_mux(UART3_RXD,1);
	light_pin_cfg(UART3_TXD,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(UART3_RXD,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(GPIO0_18,1);
	light_pin_mux(GPIO0_19,1);
	light_pin_cfg(GPIO0_18,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(GPIO0_19,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(GPIO0_20,0);
	light_pin_mux(GPIO0_21,0);
	light_pin_mux(GPIO0_22,1);
	light_pin_mux(GPIO0_23,1);
	light_pin_mux(GPIO0_24,1);
	light_pin_mux(GPIO0_25,1);
	light_pin_mux(GPIO0_26,1);
	light_pin_mux(GPIO0_27,0);
	light_pin_mux(GPIO0_28,0);
	light_pin_mux(GPIO0_29,0);
	light_pin_mux(GPIO0_30,0);
	light_pin_cfg(GPIO0_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_27,PIN_SPEED_NORMAL,PIN_PN,2);//soc_dovdd18_ir_reg
	light_pin_cfg(GPIO0_28,PIN_SPEED_NORMAL,PIN_PN,2);//soc_avdd25_ir_reg
	light_pin_cfg(GPIO0_29,PIN_SPEED_NORMAL,PIN_PN,2);//soc_dvdd12_ir_reg
	light_pin_cfg(GPIO0_30,PIN_SPEED_NORMAL,PIN_PU,2);//soc_vdd_3v3_en_reg for uart/gmac/debug_port/hdmi/sd

	light_pin_mux(GPIO1_0,1);
	light_pin_mux(GPIO1_1,1);
	light_pin_mux(GPIO1_2,1);
	light_pin_mux(GPIO1_3,1);
	light_pin_mux(GPIO1_4,1);
	light_pin_mux(GPIO1_9,0);
	light_pin_mux(GPIO1_10,0);
	light_pin_mux(GPIO1_11,0);
	light_pin_mux(GPIO1_12,0);
	light_pin_mux(GPIO1_13,0);
	light_pin_mux(GPIO1_14,0);
	light_pin_mux(GPIO1_15,0);
	light_pin_mux(GPIO1_16,0);
	light_pin_mux(GPIO1_24,3);
	light_pin_mux(GPIO1_25,3);
	light_pin_cfg(GPIO1_0,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_1,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_2,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_3,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_4,PIN_SPEED_NORMAL,PIN_PN,2);
#ifndef defined (CONFIG_TARGET_LIGHT_FM_C910_B_POWER)
	light_pin_cfg(GPIO1_9,PIN_SPEED_NORMAL,PIN_PN,2); //soc_vdd18_lcd0_en_reg --backup regulator
#else
	light_pin_cfg(GPIO1_9,PIN_SPEED_NORMAL,PIN_PU,2); //soc_vdd18_lcd0_en_reg
#endif
	light_pin_cfg(GPIO1_10,PIN_SPEED_NORMAL,PIN_PN,2);//soc_lcd0_bias_en_reg
	light_pin_cfg(GPIO1_11,PIN_SPEED_NORMAL,PIN_PN,2);
#ifndef defined (CONFIG_TARGET_LIGHT_FM_C910_B_POWER)
	light_pin_cfg(GPIO1_12,PIN_SPEED_NORMAL,PIN_PN,2);//reg_tp_pwr_en --touch pannel
#else
	light_pin_cfg(GPIO1_12,PIN_SPEED_NORMAL,PIN_PU,2);//reg_tp_pwr_en --touch pannel
#endif
	light_pin_cfg(GPIO1_13,PIN_SPEED_NORMAL,PIN_PN,2);//soc_dovdd18_rgb_reg
	light_pin_cfg(GPIO1_14,PIN_SPEED_NORMAL,PIN_PN,2);//soc_dvdd12_rgb_reg
	light_pin_cfg(GPIO1_15,PIN_SPEED_NORMAL,PIN_PN,2);//soc_avdd28_rgb_reg
	light_pin_cfg(GPIO1_16,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_24,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_25,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(CLK_OUT_0,1);
	light_pin_mux(CLK_OUT_1,1);
	light_pin_mux(CLK_OUT_2,3);
	light_pin_mux(CLK_OUT_3,1);
	light_pin_cfg(CLK_OUT_2,PIN_SPEED_NORMAL,PIN_PU,2);///volume key

	/*ap-pdmux on righ/top*/
	light_pin_mux(QSPI0_SCLK,2);
	light_pin_mux(QSPI0_CSN0,2);
	light_pin_mux(QSPI0_CSN1,2);
	light_pin_mux(QSPI0_D0_MOSI,2);
	light_pin_mux(QSPI0_D1_MISO,2);
	light_pin_mux(QSPI0_D2_WP,2);
	light_pin_mux(QSPI0_D3_HOLD,2);

	light_pin_cfg(I2C2_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C2_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(SPI_CSN,3);
	light_pin_mux(SPI_MOSI,3);
	light_pin_mux(SPI_MISO,3);
	light_pin_mux(SPI_SCLK,3);
	light_pin_cfg(SPI_SCLK,PIN_SPEED_NORMAL,PIN_PU,2);  ///vdd5v_se_en

	light_pin_mux(GPIO2_18,0);
	light_pin_mux(GPIO2_19,0);
	light_pin_mux(GPIO2_20,0);
	light_pin_mux(GPIO2_21,0);
	light_pin_mux(GPIO2_22,0);
	light_pin_cfg(GPIO2_22,PIN_SPEED_NORMAL,PIN_PU,2); ///WL_HOST_WAKE_DEV
	light_pin_mux(GPIO2_23,0);
	light_pin_cfg(GPIO2_23,PIN_SPEED_NORMAL,PIN_PU,2); ///BT_DEV_WAKE_HOST
	light_pin_mux(GPIO2_24,0);
	light_pin_mux(GPIO2_25,0);

	light_pin_cfg(GPIO2_18,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_19,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_21,PIN_SPEED_NORMAL,PIN_PU,2); //audio-pa-reset
	light_pin_cfg(GPIO2_22,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_23,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_24,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_25,PIN_SPEED_NORMAL,PIN_PN,2);


	light_pin_mux(SDIO0_WPRTN,3);
	light_pin_cfg(SDIO0_WPRTN,PIN_SPEED_NORMAL,PIN_PN,2);///NC
	light_pin_mux(SDIO1_WPRTN,3);
	light_pin_cfg(SDIO1_WPRTN,PIN_SPEED_NORMAL,PIN_PD,2);//gpio2_28 vbus_en
	light_pin_mux(SDIO1_DETN,3);
	light_pin_cfg(SDIO1_DETN,PIN_SPEED_NORMAL,PIN_PN,2); //gpio2_29 wcn33_en

	light_pin_mux(GPIO2_30,0);
	light_pin_mux(GPIO2_31,0);
	light_pin_mux(GPIO3_0,0);
	light_pin_mux(GPIO3_1,0);
	light_pin_mux(GPIO3_2,1);
	light_pin_cfg(GPIO2_30, PIN_SPEED_NORMAL, PIN_PN, 0x2);//soc_avdd28_scan_en_reg
	light_pin_cfg(GPIO2_31, PIN_SPEED_NORMAL, PIN_PN, 0x2);//soc_dovdd18_scan_reg
	light_pin_cfg(GPIO3_0, PIN_SPEED_NORMAL, PIN_PN, 0x2); //soc_dvdd12_scan_reg
	light_pin_cfg(GPIO3_1, PIN_SPEED_NORMAL, PIN_PN, 0x2);
	light_pin_cfg(GPIO3_2, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(GMAC0_COL,3);
	light_pin_mux(GMAC0_CRS,3);
	light_pin_cfg(GMAC0_COL,PIN_SPEED_NORMAL,PIN_PU,2);//gmac0_phy0_rst
	light_pin_cfg(GMAC0_CRS,PIN_SPEED_NORMAL,PIN_PU,2);//gmac0_phy0_int

	/* GMAC0 pad drive strength configurate to 0xF */
	light_pin_cfg(GMAC0_TX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXEN, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXDV, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
}

#elif defined   (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_DISCRETE)
static void light_iopin_init(void)
{
	/* aon-padmux config */
	light_pin_cfg(I2C_AON_SCL,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(I2C_AON_SDA,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(AOGPIO_7,PIN_SPEED_NORMAL,PIN_PN,2);    ///NC
	light_pin_cfg(AOGPIO_8,PIN_SPEED_NORMAL,PIN_PN,2);    ///NC
	light_pin_cfg(AOGPIO_9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_10,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_11,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_14,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_15,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AOGPIO_7,3);
	light_pin_mux(AOGPIO_8,3);
	light_pin_mux(AOGPIO_9,3);
	light_pin_mux(AOGPIO_10,3);
	light_pin_mux(AOGPIO_11,0);
	light_pin_mux(AOGPIO_12,0);                           ///aud-tx
	light_pin_mux(AOGPIO_13,0);                           ///aud-rx
	light_pin_mux(AOGPIO_14,0);
	light_pin_mux(AOGPIO_15,0);


	light_pin_mux(AUDIO_PA9,3);
	light_pin_cfg(AUDIO_PA9,PIN_SPEED_NORMAL,PIN_PN,2);  ///audio-pa-rst
	light_pin_mux(AUDIO_PA10,3);                         ///aud_3v3_en
	light_pin_cfg(AUDIO_PA10,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_mux(AUDIO_PA12,3);                         ///aud_1v8_en
	light_pin_cfg(AUDIO_PA12,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_mux(AUDIO_PA13,0);

	/*ap-padmux on left/top */
	light_pin_mux(QSPI1_CSN0,3);
	light_pin_cfg(QSPI1_CSN0,PIN_SPEED_NORMAL,PIN_PN,8);     ///se-spi
	light_pin_cfg(QSPI1_SCLK,PIN_SPEED_NORMAL,PIN_PN,8);     ///se-spi
	light_pin_cfg(QSPI1_D0_MOSI,PIN_SPEED_NORMAL,PIN_PU,8);  ///se-spi
	light_pin_cfg(QSPI1_D1_MISO,PIN_SPEED_NORMAL,PIN_PU,8);  ///se-spi
	light_pin_cfg(QSPI1_D2_WP,PIN_SPEED_NORMAL,PIN_PN,8);    ///uart5-rx
	light_pin_cfg(QSPI1_D3_HOLD,PIN_SPEED_NORMAL,PIN_PN,8);  ///uart5-tx

	light_pin_cfg(I2C0_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C0_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(UART3_TXD,1);
	light_pin_mux(UART3_RXD,1);
	light_pin_cfg(UART3_TXD,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(UART3_RXD,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(GPIO0_18,1);
	light_pin_mux(GPIO0_19,1);
	light_pin_cfg(GPIO0_18,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(GPIO0_19,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(GPIO0_20,0);
	light_pin_mux(GPIO0_21,0);
	light_pin_mux(GPIO0_22,1);
	light_pin_mux(GPIO0_23,1);
	light_pin_mux(GPIO0_24,1);
	light_pin_mux(GPIO0_25,1);
	light_pin_mux(GPIO0_26,1);
	light_pin_mux(GPIO0_27,0);
	light_pin_mux(GPIO0_28,0);
	light_pin_mux(GPIO0_29,0);
	light_pin_mux(GPIO0_30,0);
	light_pin_cfg(GPIO0_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_27,PIN_SPEED_NORMAL,PIN_PN,2); /// NC
	light_pin_cfg(GPIO0_28,PIN_SPEED_NORMAL,PIN_PN,2); /// AVDD25_IR_EN
	light_pin_cfg(GPIO0_29,PIN_SPEED_NORMAL,PIN_PN,2); /// DVDD12_IR_EN
	light_pin_cfg(GPIO0_30,PIN_SPEED_NORMAL,PIN_PN,2); /// vdd-3v3-en (gmac,uart,led)

	light_pin_mux(GPIO1_0,1);
	light_pin_mux(GPIO1_1,1);
	light_pin_mux(GPIO1_2,1);
	light_pin_mux(GPIO1_3,1);
	light_pin_mux(GPIO1_4,1);
	light_pin_mux(GPIO1_9,0);
	light_pin_mux(GPIO1_10,0);
	light_pin_mux(GPIO1_11,0);
	light_pin_mux(GPIO1_12,0);
	light_pin_mux(GPIO1_13,0);
	light_pin_mux(GPIO1_14,0);
	light_pin_mux(GPIO1_15,0);
	light_pin_mux(GPIO1_16,0);
	light_pin_mux(GPIO1_21,3);
	light_pin_mux(GPIO1_22,3);
	light_pin_mux(GPIO1_23,3);
	light_pin_mux(GPIO1_24,3);
	light_pin_mux(GPIO1_25,3);
	light_pin_cfg(GPIO1_0,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_1,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_2,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_3,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_4,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_9,PIN_SPEED_NORMAL,PIN_PN,2);   ///VDD18_LCD0_EN
	light_pin_cfg(GPIO1_10,PIN_SPEED_NORMAL,PIN_PN,2);  ///LCD0_BIAS_EN
	light_pin_cfg(GPIO1_11,PIN_SPEED_NORMAL,PIN_PN,2);  /// LCD_ID0
	light_pin_cfg(GPIO1_12,PIN_SPEED_NORMAL,PIN_PN,2);  /// VDD28_TP0_EN
	light_pin_cfg(GPIO1_13,PIN_SPEED_NORMAL,PIN_PN,2);  /// DOVDD18_RGB_EN
	light_pin_cfg(GPIO1_14,PIN_SPEED_NORMAL,PIN_PN,2);  /// DVDD12_RGB_EN
	light_pin_cfg(GPIO1_15,PIN_SPEED_NORMAL,PIN_PN,2);  /// AVDD28_RGB_EN
	light_pin_cfg(GPIO1_16,PIN_SPEED_NORMAL,PIN_PN,2);  /// RGBCAM_RESET
	light_pin_cfg(GPIO1_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_22,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_23,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_24,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_25,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_26,PIN_SPEED_NORMAL,PIN_PN,2);  /// DBB2LEDDRIVER_EN1

	light_pin_mux(CLK_OUT_0,1);
	light_pin_mux(CLK_OUT_1,1);
	light_pin_mux(CLK_OUT_3,1);
	light_pin_mux(CLK_OUT_2,3);
	light_pin_cfg(CLK_OUT_2,PIN_SPEED_NORMAL,PIN_PU,2); ///volume key

	/*ap-pdmux on righ/top*/
	light_pin_mux(QSPI0_SCLK,2);
	light_pin_mux(QSPI0_CSN0,2);
	light_pin_mux(QSPI0_CSN1,2);
	light_pin_mux(QSPI0_D0_MOSI,2);
	light_pin_mux(QSPI0_D1_MISO,2);
	light_pin_mux(QSPI0_D2_WP,2);
	light_pin_mux(QSPI0_D3_HOLD,2);

	light_pin_cfg(I2C2_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C2_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(SPI_CSN,3);
	light_pin_mux(SPI_MOSI,3);
	light_pin_mux(SPI_MISO,3);
	light_pin_mux(SPI_SCLK,3);
	light_pin_cfg(SPI_SCLK,PIN_SPEED_NORMAL,PIN_PN,2); ///NC

	light_pin_mux(GPIO2_18,0);
	light_pin_mux(GPIO2_19,0);
	light_pin_mux(GPIO2_20,0);
	light_pin_mux(GPIO2_21,0);
	light_pin_mux(GPIO2_22,0);
	light_pin_mux(GPIO2_23,0);
	light_pin_mux(GPIO2_24,0);
	light_pin_mux(GPIO2_25,0);

	light_pin_cfg(GPIO2_18,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_19,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_21,PIN_SPEED_NORMAL,PIN_PN,2); ///NC
	light_pin_cfg(GPIO2_22,PIN_SPEED_NORMAL,PIN_PN,2); ///WIFI_BT_GPIO2
	light_pin_cfg(GPIO2_23,PIN_SPEED_NORMAL,PIN_PN,2); ///WIFI_BT_GPIO3
	light_pin_cfg(GPIO2_24,PIN_SPEED_NORMAL,PIN_PN,2); ///WIFI_BT_RST_N
	light_pin_cfg(GPIO2_25,PIN_SPEED_NORMAL,PIN_PU,2); ///KEY1


	light_pin_mux(SDIO0_WPRTN,3);
	light_pin_cfg(SDIO0_WPRTN,PIN_SPEED_NORMAL,PIN_PN,2); ///NC
	light_pin_mux(SDIO1_WPRTN,3);
	light_pin_cfg(SDIO1_WPRTN,PIN_SPEED_NORMAL,PIN_PU,2); ///VBUS_EN
	light_pin_mux(SDIO1_DETN,3);
	light_pin_cfg(SDIO1_DETN,PIN_SPEED_NORMAL,PIN_PN,2);  ///WCN33_EN

	light_pin_mux(GPIO2_30,0);
	light_pin_mux(GPIO2_31,0);
	light_pin_mux(GPIO3_0,0);

	light_pin_mux(GPIO3_1,0);
	light_pin_mux(GPIO3_2,1);
	light_pin_mux(GPIO3_3,0);
	light_pin_cfg(GPIO2_30, PIN_SPEED_NORMAL, PIN_PN, 0x2); ///NC
	light_pin_cfg(GPIO2_31, PIN_SPEED_NORMAL, PIN_PN, 0x2); ///NC
	light_pin_cfg(GPIO3_0, PIN_SPEED_NORMAL, PIN_PN, 0x2);  ///NC
	light_pin_cfg(GPIO3_1, PIN_SPEED_NORMAL, PIN_PN, 0x2);
	light_pin_cfg(GPIO3_2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_3, PIN_SPEED_NORMAL, PIN_PN, 0x2);  /// WAKEUP_IN_CAT1

	light_pin_mux(GMAC0_COL,3);
	light_pin_mux(GMAC0_CRS,3);
	light_pin_cfg(GMAC0_COL,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(GMAC0_CRS,PIN_SPEED_NORMAL,PIN_PU,2);

	/* GMAC0 pad drive strength configurate to 0xF */
	light_pin_cfg(GMAC0_TX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXEN, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXDV, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
}
#elif defined   (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_BEAGLE)
static void light_iopin_init(void)
{
	/* aon-padmux config */
	light_pin_cfg(I2C_AON_SCL,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(I2C_AON_SDA,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(AOGPIO_7,PIN_SPEED_NORMAL,PIN_PN,2);     ///NC
	light_pin_cfg(AOGPIO_8,PIN_SPEED_NORMAL,PIN_PN,2);     ///NC
	light_pin_cfg(AOGPIO_9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_10,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_11,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_14,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_15,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AOGPIO_7,3);
	light_pin_mux(AOGPIO_8,3);
	light_pin_mux(AOGPIO_9,3);
	light_pin_mux(AOGPIO_10,3);
	light_pin_mux(AOGPIO_11,0);
	light_pin_mux(AOGPIO_12,1);
	light_pin_mux(AOGPIO_13,1);
	light_pin_mux(AOGPIO_14,0);
	light_pin_mux(AOGPIO_15,0);

	light_pin_mux(AUDIO_PA9,3);                         ///AUDIO-PA-RESET
	light_pin_cfg(AUDIO_PA9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AUDIO_PA10,3);                        /// AUD-3V3-EN
	light_pin_cfg(AUDIO_PA10,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_mux(AUDIO_PA12,3);                        /// AUD-1V8-EN
	light_pin_cfg(AUDIO_PA12,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_mux(AUDIO_PA13,0);

	/*ap-padmux on left/top */
	light_pin_mux(QSPI1_CSN0,3);
	light_pin_cfg(QSPI1_CSN0,PIN_SPEED_NORMAL,PIN_PN,8);  ///se-spi
	light_pin_cfg(QSPI1_SCLK,PIN_SPEED_NORMAL,PIN_PN,8);  ///se-spi
	light_pin_cfg(QSPI1_D0_MOSI,PIN_SPEED_NORMAL,PIN_PU,8);  ///se-spi
	light_pin_cfg(QSPI1_D1_MISO,PIN_SPEED_NORMAL,PIN_PU,8);  ///se-spi
	light_pin_cfg(QSPI1_D2_WP,PIN_SPEED_NORMAL,PIN_PN,8);    ///NC
	light_pin_cfg(QSPI1_D3_HOLD,PIN_SPEED_NORMAL,PIN_PN,8);  ///NC

	light_pin_cfg(I2C0_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C0_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(UART3_TXD,1);
	light_pin_mux(UART3_RXD,1);
	light_pin_cfg(UART3_TXD,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(UART3_RXD,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(GPIO0_18,1);
	light_pin_mux(GPIO0_19,1);
	light_pin_cfg(GPIO0_18,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(GPIO0_19,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(GPIO0_20,0);
	light_pin_mux(GPIO0_21,0);
	light_pin_mux(GPIO0_22,1);
	light_pin_mux(GPIO0_23,1);
	light_pin_mux(GPIO0_24,1);
	light_pin_mux(GPIO0_25,1);
	light_pin_mux(GPIO0_26,1);
	light_pin_mux(GPIO0_27,0);
	light_pin_mux(GPIO0_28,0);
	light_pin_mux(GPIO0_29,0);
	light_pin_mux(GPIO0_30,0);
	light_pin_cfg(GPIO0_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_27,PIN_SPEED_NORMAL,PIN_PN,2);                          ///< NC(not used)
	light_pin_cfg(GPIO0_28,PIN_SPEED_NORMAL,PIN_PN,2);                          ///< AVDD25_IR_EN
	light_pin_cfg(GPIO0_29,PIN_SPEED_NORMAL,PIN_PN,2);                          ///< DVDD12_IR_EN
	light_pin_cfg(GPIO0_30,PIN_SPEED_NORMAL,PIN_PU,2);                          ///< gmac,uart,led

	light_pin_mux(GPIO1_0,1);
	light_pin_mux(GPIO1_1,1);
	light_pin_mux(GPIO1_2,1);
	light_pin_mux(GPIO1_3,1);
	light_pin_mux(GPIO1_4,1);
	light_pin_mux(GPIO1_9,0);
	light_pin_mux(GPIO1_10,0);
	light_pin_mux(GPIO1_11,0);
	light_pin_mux(GPIO1_12,0);
	light_pin_mux(GPIO1_13,0);
	light_pin_mux(GPIO1_14,0);
	light_pin_mux(GPIO1_15,0);
	light_pin_mux(GPIO1_16,0);
	light_pin_mux(GPIO1_21,3);
	light_pin_mux(GPIO1_22,3);
	light_pin_mux(GPIO1_23,3);
	light_pin_mux(GPIO1_24,3);
	light_pin_mux(GPIO1_25,3);
	light_pin_cfg(GPIO1_0,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_1,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_2,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_3,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_4,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_9,PIN_SPEED_NORMAL,PIN_PN,2);                          ///<VDD18_LCD0_EN
	light_pin_cfg(GPIO1_10,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<LCD0_BIAS_EN
	light_pin_cfg(GPIO1_11,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_12,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<TOUCH-PANNEL VDD28_TP0_EN
	light_pin_cfg(GPIO1_13,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<DOVDD18_RGB_EN
	light_pin_cfg(GPIO1_14,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<DVDD12_RGB_EN
	light_pin_cfg(GPIO1_15,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<AVDD28_RGB_EN
	light_pin_cfg(GPIO1_16,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_22,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_23,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<LED_PDN
	light_pin_cfg(GPIO1_24,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_25,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_26,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<DBB2LEDDRIVER_EN

	light_pin_mux(CLK_OUT_0,1);
	light_pin_mux(CLK_OUT_1,1);
	light_pin_mux(CLK_OUT_3,1);
	light_pin_mux(CLK_OUT_2,3);
	light_pin_cfg(CLK_OUT_2,PIN_SPEED_NORMAL,PIN_PU,2);                        ///volume key "-"

	/*ap-pdmux on righ/top*/
	light_pin_mux(QSPI0_SCLK,3);            ///NC
	light_pin_mux(QSPI0_CSN0,3);            ///NC
	light_pin_mux(QSPI0_CSN1,3);            ///NC
	light_pin_mux(QSPI0_D0_MOSI,3);         ///NC
	light_pin_mux(QSPI0_D1_MISO,3);         ///NC
	light_pin_mux(QSPI0_D2_WP,3);           ///NC
	light_pin_mux(QSPI0_D3_HOLD,3);         ///NC

	light_pin_cfg(I2C2_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C2_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(SPI_CSN,3);                                                /// W_DISABLE_CATE1
	light_pin_mux(SPI_MOSI,3);                                               /// NC
	light_pin_mux(SPI_MISO,3);                                               /// RERST1_N_CAT1
	light_pin_mux(SPI_SCLK,3);
	light_pin_cfg(SPI_SCLK,PIN_SPEED_NORMAL,PIN_PN,2);                       /// NC

	light_pin_mux(GPIO2_18,0);
	light_pin_mux(GPIO2_19,0);
	light_pin_mux(GPIO2_20,0);
	light_pin_mux(GPIO2_21,0);
	light_pin_mux(GPIO2_22,0);
	light_pin_mux(GPIO2_23,0);
	light_pin_mux(GPIO2_24,0);
	light_pin_mux(GPIO2_25,0);

	light_pin_cfg(GPIO2_18,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_19,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_21,PIN_SPEED_NORMAL,PIN_PN,2);                        ///<NC
	light_pin_cfg(GPIO2_22,PIN_SPEED_NORMAL,PIN_PN,2);                        ///<WIFI_BT_GPIO2
	light_pin_cfg(GPIO2_23,PIN_SPEED_NORMAL,PIN_PN,2);                        ///<WIFI_BT_GPIO3
	light_pin_cfg(GPIO2_24,PIN_SPEED_NORMAL,PIN_PN,2);                        ///<WIFI_BT_RST_N
	light_pin_cfg(GPIO2_25,PIN_SPEED_NORMAL,PIN_PU,2);                        ///KEY1


	light_pin_mux(SDIO0_WPRTN,3);
	light_pin_cfg(SDIO0_WPRTN,PIN_SPEED_NORMAL,PIN_PN,2);                    ///< NC
	light_pin_mux(SDIO1_WPRTN,3);
	light_pin_cfg(SDIO1_WPRTN,PIN_SPEED_NORMAL,PIN_PU,2);                   ///VBUS_EN
	light_pin_mux(SDIO1_DETN,3);
	light_pin_cfg(SDIO1_DETN,PIN_SPEED_NORMAL,PIN_PN,2);                    ///WCN_33_EN

	light_pin_mux(GPIO2_30,0);
	light_pin_mux(GPIO2_31,0);
	light_pin_mux(GPIO3_0,0);
	light_pin_mux(GPIO3_1,0);
	light_pin_mux(GPIO3_2,1);
	light_pin_mux(GPIO3_3,0);
	light_pin_cfg(GPIO2_30, PIN_SPEED_NORMAL, PIN_PN, 0x2);                 ///NC
	light_pin_cfg(GPIO2_31, PIN_SPEED_NORMAL, PIN_PN, 0x2);                 ///NC
	light_pin_cfg(GPIO3_0, PIN_SPEED_NORMAL, PIN_PN, 0x2);                  ///NC
	light_pin_cfg(GPIO3_1, PIN_SPEED_NORMAL, PIN_PN, 0x2);
	light_pin_cfg(GPIO3_2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_3, PIN_SPEED_NORMAL, PIN_PN, 0x2);

	light_pin_mux(GMAC0_COL,3);
	light_pin_mux(GMAC0_CRS,3);
	light_pin_cfg(GMAC0_COL,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(GMAC0_CRS,PIN_SPEED_NORMAL,PIN_PU,2);

	/* GMAC0 pad drive strength configurate to 0xF */
	light_pin_cfg(GMAC0_TX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXEN, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXDV, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
}
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_LPI4A)
static void light_iopin_init(void)
{
	/* aon-padmux config */
	light_pin_cfg(I2C_AON_SCL, PIN_SPEED_NORMAL, PIN_PN, 8);
	light_pin_cfg(I2C_AON_SDA, PIN_SPEED_NORMAL, PIN_PN, 8);

	light_pin_mux(CPU_JTG_TCLK, 3);
	light_pin_cfg(CPU_JTG_TCLK, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(CPU_JTG_TMS, 3);
	light_pin_cfg(CPU_JTG_TMS, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(CPU_JTG_TDI, 3);
	light_pin_cfg(CPU_JTG_TDI, PIN_SPEED_NORMAL, PIN_PN, 2);

	light_pin_mux(AOGPIO_7, 1);
	light_pin_mux(AOGPIO_8, 1);
	// light_pin_mux(AOGPIO_9, 0);
	light_pin_mux(AOGPIO_10, 1);
	light_pin_mux(AOGPIO_11, 1);
	light_pin_mux(AOGPIO_12, 1);
	light_pin_mux(AOGPIO_13, 1);
	light_pin_mux(AOGPIO_14, 0);
	// light_pin_mux(AOGPIO_15,0);
	light_pin_cfg(AOGPIO_7, PIN_SPEED_NORMAL, PIN_PN, 2);     ///NC
	light_pin_cfg(AOGPIO_8, PIN_SPEED_NORMAL, PIN_PN, 2);     ///NC
	// light_pin_cfg(AOGPIO_9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_10, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(AOGPIO_11, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(AOGPIO_12, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(AOGPIO_13, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(AOGPIO_14, PIN_SPEED_NORMAL, PIN_PN, 2);
	// light_pin_cfg(AOGPIO_15,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(AUDIO_PA0, 0);
	light_pin_cfg(AUDIO_PA0, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA1, 0);
	light_pin_cfg(AUDIO_PA1, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA3, 0);
	light_pin_cfg(AUDIO_PA3, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA4, 0);
	light_pin_cfg(AUDIO_PA4, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA5, 0);
	light_pin_cfg(AUDIO_PA5, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA6, 0);
	light_pin_cfg(AUDIO_PA6, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA7, 0);
	light_pin_cfg(AUDIO_PA7, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA14, 0);
	light_pin_cfg(AUDIO_PA14, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA15, 0);
	light_pin_cfg(AUDIO_PA15, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA16, 0);
	light_pin_cfg(AUDIO_PA16, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA17, 0);
	light_pin_cfg(AUDIO_PA17, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA29, 0);
	light_pin_cfg(AUDIO_PA29, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(AUDIO_PA30, 0);
	light_pin_cfg(AUDIO_PA30, PIN_SPEED_NORMAL, PIN_PN, 2);

	// light_pin_mux(AUDIO_PA9,3);                         ///AUDIO-PA-RESET
	// light_pin_cfg(AUDIO_PA9,PIN_SPEED_NORMAL,PIN_PN,2);
	// light_pin_mux(AUDIO_PA10,3);                        /// AUD-3V3-EN
	// light_pin_cfg(AUDIO_PA10,PIN_SPEED_NORMAL,PIN_PU,2);
	// light_pin_mux(AUDIO_PA12,3);                        /// AUD-1V8-EN
	// light_pin_cfg(AUDIO_PA12,PIN_SPEED_NORMAL,PIN_PU,2);
	// light_pin_mux(AUDIO_PA13,0);

	/*ap-padmux on left/top */
	light_pin_mux(QSPI1_SCLK, 4);
	light_pin_cfg(QSPI1_SCLK, PIN_SPEED_NORMAL,PIN_PN, 8);  ///se-spi
	light_pin_mux(QSPI1_CSN0, 4);
	light_pin_cfg(QSPI1_CSN0, PIN_SPEED_NORMAL, PIN_PN, 8);  ///se-spi
	light_pin_mux(QSPI1_D0_MOSI, 4);
	light_pin_cfg(QSPI1_D0_MOSI, PIN_SPEED_NORMAL, PIN_PU, 8);  ///se-spi
	light_pin_mux(QSPI1_D1_MISO, 4);
	light_pin_cfg(QSPI1_D1_MISO, PIN_SPEED_NORMAL, PIN_PU, 8);  ///se-spi
	light_pin_mux(QSPI1_D2_WP, 4);
	light_pin_cfg(QSPI1_D2_WP, PIN_SPEED_NORMAL, PIN_PN, 8);    ///NC
	// light_pin_mux(QSPI1_D3_HOLD, 4);
	// light_pin_cfg(QSPI1_D3_HOLD, PIN_SPEED_NORMAL, PIN_PN, 8);  ///NC

	light_pin_cfg(I2C0_SCL, PIN_SPEED_NORMAL, PIN_PN, 4);
	light_pin_cfg(I2C0_SDA, PIN_SPEED_NORMAL, PIN_PN, 4);
	light_pin_cfg(I2C1_SCL, PIN_SPEED_NORMAL, PIN_PN, 4);
	light_pin_cfg(I2C1_SDA, PIN_SPEED_NORMAL, PIN_PN, 4);

	light_pin_cfg(UART1_TXD, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(UART1_RXD, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(UART4_TXD, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(UART4_RXD, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(UART4_CTSN, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(UART4_RTSN, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(UART3_TXD, 1);
	light_pin_cfg(UART3_TXD, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_mux(UART3_RXD, 1);
	light_pin_cfg(UART3_RXD, PIN_SPEED_NORMAL, PIN_PN, 2);

	// light_pin_mux(GPIO0_18,1);
	// light_pin_mux(GPIO0_19,1);
	// light_pin_cfg(GPIO0_18,PIN_SPEED_NORMAL,PIN_PN,4);
	// light_pin_cfg(GPIO0_19,PIN_SPEED_NORMAL,PIN_PN,4);

	// light_pin_mux(GPIO0_20,0);
	// light_pin_mux(GPIO0_21,0);
	// light_pin_mux(GPIO0_22,1);
	// light_pin_mux(GPIO0_23,1);
	// light_pin_mux(GPIO0_24,1);
	// light_pin_mux(GPIO0_25,1);
	// light_pin_mux(GPIO0_26,1);
	// light_pin_mux(GPIO0_27,0);
	// light_pin_mux(GPIO0_28,0);
	// light_pin_mux(GPIO0_29,0);
	// light_pin_mux(GPIO0_30,0);
	// light_pin_cfg(GPIO0_20,PIN_SPEED_NORMAL,PIN_PN,2);
	// light_pin_cfg(GPIO0_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_24, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO0_25, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO0_27, PIN_SPEED_NORMAL, PIN_PN, 2);                          ///< NC(not used)
	light_pin_cfg(GPIO0_28, PIN_SPEED_NORMAL, PIN_PN, 2);                          ///< AVDD25_IR_EN
	// light_pin_cfg(GPIO0_29,PIN_SPEED_NORMAL,PIN_PN,2);                          ///< DVDD12_IR_EN
	light_pin_cfg(GPIO0_30, PIN_SPEED_NORMAL, PIN_PU, 2);                          ///< gmac,uart,led
	light_pin_cfg(GPIO0_31, PIN_SPEED_NORMAL, PIN_PN, 2);

	light_pin_mux(GPIO1_0, 0);
	// light_pin_mux(GPIO1_1,1);
	// light_pin_mux(GPIO1_2,1);
	light_pin_mux(GPIO1_3, 0);
	light_pin_mux(GPIO1_4, 0);
	light_pin_mux(GPIO1_5, 0);
	light_pin_mux(GPIO1_6, 0);
	light_pin_mux(GPIO1_9, 0);
	light_pin_mux(GPIO1_10, 0);
	// light_pin_mux(GPIO1_11,0);
	// light_pin_mux(GPIO1_12,0);
	light_pin_mux(GPIO1_13, 0);
	light_pin_mux(GPIO1_14, 0);
	// light_pin_mux(GPIO1_15,0);
	// light_pin_mux(GPIO1_16,0);
	light_pin_cfg(GPIO1_0, PIN_SPEED_NORMAL, PIN_PN, 2);
	// light_pin_cfg(GPIO1_1,PIN_SPEED_NORMAL,PIN_PN,2);
	// light_pin_cfg(GPIO1_2,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_3, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO1_4, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO1_5, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO1_6, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO1_9, PIN_SPEED_NORMAL, PIN_PN, 2);                          ///<VDD18_LCD0_EN
	light_pin_cfg(GPIO1_10, PIN_SPEED_NORMAL, PIN_PN, 2);                         ///<LCD0_BIAS_EN
	// light_pin_cfg(GPIO1_11,PIN_SPEED_NORMAL,PIN_PN,2);
	// light_pin_cfg(GPIO1_12,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<TOUCH-PANNEL VDD28_TP0_EN
	light_pin_cfg(GPIO1_13, PIN_SPEED_NORMAL, PIN_PN, 2);                         ///<DOVDD18_RGB_EN
	light_pin_cfg(GPIO1_14, PIN_SPEED_NORMAL, PIN_PN, 2);                         ///<DVDD12_RGB_EN
	// light_pin_cfg(GPIO1_15,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<AVDD28_RGB_EN
	// light_pin_cfg(GPIO1_16,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(CLK_OUT_0, 1);
	light_pin_cfg(CLK_OUT_0, PIN_SPEED_NORMAL, PIN_PU, 2);
	light_pin_mux(CLK_OUT_1, 1);
	light_pin_cfg(CLK_OUT_1, PIN_SPEED_NORMAL, PIN_PU, 2);
	light_pin_mux(CLK_OUT_2, 0);
	light_pin_cfg(CLK_OUT_2, PIN_SPEED_NORMAL, PIN_PU, 2);
	light_pin_mux(CLK_OUT_3, 0);
	light_pin_cfg(CLK_OUT_3, PIN_SPEED_NORMAL, PIN_PU, 2);    

	// light_pin_mux(GPIO1_21,3);
	light_pin_mux(GPIO1_22, 3);
	// light_pin_mux(GPIO1_23,3);
	light_pin_mux(GPIO1_24, 3);
	// light_pin_mux(GPIO1_25,3);
	// light_pin_mux(GPIO1_26,3);
	// light_pin_mux(GPIO1_27,3);
	light_pin_mux(GPIO1_28, 0);
	// light_pin_mux(GPIO1_29,3);
	light_pin_mux(GPIO1_30, 0);
	// light_pin_cfg(GPIO1_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_22, PIN_SPEED_NORMAL, PIN_PN,2);
	// light_pin_cfg(GPIO1_23,PIN_SPEED_NORMAL,PIN_PN,2);                         ///<LED_PDN
	light_pin_cfg(GPIO1_24, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO1_28, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO1_30, PIN_SPEED_NORMAL, PIN_PN, 2);                         ///<DBB2LEDDRIVER_EN

	light_pin_cfg(UART0_TXD, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(UART0_RXD, PIN_SPEED_NORMAL, PIN_PN, 2);            

	/*ap-pdmux on righ/top*/
	// light_pin_mux(QSPI0_SCLK,3);            ///NC
	// light_pin_mux(QSPI0_CSN0,3);            ///NC
	// light_pin_mux(QSPI0_CSN1,3);            ///NC
	// light_pin_mux(QSPI0_D0_MOSI,3);         ///NC
	// light_pin_mux(QSPI0_D1_MISO,3);         ///NC
	// light_pin_mux(QSPI0_D2_WP,3);           ///NC
	// light_pin_mux(QSPI0_D3_HOLD,3);         ///NC

	light_pin_cfg(I2C2_SCL, PIN_SPEED_NORMAL, PIN_PN, 4);
	light_pin_cfg(I2C2_SDA, PIN_SPEED_NORMAL, PIN_PN, 4);
	light_pin_cfg(I2C3_SCL, PIN_SPEED_NORMAL, PIN_PN, 4);
	light_pin_cfg(I2C3_SDA, PIN_SPEED_NORMAL, PIN_PN, 4);

	// light_pin_mux(SPI_CSN,3);                                                /// W_DISABLE_CATE1
	// light_pin_mux(SPI_MOSI,3);                                               /// NC
	// light_pin_mux(SPI_MISO,3);                                               /// RERST1_N_CAT1
	// light_pin_mux(SPI_SCLK,3);
	light_pin_cfg(SPI_CSN, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(SPI_MOSI, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(SPI_MISO, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(SPI_SCLK, PIN_SPEED_NORMAL, PIN_PN, 2);

	light_pin_mux(GPIO2_13, 0);
	light_pin_mux(GPIO2_18, 1);
	light_pin_mux(GPIO2_19, 1);
	light_pin_mux(GPIO2_20, 1);
	light_pin_mux(GPIO2_21, 1);
	light_pin_mux(GPIO2_22, 1);
	light_pin_mux(GPIO2_23, 1);
	light_pin_mux(GPIO2_24, 1);
	light_pin_mux(GPIO2_25, 1);

	light_pin_cfg(GPIO2_13, PIN_SPEED_NORMAL, PIN_PN, 2);
	light_pin_cfg(GPIO2_18, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_19, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_20, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_21, PIN_SPEED_NORMAL, PIN_PN, 0xF);                        ///<NC
	light_pin_cfg(GPIO2_22, PIN_SPEED_NORMAL, PIN_PN, 0xF);                        ///<WIFI_BT_GPIO2
	light_pin_cfg(GPIO2_23, PIN_SPEED_NORMAL, PIN_PN, 0xF);                        ///<WIFI_BT_GPIO3
	light_pin_cfg(GPIO2_24, PIN_SPEED_NORMAL, PIN_PN, 0xF);                        ///<WIFI_BT_RST_N
	light_pin_cfg(GPIO2_25, PIN_SPEED_NORMAL, PIN_PU, 0xF);                        ///KEY1

	light_pin_mux(SDIO0_DETN, 0);
	light_pin_cfg(SDIO0_DETN, PIN_SPEED_NORMAL, PIN_PN, 2);
	// light_pin_mux(SDIO0_WPRTN,3);
	// light_pin_cfg(SDIO0_WPRTN,PIN_SPEED_NORMAL,PIN_PN,2);                    ///< NC
	// light_pin_mux(SDIO1_WPRTN,3);
	// light_pin_cfg(SDIO1_WPRTN,PIN_SPEED_NORMAL,PIN_PU,2);                   ///VBUS_EN
	// light_pin_mux(SDIO1_DETN,3);
	// light_pin_cfg(SDIO1_DETN,PIN_SPEED_NORMAL,PIN_PN,2);                    ///WCN_33_EN

	light_pin_mux(GPIO2_30, 1);
	light_pin_mux(GPIO2_31, 1);
	light_pin_mux(GPIO3_0, 1);
	light_pin_mux(GPIO3_1, 1);
	light_pin_mux(GPIO3_2, 1);
	light_pin_mux(GPIO3_3, 1);
	light_pin_cfg(GPIO2_30, PIN_SPEED_NORMAL, PIN_PN, 0xF);                 ///NC
	light_pin_cfg(GPIO2_31, PIN_SPEED_NORMAL, PIN_PN, 0xF);                 ///NC
	light_pin_cfg(GPIO3_0, PIN_SPEED_NORMAL, PIN_PN, 0xF);                  ///NC
	light_pin_cfg(GPIO3_1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_3, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_cfg(HDMI_SCL, PIN_SPEED_NORMAL, PIN_PN, 0x2);
	light_pin_cfg(HDMI_SDA, PIN_SPEED_NORMAL, PIN_PN, 0x2);
	light_pin_cfg(HDMI_CEC, PIN_SPEED_NORMAL, PIN_PN, 0x2);

	/* GMAC0 pad drive strength configurate to 0xF */
	light_pin_cfg(GMAC0_TX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXEN, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXDV, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	// light_pin_cfg(GMAC0_MDC, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	// light_pin_cfg(GMAC0_MDIO, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(GMAC0_COL, 3);
	light_pin_mux(GMAC0_CRS, 3);
	light_pin_cfg(GMAC0_COL, PIN_SPEED_NORMAL, PIN_PU, 2);
	light_pin_cfg(GMAC0_CRS, PIN_SPEED_NORMAL, PIN_PU, 2);
}
#else
static void light_iopin_init(void)
{
	light_pin_cfg(I2C_AON_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C_AON_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(AOGPIO_8,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(AOGPIO_9,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_mux(AOGPIO_10,1);
	light_pin_mux(AOGPIO_11,1);
	light_pin_mux(AOGPIO_12,1);
	light_pin_mux(AOGPIO_13,1);
	light_pin_mux(AUDIO_PA30,3);

	/*qspi1 cs0 gpio0-1 pad strength and pin-pull mode*/
	light_pin_mux(QSPI1_CSN0,3);
	light_pin_cfg(QSPI1_CSN0,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_SCLK,PIN_SPEED_NORMAL,PIN_PN,8);
	light_pin_cfg(QSPI1_D0_MOSI,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D1_MISO,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D2_WP,PIN_SPEED_NORMAL,PIN_PU,8);
	light_pin_cfg(QSPI1_D3_HOLD,PIN_SPEED_NORMAL,PIN_PU,8);

	light_pin_cfg(I2C0_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C0_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C1_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(UART3_TXD,1);
	light_pin_mux(UART3_RXD,1);
	light_pin_cfg(UART3_TXD,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(UART3_RXD,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(GPIO0_18,1);
	light_pin_mux(GPIO0_19,1);
	light_pin_cfg(GPIO0_18,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(GPIO0_19,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(GPIO0_20,2);
	light_pin_mux(GPIO0_21,2);
	light_pin_cfg(GPIO0_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO0_30,PIN_SPEED_NORMAL,PIN_PU,2);

	light_pin_mux(GPIO1_0,1);
	light_pin_mux(GPIO1_1,1);
	light_pin_mux(GPIO1_2,1);
	light_pin_mux(GPIO1_3,1);
	light_pin_mux(GPIO1_4,1);
	light_pin_cfg(GPIO1_0,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_1,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_2,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_3,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO1_4,PIN_SPEED_NORMAL,PIN_PN,2);

	light_pin_mux(GPIO1_24,3);
	light_pin_cfg(GPIO1_24,PIN_SPEED_NORMAL,PIN_PD,2);
	light_pin_mux(GPIO1_25,3);
	light_pin_cfg(GPIO1_25,PIN_SPEED_NORMAL,PIN_PD,2);

	light_pin_mux(CLK_OUT_0,1);
	light_pin_mux(CLK_OUT_1,1);
	light_pin_mux(CLK_OUT_2,1);
	light_pin_mux(CLK_OUT_3,1);

	light_pin_cfg(I2C2_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C2_SDA,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SCL,PIN_SPEED_NORMAL,PIN_PN,4);
	light_pin_cfg(I2C3_SDA,PIN_SPEED_NORMAL,PIN_PN,4);

	light_pin_mux(GPIO2_18,1);
	light_pin_mux(GPIO2_19,1);
	light_pin_mux(GPIO2_20,1);
	light_pin_mux(GPIO2_21,1);
	light_pin_mux(GPIO2_22,1);
	light_pin_mux(GPIO2_23,1);
	light_pin_mux(GPIO2_24,1);
	light_pin_mux(GPIO2_25,1);

	light_pin_cfg(GPIO2_13,PIN_SPEED_NORMAL,PIN_PD,2);
	light_pin_cfg(GPIO2_18,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_19,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_20,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_21,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_22,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_23,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_24,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_25,PIN_SPEED_NORMAL,PIN_PN,2);
	light_pin_cfg(GPIO2_18, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_19, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_20, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_21, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_22, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_23, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_24, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_25, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(SDIO0_WPRTN,3);
	light_pin_mux(SDIO1_WPRTN,3);
	light_pin_mux(SDIO1_DETN,3);

	light_pin_mux(GPIO2_30,1);
	light_pin_mux(GPIO2_31,1);
	light_pin_mux(GPIO3_0,1);
	light_pin_mux(GPIO3_1,1);
	light_pin_mux(GPIO3_2,1);
	light_pin_cfg(GPIO2_30, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO2_31, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GPIO3_2, PIN_SPEED_NORMAL, PIN_PN, 0xF);

	light_pin_mux(GMAC0_COL,3);
	light_pin_mux(GMAC0_CRS,3);
	light_pin_cfg(GMAC0_COL,PIN_SPEED_NORMAL,PIN_PU,2);
	light_pin_cfg(GMAC0_CRS,PIN_SPEED_NORMAL,PIN_PU,2);

	/*spi0 cs0 gpio2-15 pad strength and pin-pull mode*/
	light_pin_mux(SPI_CSN,3);
	light_pin_cfg(SPI_CSN,PIN_SPEED_NORMAL,PIN_PN,5);
	light_pin_cfg(SPI_SCLK,PIN_SPEED_NORMAL,PIN_PN,5);
	light_pin_cfg(SPI_MISO,PIN_SPEED_NORMAL,PIN_PU,5);
	light_pin_cfg(SPI_MOSI,PIN_SPEED_NORMAL,PIN_PU,5);

	/*qspi0 cs0 gpio2-3, pad strength and pin-pull mode  */
	light_pin_mux(QSPI0_CSN0,3);
	light_pin_cfg(QSPI0_CSN0,PIN_SPEED_NORMAL,PIN_PN,0xA);
	light_pin_cfg(QSPI0_SCLK,PIN_SPEED_NORMAL,PIN_PN,0xA);
	light_pin_cfg(QSPI0_D0_MOSI,PIN_SPEED_NORMAL,PIN_PU,0xA);
	light_pin_cfg(QSPI0_D1_MISO,PIN_SPEED_NORMAL,PIN_PU,0xA);
	light_pin_cfg(QSPI0_D2_WP,PIN_SPEED_NORMAL,PIN_PU,0xA);
	light_pin_cfg(QSPI0_D3_HOLD,PIN_SPEED_NORMAL,PIN_PU,0xA);

	/* GMAC0 pad drive strength configurate to 0xF */
	light_pin_cfg(GMAC0_TX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RX_CLK, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXEN, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_TXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXDV, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD0, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD1, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD2, PIN_SPEED_NORMAL, PIN_PN, 0xF);
	light_pin_cfg(GMAC0_RXD3, PIN_SPEED_NORMAL, PIN_PN, 0xF);
}
#endif


static void light_pwm_config(void)
{
	/* pwm0 */
	writel(0x4b0, (void *)0xFFEC01C008);
	writel(0x258, (void *)0xFFEC01C00c);
	writel(0x328, (void *)0xFFEC01C000);
}

int board_init(void)
{
	light_iopmp_config();
	light_iopin_init();
	clk_config();

	gmac_hw_init();
	usb_clk_config();
	usb_phy_test_config();
	wifi_en();
	iso7816_card_glb_interrupt_disable();

	light_pwm_config();

	return 0;
}

static void light_usb_boot_check(void)
{
	int boot_mode;
	uchar env_enetaddr[6]={0};
	uchar env_enet1addr[6]={0};
	int env_ethaddr_flag,env_eth1addr_flag;
	boot_mode = readl((void *)SOC_OM_ADDRBASE) & 0x7;
	if (boot_mode & BIT(2))
		return;

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("usb_fastboot", "yes");
#endif
	/*Get this version ethaddr(mac addr) env,which follows one board, trans to next version env*/
	env_ethaddr_flag  = eth_env_get_enetaddr_by_index("eth", 0, env_enetaddr);
	env_eth1addr_flag = eth_env_get_enetaddr_by_index("eth", 1, env_enet1addr);

	run_command("env default -a -f", 0);
	/*If mac addr in last version env  is valid, before save,inherit env mac addr */
	if(env_ethaddr_flag){
		eth_env_set_enetaddr_by_index("eth", 0, env_enetaddr);
		run_command("printenv ethaddr",0);
	}else{
		printf("env ethaddr not exist or invalid\n");
	}

	if(env_eth1addr_flag){
		eth_env_set_enetaddr_by_index("eth", 1, env_enet1addr);
		run_command("printenv eth1addr",0);
	}else{
		printf("env eth1addr not exist or invalid\n");
	}

	run_command("env save", 0);
	run_command("run gpt_partition", 0);
	run_command("fastboot usb 0", 0);
}


int board_late_init(void)
{

#if CONFIG_IS_ENABLED(LIGHT_SEC_UPGRADE)
	extern void sec_upgrade_thread(void);
	extern void sec_firmware_version_dump(void);
	sec_upgrade_thread();
	sec_firmware_version_dump();
#endif

	light_usb_boot_check();
	ap_peri_clk_disable();
	return 0;
}

static int do_env_ethaddr_check(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	uchar env_enetaddr[6]={0};
	int i;
	for(i=0;i<2;i++){
		if(eth_env_get_enetaddr_by_index("eth", i, env_enetaddr)){
			printf("env (eth%d) MAC address ok- %pM\n",i, env_enetaddr);
		}
		else {
			printf("env (eth%d) MAC address invalid - %pM\n",i, env_enetaddr);
			printf(" * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00,\n"
				   " is not a multicast address(first byte low bit zero is not multi addr), \n"
					"and is not FF:FF:FF:FF:FF:FF.\n");
		}

	}
	return 0;

}

U_BOOT_CMD(
	chk_ethaddr, 2, 0,	do_env_ethaddr_check,
	"check ethaddrs in environment variables is valid",
	""
);
