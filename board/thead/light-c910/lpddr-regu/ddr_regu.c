/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/types.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include "dw_iic_ll.h"
#include "iic.h"
#include "ddr_regu.h"

#define IOPMP_EMMC	0
#define IOPMP_SDIO0	1
#define IOPMP_SDIO1	2
#define IOPMP_USB0	3
#define IOPMP_AO	4
#define IOPMP_AUD	5
#define IOPMP_CHIP_DBG	6
#define IOPMP_EIP120I	7
#define IOPMP_EIP120II	8
#define IOPMP_EIP120III	9
#define IOPMP_ISP0	10
#define IOPMP_ISP1	11
#define IOPMP_DW200	12
#define IOPMP_VIPRE	13
#define IOPMP_VENC	14
#define IOPMP_VDEC	15
#define IOPMP_G2D	16
#define IOPMP_FCE	17
#define IOPMP_NPU	18
#define IOPMP0_DPU	19
#define IOPMP1_DPU	20
#define IOPMP_GPU	21
#define IOPMP_GMAC1	22
#define IOPMP_GMAC2	23
#define IOPMP_DMAC	24
#define IOPMP_TEE_DMAC	25
#define IOPMP_DSP0	26
#define IOPMP_DSP1	27

#define IIC_IDX_AONIIC  0
#define IIC_IDX_APIIC   1

#define DDR_VDD_REGU_0V6 0
#define DDR_VDD_REGU_0V8 1
#define DDR_VDD_REGU_1V1 2
#define LCD0_EN          3

#define APCPU_REGU_VDD   0
#define APCPU_REGU_VDD1  1
#define APCPU_REGU_VDDM  2

struct regulator_t {
	uint8_t  iic_id;                    ///< iic ctrl id
	uint8_t  regu_id;                   ///< regulator id
	uint8_t  dev_addr;                  ///< pmic-dev addr
	uint8_t  reg_alen;                  ///< lenght of addr, range in [1..2] uint in bytes
	uint16_t reg_addr;                  ///< reg_addr offset within pmic-dev
	uint16_t reg_addr1;                 ///< reg_addr1 offset within pmic-dev
	uint8_t  b_iic_read_sr;             ///< Sr timing between write and read
	uint8_t  rev[3];                    ///< reverse
        uint32_t regu_vol_target;           ///< target uv
	uint32_t regu_vol_min;              ///< min uv
	uint32_t regu_vol_max;              ///< max uv
	uint32_t regu_vol_step;             ///< step uv
};

#define REGU_ID_DEF(ctrl_id, regu_idx, devaddr, regaddr,regaddr1, addrlen, vol_target, vol_min, vol_max, vol_step, iic_read_Sr) \
	.iic_id=ctrl_id, \
	.regu_id=regu_idx, \
	.dev_addr=devaddr, \
	.reg_alen = addrlen, \
	.reg_addr = regaddr, \
	.reg_addr1 = regaddr1, \
	.regu_vol_target=vol_target, \
	.regu_vol_max=vol_max, \
	.regu_vol_min=vol_min, \
	.regu_vol_step=vol_step, \
	.b_iic_read_sr = iic_read_Sr

#if defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_A)
/**
 * board for EB068A10/EB065A10
 *
 */
static const struct regulator_t g_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V6,0x32,0x36,0,1,CONFIG_DDR_REGU_0V6,600000,3500000,12500,1),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V8,0x31,0x36,0,1,CONFIG_DDR_REGU_0V8,600000,3500000,12500,1),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_1V1,0x32,0x37,0,1,CONFIG_DDR_REGU_1V1,600000,3500000,12500,1),
	},
};

static const struct regulator_t g_apcpu_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDD,0x31,0x38,0,1,800000,600000,3500000,12500,1),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDDM,0x31,0x39,0,1,800000,600000,3500000,12500,1),
	},
};
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_B)
/**
 * board for EB068A10/EB065A10
 *
 */
static const struct regulator_t g_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V6,0x32,0x36,0,1,CONFIG_DDR_REGU_0V6,600000,3500000,12500,1),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V8,0x31,0x36,0,1,CONFIG_DDR_REGU_0V8,600000,3500000,12500,1),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_1V1,0x32,0x37,0,1,CONFIG_DDR_REGU_1V1,600000,3500000,12500,1),
	},
	/*lcd0-en*/
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,LCD0_EN,0x31,0x50,0,1,1800000,900000,3500000,25000,1),
	}
};

static const struct regulator_t g_apcpu_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDD,0x31,0x38,0,1,800000,600000,3500000,12500,1),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDDM,0x31,0x39,0,1,800000,600000,3500000,12500,1),
	},
};
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_A_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_REF) || (CONFIG_TARGET_LIGHT_FM_C910_BEAGLE)
/**
 * board for ant-ref
 *
 */
static const struct regulator_t g_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V6,0x5A,0xA5,0,1,CONFIG_DDR_REGU_0V6,530000,1200000,10000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V8,0x5A,0xA8,0,1,CONFIG_DDR_REGU_0V8,800000,1200000,20000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_1V1,0x5A,0xA6,0,1,CONFIG_DDR_REGU_1V1,800000,1500000,20000,0),
	},
};

static const struct regulator_t g_apcpu_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDD,0x5A,0xA4,0xB5,1,800000,300000,1500000,10000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDD1,0x5A,0xA3,0xB4,1,800000,300000,1500000,10000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDDM,0x5A,0xA7,0xB8,1,800000,800000,1500000,20000,0),
	}
};
#else
/**
 * board for EB064A10/EB064A11
 *
 */
static const struct regulator_t g_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V6,0x5A,0xA5,0,1,CONFIG_DDR_REGU_0V6,530000,1200000,10000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_0V8,0x5A,0xA8,0,1,CONFIG_DDR_REGU_0V8,800000,1200000,20000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_1V1,0x5A,0xA6,0,1,CONFIG_DDR_REGU_1V1,800000,1500000,20000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,DDR_VDD_REGU_1V1,0x5A,0xA7,0,1,CONFIG_DDR_REGU_1V1,800000,1500000,20000,0),
	}
};

static const struct regulator_t g_apcpu_regu_id_list[] = {
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDD,0x5A,0xA4,0xB5,1,800000,300000,1500000,10000,0),
	},
	{
		REGU_ID_DEF(IIC_IDX_AONIIC,APCPU_REGU_VDDM,0x5A,0xA3,0xB4,1,800000,300000,1500000,10000,0),
	},
};
#endif

#define HANDLE_REG_BASE(handle)     (handle->dev.reg_base)

static int wait_iic_transmit(dw_iic_regs_t *iic_base, uint32_t timeout)
{
    int ret = 0;

    do {
        uint32_t timecount = 0;

        while ((dw_iic_get_transmit_fifo_num(iic_base) != 0U) && (ret == 0)) {
		udelay(1000);
		timecount++;
            if (timecount >= timeout) {
                ret = -3;
            }
        }

    } while (0);

    return ret;
}

static int wait_iic_receive(dw_iic_regs_t *iic_base, uint32_t wait_data_num, uint32_t timeout)
{
    int ret = 0;

    do {
        uint32_t timecount = 0;

        while ((dw_iic_get_receive_fifo_num(iic_base) < wait_data_num) && (ret == 0)) {
		udelay(1000);
		timecount++;
            if (timecount >= timeout) {
                ret = -3;
            }
        }
    } while (0);

    return ret;
}


unsigned long soc_get_iic_freq(uint32_t idx)
{
	if (idx == IIC_IDX_AONIIC){
		return 49152000U;
	}else if (idx == IIC_IDX_APIIC ){
		return 50000000U  ;
	}else
		return 0U;
}

int target_get(uint32_t idx, csi_dev_t *dev)
{
	if(dev == NULL)
		return -1;

	switch(idx)
	{
		case IIC_IDX_AONIIC:

		dev->idx= idx;
		dev->reg_base = 0xfffff4c000;
		dev->irq_num = 79;
		break;

		case IIC_IDX_APIIC:

		dev->idx= idx;
		dev->reg_base = 0xffe7f20000;
		dev->irq_num = 44 ;
		break;

		default:
		return -1;
	};

	return 0;
}

static csi_iic_t g_iic_aoniic,g_iic_apiic;

csi_iic_t* pmic_get_iic_handle(uint32_t regu_id)
{
	switch(regu_id){
		case IIC_IDX_AONIIC:
			return (csi_iic_t*)&g_iic_aoniic;
		break;
		case IIC_IDX_APIIC:
			return (csi_iic_t*)&g_iic_apiic;
		break;
		default:
			return NULL;
	}

	return NULL;
}

int csi_iic_init(csi_iic_t *iic, uint32_t idx)
{
    int ret = 0;
    dw_iic_regs_t *iic_base;
    ret = target_get(idx, &iic->dev);

    if (ret == 0) {
        iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
        iic_base->IC_SAR = 0;
        dw_iic_disable(iic_base);
        dw_iic_clear_all_irq(iic_base);
        dw_iic_disable_all_irq(iic_base);
        iic_base->IC_FIFO_RST_EN = 0;
    }

    iic->state.writeable = 1U;
    iic->state.readable  = 1U;
    iic->state.error     = 0U;
    iic->send = NULL;
    iic->receive = NULL;
    iic->callback = NULL;

    return ret;

}


int csi_iic_mode(csi_iic_t *iic, csi_iic_mode_t mode)
{
    dw_iic_regs_t *iic_base;
    int ret = -1;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
    dw_iic_disable(iic_base);

    /* This register can be written only when the I2C is disabled */
    if (mode == IIC_MODE_MASTER) {
        iic->mode = IIC_MODE_MASTER;
        dw_iic_set_master_mode(iic_base);
        dw_iic_enable_restart(iic_base);
        ret = 0;
    } else if (mode == IIC_MODE_SLAVE) {
        iic->mode = IIC_MODE_SLAVE;
        dw_iic_set_slave_mode(iic_base);
        dw_iic_enable_restart(iic_base);
        ret = 0;
    }

    return ret;
}

int csi_iic_addr_mode(csi_iic_t *iic, csi_iic_addr_mode_t addr_mode)
{
    dw_iic_regs_t *iic_base;
    int ret = -1;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
    dw_iic_disable(iic_base);

    /* This register can be written only when the I2C is disabled */
    if (addr_mode == IIC_ADDRESS_7BIT) {
        dw_iic_set_master_7bit_addr_mode(iic_base);
        dw_iic_set_slave_7bit_addr_mode(iic_base);
        ret = 0;
    } else if (addr_mode == IIC_ADDRESS_10BIT) {
        dw_iic_set_master_10bit_addr_mode(iic_base);
        dw_iic_set_slave_10bit_addr_mode(iic_base);
        ret = 0;
    }

    return ret;
}

int csi_iic_speed(csi_iic_t *iic, csi_iic_speed_t speed)
{
    dw_iic_regs_t *iic_base;
    int ret = -1;
    iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
    uint32_t ic_clk;
    dw_iic_disable(iic_base);

    /* This register can be written only when the I2C is disabled */
    if (speed == IIC_BUS_SPEED_STANDARD) {
        dw_iic_set_transfer_speed_standard(iic_base);
        ic_clk = soc_get_iic_freq((uint32_t)iic->dev.idx) / 1000000U;
        dw_iic_set_standard_scl_hcnt(iic_base, ((5U * ic_clk) - 8U));
        dw_iic_set_standard_scl_lcnt(iic_base, ((5U * ic_clk) - 1U));
        ret = 0;
    } else if (speed == IIC_BUS_SPEED_FAST) {
        dw_iic_set_transfer_speed_fast(iic_base);
        ic_clk = soc_get_iic_freq((uint32_t)iic->dev.idx) / 1000000U;
        dw_iic_set_fast_scl_hcnt(iic_base, (((125U * ic_clk) / 100U) - 8U));
        dw_iic_set_fast_scl_lcnt(iic_base, (((125U * ic_clk) / 100U) - 1U));
        ret = 0;
    } else if (speed == IIC_BUS_SPEED_FAST_PLUS) {
        ret = -4;
    } else if (speed == IIC_BUS_SPEED_HIGH) {
        dw_iic_set_transfer_speed_high(iic_base);
        ic_clk = soc_get_iic_freq((uint32_t)iic->dev.idx) / 10000000U;
        dw_iic_set_high_scl_hcnt(iic_base, (((147U * ic_clk) / 100U) - 8U));
        dw_iic_set_high_scl_lcnt(iic_base, (((147U * ic_clk) / 100U) - 1U));
        ret = 0;
    }

    return ret;
}

int32_t csi_iic_master_send(csi_iic_t *iic, uint32_t devaddr, const void *data, uint32_t size, uint32_t timeout)
{
    int ret = 0;
    int32_t send_count = 0;
    uint32_t stop_time = 0U;
    uint8_t *send_data = (void *)data;
    uint32_t timecount = 0;

    if ((send_data == NULL) || (size == 0U)) {
        send_count = (int32_t)-1;
    } else {
        dw_iic_regs_t *iic_base;
        iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
	ret = dw_iic_wait_bus_idle(iic_base, timeout);
	if(ret)
		return ret;

        dw_iic_disable(iic_base);
        dw_iic_set_target_address(iic_base, devaddr);
        dw_iic_enable(iic_base);

        while (1) {
            if (size < IIC_MAX_FIFO) {
                for (uint8_t i = 0U; i < size; i++) {
                    dw_iic_transmit_data(iic_base, *(send_data++));
                }

                ret = wait_iic_transmit(iic_base, timeout);        ///< wait data send

                if (ret != 0) {
                    send_count = (int32_t)ret;
                    break;
                }

                send_count += (int32_t)size;

            } else {
                while (send_count < size) {
                    uint8_t send_num = IIC_MAX_FIFO - dw_iic_get_transmit_fifo_num(iic_base);

                    for (uint8_t i = 0U; i < send_num; i++) {
                        dw_iic_transmit_data(iic_base, *(send_data++));
                    }

                    send_count += send_num;

                    if (timeout >= timecount) {
                        ret = -7;
                        break;
                    }
                }

                if (ret != 0) {
                    break;
                }
            }

            if ((send_count == (int32_t)size) && (ret == 0)) {
		    stop_time = 0;
                while (!(dw_iic_get_raw_interrupt_state(iic_base) & DW_IIC_RAW_STOP_DET)) {
			udelay(1000);
			stop_time++;

			if (stop_time > timeout) {
				ret = -3;
				break;
			}
                }

                break;
            }
        }
    }

    return send_count;
}

int32_t csi_iic_master_receive(csi_iic_t *iic, uint32_t devaddr, void *data, uint32_t size, uint32_t timeout)
{
    int ret = 0;
    int32_t read_count = 0;
    uint8_t *receive_data = (void *)data;

    if ((receive_data == NULL) || (size == 0U)) {
        read_count = (int32_t)-1;
    } else {
        dw_iic_regs_t *iic_base;
        uint32_t cmd_num;
        iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
	ret = dw_iic_wait_bus_idle(iic_base, timeout);
	if(ret)
		return ret;
        dw_iic_disable(iic_base);
        dw_iic_set_target_address(iic_base, devaddr);
        dw_iic_enable(iic_base);

        if (size < IIC_MAX_FIFO) {
            for (read_count = 0; read_count < (int32_t)size; read_count++) {
                dw_iic_data_cmd(iic_base);
            }

            ret = wait_iic_receive(iic_base, size, timeout);

            if (ret == 0) {
                for (read_count = 0; read_count < (int32_t)size; read_count++) {
                    *(receive_data++) = dw_iic_receive_data(iic_base);
                }
            } else {
                read_count = (int32_t)ret;
            }

        } else {
            read_count = 0;

            for (cmd_num = size; cmd_num > (size - IIC_MAX_FIFO); cmd_num--) {
                dw_iic_data_cmd(iic_base);
            }

            while (read_count < size) {
                ret = wait_iic_receive(iic_base, 1U, timeout);

                if (ret != 0) {
                    read_count = (int32_t)ret;
                    break;
                }

                *(receive_data++) = dw_iic_receive_data(iic_base);
                read_count ++;

                if (cmd_num > 0U) {
                    dw_iic_data_cmd(iic_base);
                    cmd_num --;
                }
            }

            uint32_t timecount = 0;

            while (!(dw_iic_get_raw_interrupt_state(iic_base) & DW_IIC_RAW_STOP_DET)) {
		    udelay(1000);
		    timecount++;

                if (timecount >= timeout) {
                    ret = -3;
                    break;
                }
            }
        }
    }

    return read_count;
}

int32_t csi_iic_mem_receive_sr(csi_iic_t *iic, uint32_t devaddr, uint16_t memaddr, csi_iic_mem_addr_size_t memaddr_size, void *data, uint32_t size, uint32_t timeout)
{
    int ret = 0;
    int32_t read_count = 0;
    uint32_t stop_time = 0U;
    uint8_t *receive_data = (void *)data;
    uint8_t mem_addr_flag = 1U;

    if ((receive_data == NULL) || (size == 0U)) {
        read_count = (int32_t)-1;
    } else {

        dw_iic_regs_t *iic_base;
        iic_base = (dw_iic_regs_t *)HANDLE_REG_BASE(iic);
	ret = dw_iic_wait_bus_idle(iic_base, timeout);
	if(ret)
		return ret;

        dw_iic_disable(iic_base);
        dw_iic_set_target_address(iic_base, devaddr);
        dw_iic_enable(iic_base);

        if (memaddr_size == IIC_MEM_ADDR_SIZE_8BIT) {
            dw_iic_transmit_data(iic_base, (uint8_t)memaddr);
        }

        if (memaddr_size == IIC_MEM_ADDR_SIZE_16BIT) {
            dw_iic_transmit_data(iic_base, (uint8_t)(memaddr >> 8));
            dw_iic_transmit_data(iic_base, (uint8_t)memaddr);

        }

        uint32_t cmd_num;

        if (mem_addr_flag == 1U) {
            if (size < IIC_MAX_FIFO) {
                for (read_count = 0; read_count < (int32_t)size; read_count++) {
		    if(read_count == 0 )
			dw_iic_data_sr_cmd(iic_base);
		    else
			dw_iic_data_cmd(iic_base);
                }

                ret = wait_iic_receive(iic_base, size, timeout);

                if (ret == 0) {
                    for (read_count = 0; read_count < (int32_t)size; read_count++) {
                        *(receive_data++) = dw_iic_receive_data(iic_base);
                    }
                } else {
                    read_count = (int32_t)ret;
                }

            } else {
                read_count = 0;

                for (cmd_num = size; cmd_num > (size - IIC_MAX_FIFO); cmd_num--) {
		    if(cmd_num == size )
			dw_iic_data_sr_cmd(iic_base);
		    else
                        dw_iic_data_cmd(iic_base);
                }

                while (mem_addr_flag) {
                    ret = wait_iic_receive(iic_base, 1U, timeout);

                    if (ret != 0) {
                        read_count = (int32_t)ret;
                        break;
                    }

                    *(receive_data++) = dw_iic_receive_data(iic_base);
                    read_count ++;

                    if (read_count == (int32_t)size) {
                        while (!(dw_iic_get_raw_interrupt_state(iic_base) & DW_IIC_RAW_STOP_DET)) {
                            stop_time ++;

                            if (stop_time > DW_IIC_TIMEOUT_DEF_VAL) {
                                ret = -3;
                                break;
                            }
                        }

                        break;
                    }

                    if (cmd_num > 0U) {
                        dw_iic_data_cmd(iic_base);
                        cmd_num --;
                    }
                }
            }
        } else {
            read_count = (int32_t)-1;
        }

    }

    return read_count;
}

static int pmic_read_reg_sr(csi_iic_t *iic_handle,uint16_t dev_addr,uint32_t offset, uint32_t *val)
{
	int32_t num;
	uint8_t temp[2] = {0};
	num = csi_iic_mem_receive_sr(iic_handle,dev_addr,offset,(offset>>8)?IIC_MEM_ADDR_SIZE_16BIT:IIC_MEM_ADDR_SIZE_8BIT,&temp,1,10);
	if(num != 1) {
		return -1;
	}
	*val = temp[0];
	return 0;
}

static int pmic_write_reg(csi_iic_t *iic_handle,uint16_t dev_addr,uint32_t offset, uint32_t val)
{
	int32_t num;
	uint8_t temp[6] = {0};
	uint32_t len = 0;

	if((offset >> 8) & 0xff ){
		/*16 bit addr*/
		temp[len++] = (offset >> 8) & 0xff;
		temp[len++] = offset & 0xff;
		temp[len++]= (uint8_t)val;
	}else{
		/*8bit addr*/
		temp[len++] = offset & 0xff;
		temp[len++] = (uint8_t)val;
	}

	num = csi_iic_master_send(iic_handle,dev_addr,temp,len,10);
	if(num != len) {
		return -1;
	}

	return 0;
}

static int pmic_read_reg(csi_iic_t *iic_handle,uint16_t dev_addr,uint32_t offset, uint32_t *val)
{
	int32_t num;
	uint8_t temp[2] = {0};
	uint32_t len = 0;

	if ((offset >> 8) & 0xff)
	{
		temp[len++] = (offset >> 8) & 0xff;
		temp[len++] = offset & 0xff;
	}
	else
	{
		temp[len++] = offset & 0xff;
	}

	num = csi_iic_master_send(iic_handle,dev_addr,&temp,len,10);
	if(num != len) {
		return -1;
	}
	num = csi_iic_master_receive(iic_handle,dev_addr,&temp[0],1,10);
	if(num != 1) {
		return -1;
	}

	*val = temp[0];
	return 0;
}

static int _pmic_ddr_regu_init(uint32_t idx)
{
	int ret;


	csi_iic_t *handle = pmic_get_iic_handle(idx);

	if ( handle == NULL)
		return -1;

	ret = csi_iic_init(handle,idx);
	if (ret)
		return ret;
	ret = csi_iic_mode(handle, IIC_MODE_MASTER);
	if( ret )
		return ret;
	ret = csi_iic_addr_mode(handle,IIC_ADDRESS_7BIT);
	if( ret )
		return ret;
	ret = csi_iic_speed(handle,IIC_BUS_SPEED_FAST);
	if(ret)
		return ret;
	return ret;
}

static const struct light_iopmp_list {
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
};

static void light_iopmp_config(void)
{
	int i = 0;
	int entry_size;
	entry_size = ARRAY_SIZE(light_iopmp_lists);

	while (i < entry_size) {
		writel(0xffffffff, (void *)(light_iopmp_lists[i].reg) + 0xc0);
		i++;
	}
}

int pmic_ddr_regu_init(void)
{
#define AON_PADMUX_BASE (0xfffff4a000)
	int ret;
	uint32_t val;
	/*init iopmp */
	light_iopmp_config();
	/*init aon-iic pad*/
	val = readl((void *)(AON_PADMUX_BASE + 0x10));
	val &=~0xffff0000;
	val |=0x208<<16;
	writel( val, (void *)(AON_PADMUX_BASE + 0x10));
	val = readl((void *)(AON_PADMUX_BASE + 0x14));
	val &=~0xffff;
	val |=0x208;
	writel( val, (void *)(AON_PADMUX_BASE + 0x14));

	/*init iic device*/
	ret = _pmic_ddr_regu_init(IIC_IDX_AONIIC);
	if(ret)
		return ret;
	ret = _pmic_ddr_regu_init(IIC_IDX_APIIC);
	if(ret)
		return ret;
	return ret;
}

int pmic_ddr_set_voltage(void)
{
	int ret = 0;
	uint32_t val = 0;
	uint32_t regu_num = ARRAY_SIZE(g_regu_id_list);
	uint32_t i;
	struct regulator_t *pregu;
	csi_iic_t          *dev_handle;

#if 0 //currently,no need to modify ddr regulator voltage
	pregu = (struct regulator_t*)g_regu_id_list;
	for (i = 0; i < regu_num; i++, pregu++) {
		if (pregu->regu_vol_target < pregu->regu_vol_min || pregu->regu_vol_target > pregu->regu_vol_max)
			continue;
		val = (pregu->regu_vol_target - pregu->regu_vol_min)/pregu->regu_vol_step;
		dev_handle = pmic_get_iic_handle(pregu->iic_id);
		ret = pmic_write_reg(dev_handle,pregu->dev_addr, pregu->reg_addr,val);
		if (ret)
			return ret;
	}
#endif

#if defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_B)
	/*enable lcd0_en ldo*/
	pregu = (struct regulator_t*)&g_regu_id_list[LCD0_EN];
	dev_handle = pmic_get_iic_handle(pregu->iic_id);
	ret = pmic_read_reg_sr(dev_handle,pregu->dev_addr,0x44,&val);
	if(ret)
		return ret;
	val |=(1<<4);
	ret = pmic_write_reg(dev_handle,pregu->dev_addr,0x44,val);
#endif
	return ret;
}

struct regulator_t* pmic_get_regu_by_reguid(const struct regulator_t *regu_dsc_list,uint32_t regu_dsc_list_size, uint32_t regu_id)
{
	uint32_t i;
	struct regulator_t *pregu = NULL;

	if(regu_dsc_list == NULL)
		return NULL;
	for(i = 0; i < regu_dsc_list_size; i++) {
		if (regu_dsc_list[i].regu_id == regu_id) {
			pregu = (struct regulator_t*)&regu_dsc_list[i];
			break;
		}
	}

	return pregu;
}

#if defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_A) ||defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_B)
int pmic_reset_apcpu_voltage(void)
{
	csi_iic_t          *dev_handle;
	struct regulator_t *regu_vdd,*regu_vddm;
	uint32_t cur_target1,cur_target2;
	uint32_t target_uv1,target_uv2,val;
	int ret;
	int32_t  nstep1,nstep2;

	regu_vdd = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDD);
	regu_vddm = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDDM);
	dev_handle = pmic_get_iic_handle(regu_vdd->iic_id);
	if (!dev_handle || !regu_vdd || !regu_vddm)
		return -1;

	/*get cur_target uv*/
	ret = pmic_read_reg_sr(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,&cur_target1);
	if(ret)
		return ret;

	ret = pmic_read_reg_sr(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,&cur_target2);
	if(ret)
		return ret;

	target_uv1 = (regu_vdd->regu_vol_target-regu_vdd->regu_vol_min)/regu_vdd->regu_vol_step;
	target_uv2 = (regu_vddm->regu_vol_target-regu_vdd->regu_vol_min)/regu_vddm->regu_vol_step;

	/*calculate nsteps for each regu_id*/
	nstep1 = (target_uv1-cur_target1);
	nstep2 = (target_uv2-cur_target2);

	/*do adjusting voltage step by step */
	while(nstep1 || nstep2) {

		if(nstep1 > 0 ) {
			cur_target1 +=1;
			ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,cur_target1);
			if (ret )
				return ret;
			nstep1--;
		}else if (nstep1 < 0 ){
			cur_target1 -=1;
			ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,cur_target1);
			if (ret )
				return ret;
			nstep1++;
		}

		if(nstep2 > 0 ) {
			cur_target2 +=1;
			ret = pmic_write_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,cur_target2);
			if (ret )
				return ret;
			nstep2--;
		}else if (nstep2 < 0 ){
			cur_target2 -=1;
			ret = pmic_write_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,cur_target2);
			if (ret )
				return ret;
			nstep2++;
		}

	}

	mdelay(2);

	/*disable watchdog*/
	ret = pmic_read_reg_sr(dev_handle,0x31,0x0B,&val);
	if (ret)
		return ret;
	val &=~(1<<2);
	ret = pmic_write_reg(dev_handle,0x31,0x0B,val);
	if (ret)
		return ret;
	return 0;
}
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_A_REF) || defined (CONFIG_TARGET_LIGHT_FM_C910_B_REF)|| (CONFIG_TARGET_LIGHT_FM_C910_BEAGLE)
int pmic_reset_apcpu_voltage(void)
{
	int                ret = -1;
	csi_iic_t          *dev_handle;
	struct regulator_t *regu_vdd,*regu_vdd1,*regu_vddm;
	uint32_t           val,dvc,dvc1;

	regu_vdd = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDD);
	regu_vdd1 = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDD1);
	regu_vddm = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDDM);
	dev_handle = pmic_get_iic_handle(regu_vdd->iic_id);
	if (!dev_handle || !regu_vdd || !regu_vdd1 || !regu_vddm)
		return -1;
	/* get current regulator rail */
	ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,0x32,&dvc);
	if (ret)
		return ret;
	ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,0x33,&dvc1);
	if (ret)
		return ret;
	/*bcore1 and bcore2 should use the same rail(both rail-a or both rail-b */
	if (!((dvc & 0x3) == 0 || (dvc & 0x3) == 3) )
		return -1;
	/* cur-rail is rail-a, then copy current voltage to rail-b*/
	if ( (dvc & 0x3) == 0 ) {
		ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,&val);
		if (ret)
			return ret;
		ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr1,val);
		if (ret)
			return ret;
		ret = pmic_read_reg(dev_handle,regu_vdd1->dev_addr,regu_vdd1->reg_addr,&val);
		if (ret)
			return ret;
		ret = pmic_write_reg(dev_handle,regu_vdd1->dev_addr,regu_vdd1->reg_addr1,val);
		if (ret)
			return ret;
		ret = pmic_read_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,&val);
		if (ret)
			return ret;
		ret = pmic_write_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr1,val);
		if (ret)
			return ret;
		/*select rail-b*/
		dvc |=0x3;
		ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x32,dvc);
		if (ret)
			return ret;
		dvc1 |=0x1;
		ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x33,dvc1);
		if (ret)
			return ret;
		mdelay(1);
	}

	/*modify rail-a voltate, then select rail-a */
	val = (regu_vdd->regu_vol_target - regu_vdd->regu_vol_min)/regu_vdd->regu_vol_step;
	ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,val);
	if (ret)
		return ret;

	val = (regu_vdd1->regu_vol_target - regu_vdd1->regu_vol_min)/regu_vdd1->regu_vol_step;
	ret = pmic_write_reg(dev_handle,regu_vdd1->dev_addr,regu_vdd1->reg_addr,val);
	if (ret)
		return ret;

	val = (regu_vddm->regu_vol_target - regu_vddm->regu_vol_min)/regu_vddm->regu_vol_step;
	ret = pmic_write_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,val);
	if (ret)
		return ret;
	/*select rail-a*/
	dvc &=~0x3;
	ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x32,dvc);
	if (ret)
		return ret;
	dvc1 &=~0x1;
	ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x33,dvc1);
	if (ret)
		return ret;
	mdelay(1);

	/*disable watchdog*/
	ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,0x11,&val);
	if (ret)
		return ret;
	val &=~0x7;
	pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x11,val);
	return 0;
}
#else
int pmic_reset_apcpu_voltage(void)
{
	int                ret = -1;
	csi_iic_t          *dev_handle;
	struct regulator_t *regu_vdd,*regu_vddm;
	uint32_t           val,dvc;

	regu_vdd = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDD);
	regu_vddm = pmic_get_regu_by_reguid(g_apcpu_regu_id_list, ARRAY_SIZE(g_apcpu_regu_id_list), APCPU_REGU_VDDM);
	dev_handle = pmic_get_iic_handle(regu_vdd->iic_id);
	if (!dev_handle || !regu_vdd || !regu_vddm)
		return -1;
	/* get current regulator rail */
	ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,0x32,&dvc);
	if (ret)
		return ret;
	/*bcore1 and bcore2 should use the same rail(both rail-a or both rail-b */
	if (!((dvc & 0x3) == 0 || (dvc & 0x3) == 3) )
		return -1;
	/* cur-rail is rail-a, then copy current voltage to rail-b*/
	if ( (dvc & 0x3) == 0 ) {
		ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,&val);
		if (ret)
			return ret;
		ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr1,val);
		if (ret)
			return ret;
		ret = pmic_read_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,&val);
		if (ret)
			return ret;
		ret = pmic_write_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr1,val);
		if (ret)
			return ret;
		/*select rail-b*/
		dvc |=0x3;
		ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x32,dvc);
		if (ret)
			return ret;
		mdelay(1);
	}

	/*modify rail-a voltate, then select rail-a */
	val = (regu_vdd->regu_vol_target - regu_vdd->regu_vol_min)/regu_vdd->regu_vol_step;
	ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,regu_vdd->reg_addr,val);
	if (ret)
		return ret;

	val = (regu_vddm->regu_vol_target - regu_vddm->regu_vol_min)/regu_vddm->regu_vol_step;
	ret = pmic_write_reg(dev_handle,regu_vddm->dev_addr,regu_vddm->reg_addr,val);
	if (ret)
		return ret;
	/*select rail-a*/
	dvc &=~0x3;
	ret = pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x32,dvc);
	if (ret)
		return ret;
	mdelay(1);

	/*disable watchdog*/
	ret = pmic_read_reg(dev_handle,regu_vdd->dev_addr,0x11,&val);
	if (ret)
		return ret;
	val &=~0x7;
	pmic_write_reg(dev_handle,regu_vdd->dev_addr,0x11,val);
	return 0;
}
#endif
