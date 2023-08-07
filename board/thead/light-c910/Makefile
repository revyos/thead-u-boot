#
# Copyright (C) 2017-2020 Alibaba Group Holding Limited
#
# SPDX-License-Identifier: GPL-2.0+
#
DDR_SRC_PATH=lpddr4/src
DDR_REGU_SRC=lpddr-regu
DDR_FW_PATH=$(DDR_SRC_PATH)/ddr_phy_fw

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += sys_clk.o

ifdef CONFIG_PMIC_VOL_INIT
obj-$(CONFIG_PMIC_VOL_INIT) += $(DDR_REGU_SRC)/ddr_regu.o
obj-$(CONFIG_PMIC_VOL_INIT) += $(DDR_REGU_SRC)/dw_iic_ll.o
endif

ifdef CONFIG_TARGET_LIGHT_FPGA_FM_C910
obj-y += ddr.o
else # // CONFIG_TARGET_LIGHT_FPGA_FM_C910
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/init_ddr.o
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/pinmux.o
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/waitfwdone.o
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/ddr_common_func.o
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/common_lib.o
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/lpddr4_init.o
ifdef CONFIG_DDR_DBI_OFF
obj-$(CONFIG_DDR_LP4X_4266_DUALRANK) += $(DDR_FW_PATH)/lp4x_4266_phy_train1d2d_dualrank_dbioff.o
obj-$(CONFIG_DDR_LP4X_3733_DUALRANK) += $(DDR_FW_PATH)/lp4x_3733_phy_train1d2d_dualrank_dbioff.o
obj-$(CONFIG_DDR_LP4X_3200_DUALRANK) += $(DDR_FW_PATH)/lp4x_3200_phy_train1d2d_dualrank_dbioff.o
obj-$(CONFIG_DDR_LP4_4266_DUALRANK) += $(DDR_FW_PATH)/lp4_4266_phy_train1d2d_dualrank_dbioff.o
obj-$(CONFIG_DDR_LP4_3733_DUALRANK) += $(DDR_FW_PATH)/lp4_3733_phy_train1d2d_dualrank_dbioff.o
obj-$(CONFIG_DDR_LP4X_4266_SINGLERANK) += $(DDR_FW_PATH)/lp4x_4266_phy_train1d2d_dbioff.o
obj-$(CONFIG_DDR_LP4X_3733_SINGLERANK) += $(DDR_FW_PATH)/lp4x_3733_phy_train1d2d_dbioff.o
obj-$(CONFIG_DDR_LP4X_3200_SINGLERANK) += $(DDR_FW_PATH)/lp4x_3200_phy_train1d2d_dbioff.o
obj-$(CONFIG_DDR_LP4_4266_SINGLERANK) += $(DDR_FW_PATH)/lp4_4266_phy_train1d2d_dbioff.o
obj-$(CONFIG_DDR_LP4_3733_SINGLERANK) += $(DDR_FW_PATH)/lp4_3733_phy_train1d2d_dbioff.o
else # // CONFIG_DDR_DBI_OFF
obj-$(CONFIG_DDR_LP4X_4266_DUALRANK) += $(DDR_FW_PATH)/lp4x_4266_phy_train1d2d_dualrank.o
obj-$(CONFIG_DDR_LP4X_3733_DUALRANK) += $(DDR_FW_PATH)/lp4x_3733_phy_train1d2d_dualrank.o
obj-$(CONFIG_DDR_LP4X_3200_DUALRANK) += $(DDR_FW_PATH)/lp4x_3200_phy_train1d2d_dualrank.o
obj-$(CONFIG_DDR_LP4X_2133_DUALRANK) += $(DDR_FW_PATH)/lp4x_2133_phy_train1d2d_dualrank.o
obj-$(CONFIG_DDR_LP4_4266_DUALRANK) += $(DDR_FW_PATH)/lp4_4266_phy_train1d2d_dualrank.o
obj-$(CONFIG_DDR_LP4_3733_DUALRANK) += $(DDR_FW_PATH)/lp4_3733_phy_train1d2d_dualrank.o
obj-$(CONFIG_DDR_LP4X_4266_SINGLERANK) += $(DDR_FW_PATH)/lp4x_4266_phy_train1d2d.o
obj-$(CONFIG_DDR_LP4X_3733_SINGLERANK) += $(DDR_FW_PATH)/lp4x_3733_phy_train1d2d.o
obj-$(CONFIG_DDR_LP4X_3200_SINGLERANK) += $(DDR_FW_PATH)/lp4x_3200_phy_train1d2d.o
obj-$(CONFIG_DDR_LP4X_2133_SINGLERANK) += $(DDR_FW_PATH)/lp4x_2133_phy_train1d2d.o
obj-$(CONFIG_DDR_LP4_4266_SINGLERANK) += $(DDR_FW_PATH)/lp4_4266_phy_train1d2d.o
obj-$(CONFIG_DDR_LP4_3733_SINGLERANK) += $(DDR_FW_PATH)/lp4_3733_phy_train1d2d.o
endif # // CONFIG_DDR_DBI_OFF
obj-$(CONFIG_LPDDR_EYE) += $(DDR_SRC_PATH)/lp4_diag_eye.o
endif # // CONFIG_TARGET_LIGHT_FPGA_FM_C910
else # // CONFIG_SPL_BUILD
obj-y += light.o
obj-y += board.o
obj-$(CONFIG_THEAD_LIGHT_TIMER) += timer.o
obj-$(CONFIG_THEAD_LIGHT_DIGITAL_SENSOR) += digital_sensor.o digital_sensor_test.o
obj-y += clock_config.o
obj-y += sec_check.o
obj-y += boot.o
ifndef CONFIG_TARGET_LIGHT_FPGA_FM_C910
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/ddr_common_func.o
obj-$(CONFIG_LPDDR) += $(DDR_SRC_PATH)/common_lib.o
endif

obj-y += light-sv/pll_io_test.o
obj-y += light-sv/adc_test.o
obj-y += version_rollback.o
obj-$(CONFIG_AVB_VERIFY) += secimg_load.o
endif
