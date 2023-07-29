// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Alibaba Group Holding Limited.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include "snps_sdhci.h"

#define HS400_DELAY_LANE	24

volatile int DELAY_LANE = 50;
volatile int manual_set_delay = 0;  //flag for cmd manual setted DELAY_LANE,non-zero is setted. auto clear in cmd

static void sdhci_phy_1_8v_init_no_pull(struct sdhci_host *host)
{
    uint32_t val;
    sdhci_writel(host, 1, DWC_MSHC_PTR_PHY_R);
    sdhci_writeb(host, 1 << 4, PHY_SDCLKDL_CNFG_R);
    sdhci_writeb(host, 0x40, PHY_SDCLKDL_DC_R);
    sdhci_writeb(host, 0xa, PHY_DLL_CNFG2_R);

    val = sdhci_readb(host, PHY_SDCLKDL_CNFG_R);
    val &= ~(1 << 4);
    sdhci_writeb(host, val, PHY_SDCLKDL_CNFG_R);


    val = sdhci_readw(host, PHY_CMDPAD_CNFG_R);
    sdhci_writew(host, val | 1, PHY_CMDPAD_CNFG_R);

    val = sdhci_readw(host, PHY_DATAPAD_CNFG_R);
    sdhci_writew(host, val | 1, PHY_DATAPAD_CNFG_R);

    val = sdhci_readw(host, PHY_RSTNPAD_CNFG_R);
    sdhci_writew(host, val | 1, PHY_RSTNPAD_CNFG_R);

    val = sdhci_readw(host, PHY_STBPAD_CNFG_R);
    sdhci_writew(host, val | 1, PHY_STBPAD_CNFG_R);

    val = sdhci_readb(host, PHY_DLL_CTRL_R);
    sdhci_writeb(host, val | 1, PHY_DLL_CTRL_R);
}

static void sdhci_phy_3_3v_init_no_pull(struct sdhci_host *host)
{
    uint32_t val;
    sdhci_writel(host, 1, DWC_MSHC_PTR_PHY_R);
    sdhci_writeb(host, 1 << 4, PHY_SDCLKDL_CNFG_R);
    sdhci_writeb(host, 0x40, PHY_SDCLKDL_DC_R);
    sdhci_writeb(host, 0xa, PHY_DLL_CNFG2_R);

    val = sdhci_readb(host, PHY_SDCLKDL_CNFG_R);
    val &= ~(1 << 4);
    sdhci_writeb(host, val, PHY_SDCLKDL_CNFG_R);

    val = sdhci_readw(host, PHY_CMDPAD_CNFG_R);
    sdhci_writew(host, val | 2, PHY_CMDPAD_CNFG_R);

    val = sdhci_readw(host, PHY_DATAPAD_CNFG_R);
    sdhci_writew(host, val | 2, PHY_DATAPAD_CNFG_R);

    val = sdhci_readw(host, PHY_RSTNPAD_CNFG_R);
    sdhci_writew(host, val | 2, PHY_RSTNPAD_CNFG_R);

    val = sdhci_readw(host, PHY_STBPAD_CNFG_R);
    sdhci_writew(host, val | 2, PHY_STBPAD_CNFG_R);

    val = sdhci_readb(host, PHY_DLL_CTRL_R);
    sdhci_writeb(host, val | 1, PHY_DLL_CTRL_R);
}

static void sdhci_phy_1_8v_init(struct sdhci_host *host)
{
    uint32_t val;
	struct snps_sdhci_plat *plat = dev_get_platdata(host->mmc->dev);
    if(plat->pull_up_en == false) {
        sdhci_phy_1_8v_init_no_pull(host);
        return;
    }
    //set driving force
    sdhci_writel(host, (1 << PHY_RSTN) | (0xc << PAD_SP) | (0xc << PAD_SN), PHY_CNFG_R);

    //disable delay lane
    sdhci_writeb(host, 1 << UPDATE_DC, PHY_SDCLKDL_CNFG_R);
    //set delay lane
    sdhci_writeb(host, DELAY_LANE, PHY_SDCLKDL_DC_R);
    sdhci_writeb(host, 0xa, PHY_DLL_CNFG2_R);
    //enable delay lane
    val = sdhci_readb(host, PHY_SDCLKDL_CNFG_R);
    val &= ~(1 << UPDATE_DC);
    sdhci_writeb(host, val, PHY_SDCLKDL_CNFG_R);

    val = (1 << RXSEL) | (1 << WEAKPULL_EN) | (3 << TXSLEW_CTRL_P) | (3 << TXSLEW_CTRL_N);
    sdhci_writew(host, val, PHY_CMDPAD_CNFG_R);
    sdhci_writew(host, val, PHY_DATAPAD_CNFG_R);
    sdhci_writew(host, val, PHY_RSTNPAD_CNFG_R);

    val = (3 << TXSLEW_CTRL_P) | (3 << TXSLEW_CTRL_N);
    sdhci_writew(host, val, PHY_CLKPAD_CNFG_R);

    val = (1 << RXSEL) | (2 << WEAKPULL_EN) | (3 << TXSLEW_CTRL_P) | (3 << TXSLEW_CTRL_N);
    sdhci_writew(host, val, PHY_STBPAD_CNFG_R);

    /* enable data strobe mode */
    sdhci_writeb(host, 0, PHY_DLL_CTRL_R);
    sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);
    sdhci_writew(host, 0x8000, PHY_DLLBT_CNFG_R);
    sdhci_writeb(host, 3 << SLV_INPSEL, PHY_DLLDL_CNFG_R);
    sdhci_writeb(host, 0x25, PHY_DLL_CNFG1_R);
    sdhci_writew(host, 0x7, SDHCI_CLOCK_CONTROL);
    sdhci_writeb(host, (1 << DLL_EN),  PHY_DLL_CTRL_R);
}

static void sdhci_phy_3_3v_init(struct sdhci_host *host)
{
    uint32_t val;
	struct snps_sdhci_plat *plat = dev_get_platdata(host->mmc->dev);
    if(plat->pull_up_en == false) {
        sdhci_phy_3_3v_init_no_pull(host);
        return;
    }
    //set driving force
    sdhci_writel(host, (1 << PHY_RSTN) | (0xc << PAD_SP) | (0xc << PAD_SN), PHY_CNFG_R);

    //disable delay lane
    sdhci_writeb(host, 1 << UPDATE_DC, PHY_SDCLKDL_CNFG_R);
    //set delay lane
    sdhci_writeb(host, DELAY_LANE, PHY_SDCLKDL_DC_R);
    sdhci_writeb(host, 0xa, PHY_DLL_CNFG2_R);
    //enable delay lane
    val = sdhci_readb(host, PHY_SDCLKDL_CNFG_R);
    val &= ~(1 << UPDATE_DC);
    sdhci_writeb(host, val, PHY_SDCLKDL_CNFG_R);

    val = (2 << RXSEL) | (1 << WEAKPULL_EN) | (3 << TXSLEW_CTRL_P) | (3 << TXSLEW_CTRL_N);
    sdhci_writew(host, val, PHY_CMDPAD_CNFG_R);
    sdhci_writew(host, val, PHY_DATAPAD_CNFG_R);
    sdhci_writew(host, val, PHY_RSTNPAD_CNFG_R);

    val = (3 << TXSLEW_CTRL_P) | (3 << TXSLEW_CTRL_N);
    sdhci_writew(host, val, PHY_CLKPAD_CNFG_R);

    val = (2 << RXSEL) | (2 << WEAKPULL_EN) | (3 << TXSLEW_CTRL_P) | (3 << TXSLEW_CTRL_N);
    sdhci_writew(host, val, PHY_STBPAD_CNFG_R);

    sdhci_writeb(host, (1 << DLL_EN),  PHY_DLL_CTRL_R);
    /*set i wait*/
    sdhci_writeb(host, 0x5, PHY_DLL_CNFG1_R);
}

void snps_set_uhs_timing(struct sdhci_host *host)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u32 reg;
	int restore_delay;
	reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	reg &= ~SDHCI_CTRL_UHS_MASK;
    
	if(manual_set_delay){
		DELAY_LANE = DELAY_LANE & 0x7f; /*limit bit[0:6]*/
		printf("%s: manual set delay (%d) active \n",host->name,DELAY_LANE);
	}
	switch (mmc->selected_mode) {
	case UHS_SDR50:
	case MMC_HS_52:
		sdhci_phy_1_8v_init(host);
		reg |= SDHCI_CTRL_UHS_SDR50;
		break;
	case UHS_DDR50:
	case MMC_DDR_52:
		sdhci_phy_1_8v_init(host);
		reg |= SDHCI_CTRL_UHS_DDR50;
		break;
	case UHS_SDR104:
	case MMC_HS_200:
		sdhci_phy_1_8v_init(host);
		reg |= SDHCI_CTRL_UHS_SDR104;
		break;
	case MMC_HS_400:
		restore_delay = DELAY_LANE;
		if(!manual_set_delay){  /*default not set manual in cmd,when set in cmd,use DELAY_LANE set in cmd*/
			 DELAY_LANE = HS400_DELAY_LANE;
		}
		sdhci_phy_1_8v_init(host);
		reg |= SNPS_SDHCI_CTRL_HS400;
		DELAY_LANE = restore_delay; /*restore for other modes*/
		break;
	default:
		sdhci_phy_3_3v_init(host);
		reg |= SDHCI_CTRL_UHS_SDR12;
	}

	sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);

    if (mmc->selected_mode == MMC_HS_400) {
       // //disable delay lane
       // sdhci_writeb(host, 1 << UPDATE_DC, PHY_SDCLKDL_CNFG_R);
       // //set delay lane
       // sdhci_writeb(host, DELAY_LANE, PHY_SDCLKDL_DC_R);
       // //enable delay lane
       // reg = sdhci_readb(host, PHY_SDCLKDL_CNFG_R);
       // reg &= ~(1 << UPDATE_DC);
       // sdhci_writeb(host, reg, PHY_SDCLKDL_CNFG_R);

        //disable auto tuning
        reg = sdhci_readl(host, AT_CTRL_R);
        reg &= ~1;
        sdhci_writel(host, reg, AT_CTRL_R);
    } else {
        sdhci_writeb(host, 0, PHY_DLLDL_CNFG_R);
    }
}

static void snps_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	struct snps_sdhci_plat *plat = dev_get_platdata(host->mmc->dev);
	u32 reg;

    reg = sdhci_readw(host, EMMC_CTRL_R);
	if (IS_SD(host->mmc)) {
        reg &=~EMMC_CARD;
	} else {
        reg |=EMMC_CARD;
    }

    sdhci_writeb(host, reg, EMMC_CTRL_R);

    if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
        reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
        reg |= SDHCI_CTRL_VDD_180;
        sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
        sdhci_phy_1_8v_init(host);
    } else {
        reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
        reg &= ~SDHCI_CTRL_VDD_180;
	if (plat->io_fixed_1v8)
	    reg |= SDHCI_CTRL_VDD_180;
        sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
        sdhci_phy_3_3v_init(host);
    }

    snps_set_uhs_timing(host);
}

extern int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
static int snps_execute_tuning(struct mmc *mmc, u8 opcode)
{
#define SDHCI_TUNING_LOOP_COUNT 128
	struct sdhci_host *host = dev_get_priv(mmc->dev);
    struct mmc_cmd cmd;
    struct mmc_data data;
    char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
    uint32_t val = 0;
    uint16_t ctrl = 0;

    debug("%s\n", __func__);

    sdhci_writeb(host, 3 << INPSEL_CNFG, PHY_ATDL_CNFG_R);

    val = sdhci_readl(host, AT_CTRL_R);

    val &= ~((1 << CI_SEL) | (1 << RPT_TUNE_ERR)\
            | (1 << SW_TUNE_EN) |(0xf << WIN_EDGE_SEL));
    val |= (1 << AT_EN) | (1 << SWIN_TH_EN) | (1 << TUNE_CLK_STOP_EN)\
            | (1 << PRE_CHANGE_DLY) | (3 << POST_CHANGE_DLY) | (9 << SWIN_TH_VAL);

    sdhci_writel(host, val, AT_CTRL_R);
    val = sdhci_readl(host, AT_CTRL_R);
    if(!(val & (1 << AT_EN))) {
        printf("*****Auto Tuning is NOT Enable!!!\n");
        return -1;
    }

    //Start Tuning
    ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

    mdelay(1);

    do {
        cmd.cmdidx = opcode;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 0;

        data.blocksize = 64;
        data.blocks = 1;
        data.flags = MMC_DATA_READ;

        if (tuning_loop_counter-- == 0)
            break;

        if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200 &&
            mmc->bus_width == 8)
            data.blocksize = 128;

        sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
                            data.blocksize),
                 SDHCI_BLOCK_SIZE);
        sdhci_writew(host, data.blocks, SDHCI_BLOCK_COUNT);
        sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

        mmc_send_cmd(mmc, &cmd, NULL);
        ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

        if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK)
            udelay(1);

	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (tuning_loop_counter < 0) {
		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		sdhci_writel(host, ctrl, SDHCI_HOST_CONTROL2);
	}

	if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		printf("%s:Tuning failed\n", __func__);
		return -1;
	}

    return 0;
}

const struct sdhci_ops snps_ops = {
	.platform_execute_tuning = &snps_execute_tuning,
	.set_control_reg = &snps_sdhci_set_control_reg,
};

static int snps_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct snps_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);

	u32 max_clk;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	plat->pull_up_en = false;
	plat->io_fixed_1v8 = false;
	host->name = dev->name;
	host->ioaddr = (void *)devfdt_get_addr(dev);

	max_clk = clk_get_rate(&clk);
	if (IS_ERR_VALUE(max_clk)) {
		ret = max_clk;
		goto err;
	}
	//get Maximum Base Clock frequency from dts clock-frequency
	if(0 == dev_read_u32(dev, "clock-frequency", &max_clk)){
		host->max_clk = max_clk;
	}
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	upriv->mmc = host->mmc;
	host->ops = &snps_ops;

 	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		goto err;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		goto err;

	host->voltages = MMC_VDD_33_34;
	ret = sdhci_probe(dev);
	if (ret)
		goto err;

	if (dev_read_bool(dev, "pull_up"))
		plat->pull_up_en = true;
	if (dev_read_bool(dev, "io_fixed_1v8"))
		plat->io_fixed_1v8 = true;

	uint16_t val = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	if (host->voltages == MMC_VDD_165_195) {
		val |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, val, SDHCI_HOST_CONTROL2);
		sdhci_phy_1_8v_init(host);
	} else {
		val &= ~SDHCI_CTRL_VDD_180;
		if (plat->io_fixed_1v8)
			val |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, val, SDHCI_HOST_CONTROL2);
		sdhci_phy_3_3v_init(host);
	}

	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

	return 0;

err:
	clk_disable(&clk);
	clk_free(&clk);
	return ret;
}

int snps_sdhci_init(struct mmc *mmc)
{
	struct sdhci_host *host = dev_get_priv(mmc->dev);
	struct snps_sdhci_plat *plat = dev_get_platdata(host->mmc->dev);
	int ret;
	host->voltages = MMC_VDD_33_34;
	ret = sdhci_probe(mmc->dev);
	if (ret)
		return -1;

	uint16_t val = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	if (host->voltages == MMC_VDD_165_195) {
		val |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, val, SDHCI_HOST_CONTROL2);
		sdhci_phy_1_8v_init(host);
	} else {
		val &= ~SDHCI_CTRL_VDD_180;
		if (plat->io_fixed_1v8)
			val |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, val, SDHCI_HOST_CONTROL2);
		sdhci_phy_3_3v_init(host);
	}

	return 0;
}

static int snps_sdhci_bind(struct udevice *dev)
{
	struct snps_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id snps_sdhci_ids[] = {
	{ .compatible = "snps,dwcmshc-sdhci" },
	{ }
};

U_BOOT_DRIVER(snps_sdhci_drv) = {
	.name           = "snps_sdhci",
	.id             = UCLASS_MMC,
	.of_match       = snps_sdhci_ids,
	.ops            = &sdhci_ops,
	.bind           = snps_sdhci_bind,
	.probe          = snps_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct snps_sdhci_plat),
};
