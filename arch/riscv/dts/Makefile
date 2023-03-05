# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_AX25_AE350) += ae350_32.dtb ae350_64.dtb
dtb-$(CONFIG_TARGET_SIFIVE_FU540) += hifive-unleashed-a00.dtb
dtb-$(CONFIG_TARGET_ICE_C910) += ice-c910.dtb
dtb-$(CONFIG_TARGET_LIGHT_EVB_MPW_C910) += light-evb-mpw-c910.dtb
dtb-$(CONFIG_TARGET_LIGHT_FPGA_FM_C910) += light-fpga-fm-c910.dtb
dtb-$(CONFIG_TARGET_LIGHT_C910) += light-a-ref.dtb light-b-ref.dtb light-a-val.dtb light-b-product.dtb light-a-product.dtb light-ant-ref.dtb light-beagle.dtb light-b-power.dtb light-lpi4a.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
