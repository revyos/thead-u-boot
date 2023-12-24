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
#include <abuf.h>
#include "sec_library.h"

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

#define C906_START_ADDRESS_L	0x32000000
#define C906_START_ADDRESS_H	0x00
#define C910_C906_START_ADDRESS	0x0032000000

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

#ifdef CONFIG_BOARD_RNG_SEED
const char pre_gen_seed[128] = {211, 134, 226, 116, 1, 13, 224, 196, 88, 213, 188, 219, 128, 41, 231, 228, 129, 123, 173, 234, 219, 79, 152, 154, 169, 27, 183, 166, 52, 21, 118, 7, 155, 89, 124, 156, 102, 92, 96, 190, 49, 28, 154, 177, 69, 129, 149, 199, 253, 66, 177, 216, 146, 73, 114, 59, 100, 41, 225, 152, 62, 88, 160, 217, 177, 28, 117, 23, 120, 213, 213, 169, 242, 111, 90, 55, 241, 239, 254, 238, 50, 175, 198, 196, 248, 56, 255, 92, 97, 224, 245, 160, 56, 149, 121, 233, 177, 239, 0, 41, 196, 214, 210, 182, 69, 44, 238, 54, 27, 236, 36, 77, 156, 234, 17, 148, 34, 16, 241, 132, 241, 230, 36, 41, 123, 157, 19, 44};
/* Use hardware rng to seed Linux random. */
int board_rng_seed(struct abuf *buf)
{
	size_t len = 128;
	uint8_t *data = NULL;
	int sc_err = SC_FAIL;

	/* abuf is working up in asynchronization mode, so the memory usage for random data storage must
	   be allocated first. */
	data = malloc(len);
	if (!data) {
		printf("Fail to allocate memory, using pre-defined entropy\n");
		return -1;
	}

#if defined(CONFIG_AVB_HW_ENGINE_ENABLE)
	/* We still use pre-define entropy data in case hardware random engine does not work */
	sc_err = csi_sec_library_init();
	if (sc_err != SC_OK) {
		printf("Fail to initialize sec library, using pre-defined entropy\n");
		goto _err;
	}

	sc_err = sc_rng_get_random_bytes(data, len);
	if (sc_err != SC_OK) {
		printf("Fail to retrieve random data, using pre-defined entropy\n");
		goto _err;
	}

	abuf_init_set(buf, data, len);
	return 0;

_err:
#endif
	/* use pre-defined random data in case of the random engine is disable */
	memcpy(data, pre_gen_seed, len);
	abuf_init_set(buf, data, len);

	return 0;
}
#endif
