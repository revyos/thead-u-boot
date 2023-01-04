// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <asm/io.h>
#include <dwc3-uboot.h>
#include <usb.h>
#include <cpu_func.h>

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_SUPER,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

int usb_gadget_handle_interrupts(int index)
{
	dwc3_uboot_handle_interrupt(index);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	dwc3_device_data.base = 0xFFE7040000UL;
	return dwc3_uboot_init(&dwc3_device_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	dwc3_uboot_exit(index);
	return 0;
}

int g_dnl_board_usb_cable_connected(void)
{
	return 1;
}
#endif

#ifdef CONFIG_CMD_BOOT_SLAVE
#define E902_SYSREG_START	0xfffff48044
#define E902_SYSREG_RESET	0xfffff44024
#define E902_START_ADDRESS	0xFFEF8000
#define C910_E902_START_ADDRESS 0xFFFFEF8000
#define E902_IOPMP_BASE		0xFFFFC21000

#define C906_RST_ADDR_L		0xfffff48048
#define C906_RST_ADDR_H		0xfffff4804C
#define C906_START_ADDRESS_L	0xc0000000
#define C906_START_ADDRESS_H	0xff
#define C910_C906_START_ADDRESS	0xffc0000000
#define C906_CPR_IPCG_ADDRESS   0xFFCB000010
#define C906_IOCTL_GPIO_SEL_ADDRESS     0xFFCB01D000
#define C906_IOCTL_AF_SELH_ADDRESS      0xFFCB01D008
#define C906_RESET_REG                  0xfffff4403c


void set_slave_cpu_entry(phys_addr_t entry)
{
    writel(entry, (void *)E902_SYSREG_START);
}

void disable_slave_cpu(void)
{
    writel(0x0, (void *)E902_SYSREG_RESET);
}

void enable_slave_cpu(void)
{
    writel(0x3, (void *)E902_SYSREG_RESET);
}

void set_c906_cpu_entry(phys_addr_t entry_h, phys_addr_t entry_l)
{
	writel(entry_h, (volatile void *)C906_RST_ADDR_H);
	writel(entry_l, (volatile void *)C906_RST_ADDR_L);
}

void boot_audio(void)
{
        writel(0x37, (volatile void *)C906_RESET_REG);

        set_c906_cpu_entry(C906_START_ADDRESS_H, C906_START_ADDRESS_L);
        flush_cache((uintptr_t)C910_C906_START_ADDRESS, 0x20000);

        writel(0x7ffff1f, (volatile void *)C906_CPR_IPCG_ADDRESS);
        writel((1<<23) | (1<<24), (volatile void *)C906_IOCTL_GPIO_SEL_ADDRESS);
        writel(0, (volatile void *)C906_IOCTL_AF_SELH_ADDRESS);

        writel(0x3f, (volatile void *)C906_RESET_REG);
}

void boot_aon(void)
{
	writel(0xffffffff, (void *)(E902_IOPMP_BASE + 0xc0));
	disable_slave_cpu();
	set_slave_cpu_entry(E902_START_ADDRESS);
	flush_cache((uintptr_t)C910_E902_START_ADDRESS, 0x10000);
	enable_slave_cpu();
}

int do_bootslave(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	boot_aon();
	mdelay(100);
	boot_audio();
	return 0;
}
#endif
