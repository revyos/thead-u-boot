/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     soc.h
 * @brief    CSI Core Peripheral Access Layer Header File for
 *           CSKYSOC Device Series
 * @version  V1.0
 * @date     7. April 2020
 ******************************************************************************/

#ifndef _SOC_H_
#define _SOC_H_

#include <stdint.h>
#include "csi_core.h"
#include "sys_clk.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef EHS_VALUE
#define EHS_VALUE               20000000U
#endif

#ifndef ELS_VALUE
#define ELS_VALUE               32768U
#endif

#ifndef IHS_VALUE
#define IHS_VALUE               50000000U
#endif

#ifndef ILS_VALUE
#define ILS_VALUE               32768U
#endif

#define RISCV_CORE_TIM_FREQ 3000000

typedef enum {
    Supervisor_Software_IRQn        =  1U,
    Machine_Software_IRQn           =  3U,
    Supervisor_Timer_IRQn           =  5U,
    CORET_IRQn                      =  7U,
    Supervisor_External_IRQn        =  9U,
    Machine_External_IRQn           =  11U,
    DW_TIMER0_IRQn                  =  16U,
    DW_TIMER1_IRQn                  =  17U,
    DW_TIMER2_IRQn                  =  18U,
    DW_TIMER3_IRQn                  =  19U,
    DW_TIMER4_IRQn                  =  20U,
    DW_TIMER5_IRQn                  =  21U,
    DW_TIMER6_IRQn                  =  22U,
    DW_TIMER7_IRQn                  =  23U,
    WJ_MBOX_IRQn                    =  28U,
    DW_UART0_IRQn                   =  36U,
    DW_UART1_IRQn                   =  37U,
    DW_UART2_IRQn                   =  38U,
    DW_UART3_IRQn                   =  39U,
    DW_UART4_IRQn                   =  40U,
    DW_UART5_IRQn                   =  41U,
    DW_I2C0_IRQn                    =  44U,
    DW_I2C2_IRQn                    =  46U,
    DW_QSPI0_IRQn                   =  52U,
    DW_QSPI1_IRQn                   =  53U,
    DW_SPI0_IRQn                    =  54U,
    DW_GPIO0_IRQn                   =  56U,
    DW_GPIO1_IRQn                   =  57U,
    DW_GPIO2_IRQn                   =  58U,
    DW_GPIO3_IRQn                   =  59U,
    DW_EMMC_IRQn                    =  62U,
    DW_SD_IRQn                      =  64U,
    DW_USB_IRQn                     =  68U,
    DW_DMA0_IRQn                    =  27U,
    DCD_ISO7816_IRQn                =  69U,
    DW_DMA1_IRQn                    =  71U,
    DW_DMA2_IRQn                    =  72U,
    DW_DMA3_IRQn                    =  73U,
	WJ_EFUSE_IRQn                   =  80U,
    DW_WDT0_IRQn                    =  111U,
    DW_WDT1_IRQn                    =  112U,
    RB_120SI_AV_IRQn                =  121U,
    RB_120SII_AV_IRQn               =  124U,
    RB_120SIII_AV_IRQn              =  127U,
    RB_150B_AIC_IRQn                =  128U,
    RB_150B_PKA1_IRQn               =  130U,
    RB_150B_ERR_IRQn                =  132U,
    RB_150B_TRNG_IRQn               =  133U,
} irqn_type_t;

typedef enum {
    WJ_IOCTL_Wakeupn               =  29U,     /* IOCTOL wakeup */
} wakeupn_type_t;

typedef enum {
    WJ_USB_CLK_MANAGERN            = 28U,
} clk_manager_type_t;

typedef enum {
	PAD_GRP_BASE1,
	PAD_UART0_TXD = PAD_GRP_BASE1,
	PAD_UART0_RXD,
	PAD_QSPI0_SCLK,
	PAD_QSPI0_CSN0,
	PAD_QSPI0_CSN1,
	PAD_QSPI0_D0_MOSI,
	PAD_QSPI0_D1_MISO,
	PAD_QSPI0_D2_WP,
	PAD_QSPI0_D3_HOLD,
	PAD_I2C2_SCL,
	PAD_I2C2_SDA,
	PAD_I2C3_SCL,
	PAD_I2C3_SDA,
	PAD_GPIO2_13,
	PAD_SPI_SCLK,
	PAD_SPI_CSN,
	PAD_SPI_MOSI,
	PAD_SPI_MISO,
	PAD_GPIO2_18,
	PAD_GPIO2_19,
	PAD_GPIO2_20,
	PAD_GPIO2_21,
	PAD_GPIO2_22,
	PAD_GPIO2_23,
	PAD_GPIO2_24,
	PAD_GPIO2_25,
	PAD_SDIO0_WPRTN,
	PAD_SDIO0_DETN,
	PAD_SDIO1_WPRTN,
	PAD_SDIO1_DETN,
	PAD_GPIO2_30,
	PAD_GPIO2_31,
	PAD_GPIO3_0,
	PAD_GPIO3_1,
	PAD_GPIO3_2,
	PAD_GPIO3_3,
	PAD_HDMI_SCL,
	PAD_HDMI_SDA,
	PAD_HDMI_CEC,
	PAD_GMAC0_TX_CLK,
	PAD_GMAC0_RX_CLK,
	PAD_GMAC0_TXEN,
	PAD_GMAC0_TXD0,
	PAD_GMAC0_TXD1,
	PAD_GMAC0_TXD2,
	PAD_GMAC0_TXD3,
	PAD_GMAC0_RXDV,
	PAD_GMAC0_RXD0,
	PAD_GMAC0_RXD1,
	PAD_GMAC0_RXD2,
	PAD_GMAC0_RXD3,
	PAD_GMAC0_MDC,
	PAD_GMAC0_MDIO,
	PAD_GMAC0_COL,
	PAD_GMAC0_CRS,

	PAD_GRP_BASE2,
	PAD_QSPI1_SCLK = PAD_GRP_BASE2,
	PAD_QSPI1_CSN0,
	PAD_QSPI1_D0_MOSI,
	PAD_QSPI1_D1_MISO,
	PAD_QSPI1_D2_WP,
	PAD_QSPI1_D3_HOLD,
	PAD_I2C0_SCL,
	PAD_I2C0_SDA,
	PAD_I2C1_SCL,
	PAD_I2C1_SDA,
	PAD_UART1_TXD,
	PAD_UART1_RXD,
	PAD_UART4_TXD,
	PAD_UART4_RXD,
	PAD_UART4_CTSN,
	PAD_UART4_RTSN,
	PAD_UART3_TXD,
	PAD_UART3_RXD,
	PAD_GPIO0_18,
	PAD_GPIO0_19,
	PAD_GPIO0_20,
	PAD_GPIO0_21,
	PAD_GPIO0_22,
	PAD_GPIO0_23,
	PAD_GPIO0_24,
	PAD_GPIO0_25,
	PAD_GPIO0_26,
	PAD_GPIO0_27,
	PAD_GPIO0_28,
	PAD_GPIO0_29,
	PAD_GPIO0_30,
	PAD_GPIO0_31,
	PAD_GPIO1_0,
	PAD_GPIO1_1,
	PAD_GPIO1_2,
	PAD_GPIO1_3,
	PAD_GPIO1_4,
	PAD_GPIO1_5,
	PAD_GPIO1_6,
	PAD_GPIO1_7,
	PAD_GPIO1_8,
	PAD_GPIO1_9,
	PAD_GPIO1_10,
	PAD_GPIO1_11,
	PAD_GPIO1_12,
	PAD_GPIO1_13,
	PAD_GPIO1_14,
	PAD_GPIO1_15,
	PAD_GPIO1_16,
	PAD_CLK_OUT_0,
	PAD_CLK_OUT_1,
	PAD_CLK_OUT_2,
	PAD_CLK_OUT_3,
	PAD_GPIO1_21,
	PAD_GPIO1_22,
	PAD_GPIO1_23,
	PAD_GPIO1_24,
	PAD_GPIO1_25,
	PAD_GPIO1_26,
	PAD_GPIO1_27,
	PAD_GPIO1_28,
	PAD_GPIO1_29,
	PAD_GPIO1_30,
} pin_name_t;


typedef enum {
	PAD_UART0_TXD_ALT_TXD       =0,
	PAD_UART0_TXD_ALT_GPIO2_0   =3,
	PAD_UART0_RXD_ALT_RXD       =0,
	PAD_UART0_RXD_ALT_GPIO2_1   =3,
    PAD_QSPI0_SCLK_ALT_QSPI0_SCK= 0,
    PAD_QSPI0_SCLK_ALT_PWM0     = 1,
    PAD_QSPI0_SCLK_ALT_I2S_SDA0 = 2,
    PAD_QSPI0_SCLK_ALT_GPIO2_2  = 3,
    PAD_QSPI0_CSN0_ALT_QSPI0_CSN0=0,
    PAD_QSPI0_CSN0_ALT_PWM1      =1,
    PAD_QSPI0_CSN0_ALT_I2S_SDA1  =2,
    PAD_QSPI0_CSN0_ALT_GPIO2_3   =3,
    PAD_QSPI0_CSN1_ALT_QSPI0_CSN1=0,
    PAD_QSPI0_CSN1_ALT_PWM2      =1,
    PAD_QSPI0_CSN1_ALT_I2S_SDA2  =2,
    PAD_QSPI0_CSN1_ALT_GPIO2_4   =3,
    PAD_QSPI0_D0_MOSI_ALT_QSPI0_MOSI=0,
    PAD_QSPI0_D0_MOSI_ALT_PWM3      =1,
    PAD_QSPI0_D0_MOSI_ALT_I2S_SDA3  =2,
    PAD_QSPI0_D0_MOSI_ALT_GPIO2_5   =3,
    PAD_QSPI0_D1_MISO_ALT_QSPI0_MISO=0,
    PAD_QSPI0_D1_MISO_ALT_QSPI0_PWM4=1,
    PAD_QSPI0_D1_MISO_ALT_I2S_MCLK  =2,
    PAD_QSPI0_D1_MISO_ALT_GPIO2_6   =3,
    PAD_QSPI0_D2_WP_ALT_QSPI0_WP    =0,
    PAD_QSPI0_D2_WP_ALT_PWM5        =1,
    PAD_QSPI0_D2_WP_ALT_I2S_SCK     =2,
    PAD_QSPI0_D2_WP_ALT_GIOP2_7     =3,
    PAD_QSPI0_D3_HOLD_ALT_QSPI0_HOLD=0,
    PAD_QSPI0_D3_HOLD_ALT_I2S_WS    =2,
    PAD_QSPI0_D3_HOLD_ALT_GPIO2_8   =3,

	PAD_UART1_TXD_ALT_TXD           =0,
	PAD_UART1_TXD_ALT_GPIO0_10      =3,
	PAD_UART1_RXD_ALT_RXD           =0,
	PAD_UART1_RXD_ALT_GPIO011       =3,

    PIN_FUNC_GPIO                   =  3U,
} pin_func_t;

#define CONFIG_GPIO_NUM             3
#define CONFIG_IRQ_NUM              112
#define CONFIG_DMA_NUM              1

#define WJ_EFUSE_BASE               0xFFFF210000UL
#define WJ_EFUSE_SIZE               0x10000U

#define DW_USB_BASE                 0xFFE7040000UL
#define DW_USB_SIZE                 0x10000U

#define DW_TIMER0_BASE              0xFFEFC32000UL
#define DW_TIMER0_SIZE              0x14U

#define DW_TIMER1_BASE              (DW_TIMER0_BASE+DW_TIMER0_SIZE)
#define DW_TIMER1_SIZE              DW_TIMER0_SIZE

#define DW_TIMER2_BASE              0xFFFFC33000UL
#define DW_TIMER2_SIZE              DW_TIMER1_SIZE

#define DW_TIMER3_BASE              (DW_TIMER2_BASE+DW_TIMER2_SIZE)
#define DW_TIMER3_SIZE              DW_TIMER2_SIZE

#define DW_UART0_BASE               0xFFE7014000UL
#define DW_UART0_SIZE               0x4000U

#define DW_UART1_BASE               0xFFE7F00000UL
#define DW_UART1_SIZE               0x4000U

#define DW_UART2_BASE               0xFFEC010000UL
#define DW_UART2_SIZE               0x4000U

#define DW_UART3_BASE               0xFFE7F04000UL
#define DW_UART3_SIZE               0x4000U

#define DW_UART4_BASE               0xFFF7F08000UL
#define DW_UART4_SIZE               0x4000U

#define DW_UART5_BASE               0xFFF7F0C000UL
#define DW_UART5_SIZE               0x4000U

#define DW_GPIO0_BASE               0xFFEC005000UL
#define DW_GPIO0_SIZE               0x1000U

#define DW_GPIO1_BASE               0xFFEC006000UL
#define DW_GPIO1_SIZE               0x1000U

#define DW_GPIO2_BASE               0xFFE7F34000UL
#define DW_GPIO2_SIZE               0x4000U

#define DW_GPIO3_BASE               0xFFE7F38000UL
#define DW_GPIO3_SIZE               0x4000U

#define DW_WDT_BASE                 0xFFEFC30000UL
#define DW_WDT_BASE_SZIE            0x1000U

#define DW_DMA_BASE                 0xFFEFC00000UL
#define DW_DMA_BASE_SZIE            0x4000U

#define WJ_IOC_BASE1                0xFFEC007000UL
#define WJ_IOC_SIZE                 0x1000U

#define WJ_IOC_BASE2                0xFFE7F3C000UL
#define WJ_IOC_SIZE                 0x1000U

#define WJ_CPR_BASE                 0xFFCB000000UL
#define WJ_CPR_BASE_SIZE            0x1000000U

#define DW_SPI0_BASE                0xFFF700C000UL
#define DW_SPI0_BASE_SIZE           0x10000U

#define DW_QSPI0_BASE              0xFFEA000000UL
#define DW_QSPI0_BASE_SIZE         0x10000U

#define DW_QSPI1_BASE              0xFFE8000000UL
#define DW_QSPI1_BASE_SIZE         0x10000U

#define DW_I2C0_BASE               0xFFE701C000UL
#define DW_I2C0_BASE_SIZE          0x4000U

#define DW_I2C1_BASE               0xFFE7F24000UL
#define DW_I2C1_BASE_SIZE          0x4000U

#define DW_I2C2_BASE               0xFFEC00C000UL
#define DW_I2C2_BASE_SIZE          0x4000U

#define DW_I2C3_BASE               0xFFFC010000UL
#define DW_I2C3_BASE_SIZE          0x4000U

#define DW_I2C4_BASE               0xFFE7F28000UL
#define DW_I2C4_BASE_SIZE          0x4000U

#define DW_I2C5_BASE               0xFFE7F2C000UL
#define DW_I2C5_BASE_SIZE          0x4000U

#define WJ_MBOX_BASE               0xFFFFC38000UL
#define WJ_MBOX_SIZE               0x1000U

#define WJ_MBOX1_BASE              0xFFFFC48000UL
#define WJ_MBOX1_SIZE              0x1000U

#define DW_EMMC_BASE               0xFFE7080000UL
#define DW_EMMC_SIZE               0x1000U

#define DW_SD_BASE                 0xFFE7090000UL
#define DW_SD_SIZE                 0x1000U

#define DCD_ISO7816_BASE           0xFFF7F30000ULL
#define DCD_ISO7816_SIZE           0x4000UL

#define RB_RNG_BASE                0xFFFF300000UL
#define RB_RNG_SIZE                0x10000U

#define RB_EIP150B_BASE            0xFFFF300000UL
#define RB_EIP150B_SIZE            0x10000U


#define RB_EIP28_BASE              (RB_EIP150B_BASE + 0x4000)
#define RB_EIP28_SIZE              0x3FFCU

#define RB_EIP120SI_BASE           0xFFFF310000UL
#define RB_EIP120SI_SIZE           0x10000U

#define RB_EIP120SII_BASE          0xFFFF320000UL
#define RB_EIP120SII_SIZE          0x10000U

#define RB_EIP120SIII_BASE         0xFFFF330000UL
#define RB_EIP120SIII_SIZE         0x10000U

#define TEE_SYS_BASE               0xFFFF200000UL
#define TEE_SYS_SIZE               0x10000U

#define PLIC_BASE                  0xFFD8000000ULL

#define WJ_AON_SYSRST_GEN_BASE     0xFFFFF44000UL
#define WJ_AON_SYSRST_GEN_SIZE     0x2000U
#define KEYRAM_BASE                0xFFFF260000UL
#define KEYRAM_SIZE                0x10000U

#define TEE_SYS_BASE               0xFFFF200000UL
#define TEE_SYS_SIZE               0x10000U
#define TEE_SYS_EFUSE_LC_PRELD_OFF 0x64
#define TEE_SYS_EFUSE_LC_READ_OFF  0x68
#define TEE_SYS_EFUSE_DBG_KEY1_OFF 0x70

#define IOPMP_EIP120I_BASE         0xFFFF220000UL
#define IOPMP_EIP120I_SIZE         0x10000
#define IOPMP_EIP120II_BASE        0xFFFF230000UL
#define IOPMP_EIP120II_SIZE        0x10000
#define IOPMP_EIP120III_BASE       0xFFFF240000UL
#define IOPMP_EIP120III_SIZE       0x10000
#define IOPMP_TEE_DMAC_BASE        0xFFFF250000UL
#define IOPMP_TEE_DMAC_SIZE        0x10000

#define IOPMP_EMMC_BASE            0xFFFC028000UL
#define IOPMP_EMMC_SIZE            0x1000
#define IOPMP_SDIO0_BASE           0xFFFC029000UL
#define IOPMP_SDIO0_SIZE           0x1000
#define IOPMP_SDIO1_BASE           0xFFFC02a000UL
#define IOPMP_SDIO1_SIZE            0x1000


#define CONFIG_MAILBOX_CHANNEL_NUM  4U

#define CONFIG_RTC_FAMILY_D

#define CONFIG_DW_AXI_DMA_8CH_NUM_CHANNELS
#define SOC_OM_ADDRBASE             0xFFEF018010
#define SOC_OSC_BOOT_ADDRBASE       0xFFEF010314
#define SOC_INTERNAL_SRAM_BASEADDR  0xFFE0000000
#define SOC_INTERNAL_SRAM_SIZE      (1536 * 1024)   //1.5MB
#define SOC_BROM_BASE_ADDRESS       0xFFFFD00000

#define CONFIG_OTP_BASE_ADDR        0   // FIXME:
#define CONFIG_OTP_BANK_SIZE        (8 * 1024)

#define AO_SYS_REG_BASE             0xFFFFF48000UL
#define AO_SYS_REG_SIZE             0x2000U

#define SPIFLASH_BASE               (0x18000000UL)

#define bootsel() \
    ({ unsigned int __v = (*(volatile uint32_t *) (0xFFEF018010)); __v&0x7; })

#define osc_bootsel() \
    ({ unsigned int __v = (*(volatile uint32_t *) (0xFFEF010314)); __v&0x1; })

#define FULLMASK_APTEECLK_ADDRBASE	0xFFFF011000
#define FULLMASK_TEE_PLL_CFG0_OFF	0x60
#define FULLMASK_TEE_PLL_CFG1_OFF	0x64
#define FULLMASK_TEE_PLL_CFG3_OFF	0x6c
#define FULLMASK_PLL_STS_OFF		0x80
#define FULLMASK_TEESYS_CLK_TEECFG_OFF	0x1cc
#define FULLMASK_TEESYS_HCLK_SWITCH_SEL	(0x2000U)
#define FULLMASK_PLL_STS_TEE_PLL_LOCK	(0x400U)
#define FULLMASK_TEE_PLL_LOCK_TIMEOUT	(0x3U) //unit: 10us
#define FULLMASK_TEE_PLL_CFG3_CALLOCK_CNT_EN	(0x400)
#define FULLMASK_TEE_PLL_CFG3_DSKEWCAL_PULSE	(0x200)
#define FULLMASK_TEE_PLL_CFG3_DSKEWCAL_SWEN	(0x100)
#define FULLMASK_TEE_PLL_CFG3_DSKEWCAL_RDY	(0x80)
#define FULLMASK_TEE_PLL_DSKEWCAL_RDY_TIMEOUT	(200U) //unit: 10us
#define FULLMASK_TEE_PLL_CFG1_PWR_DOWN		0x21000000
#define FULLMASK_TEE_PLL_CFG1_PWR_ON		0x01000000
#define FULLMASK_TEE_PLL_CFG0_792M		0x01306301

#define FULLMASK_AONSYSREG_ADDRBASE		0xFFFFF48000
#define FULLMASK_AONSYSREG_PLL_DSKEW_LOCK_OFF	0x22c
#define FULLMASK_AONSYSREG_PLL_DSKEW_BYPASS	(0x2U)
#define FULLMASK_AONSYSREG_RC_READY_OFF		0x7c
#define FULLMASK_AONSYSREG_RC_READY		(0x1U)
#define FULLMASK_RC_READY_TIMEOUT		(2U)  //unit: 10us

#define FULLMASK_AONSYSREG_RC_OFF	    0x74
#define FULLMASK_AONSYSREG_RC_VAL_POS   0
#define FULLMASK_AONSYSREG_RC_VAL_MSK   0xFFF
#ifdef __cplusplus
}
#endif

#endif  /* _SOC_H_ */
