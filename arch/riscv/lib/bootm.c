// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/root.h>
#include <image.h>
#include <opensbi.h>
#include <asm/byteorder.h>
#include <asm/csr.h>
#include <asm/io.h>
#include <asm/smp.h>
#include <asm/barrier.h>
#include <asm/atomic.h>
#include <asm/arch-thead/light-reset.h>
#include <dm/device.h>
#include <dm/root.h>
#include <u-boot/zlib.h>

DECLARE_GLOBAL_DATA_PTR;

static struct fw_dynamic_info opensbi_info;
static atomic_t _harts_count = ATOMIC_INITIALIZER(3);
static ulong _load_start;
static ulong _dtb_addr;
static ulong _dyn_info_addr;

extern void secondary_entry();

__weak void board_quiesce_devices(void)
{
}

int arch_fixup_fdt(void *blob)
{
	return 0;
}

/**
 * announce_and_cleanup() - Print message and prepare for kernel boot
 *
 * @fake: non-zero to do everything except actually boot
 */
static void announce_and_cleanup(int fake)
{
	printf("\nStarting kernel ...%s\n\n", fake ?
		"(fake run for tracing)" : "");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");
#ifdef CONFIG_BOOTSTAGE_FDT
	bootstage_fdt_add_report();
#endif
#ifdef CONFIG_BOOTSTAGE_REPORT
	bootstage_report();
#endif

#ifdef CONFIG_USB_DEVICE
	udc_disconnect();
#endif

	board_quiesce_devices();

	/*
	 * Call remove function of all devices with a removal flag set.
	 * This may be useful for last-stage operations, like cancelling
	 * of DMA operation or releasing device internal buffers.
	 */
	dm_remove_devices_flags(DM_REMOVE_ACTIVE_ALL);

	cleanup_before_linux();
}

static void boot_prep_linux(bootm_headers_t *images)
{
	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
#ifdef CONFIG_OF_LIBFDT
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
#endif
	} else {
		printf("Device tree not found or missing FDT support\n");
		hang();
	}
}

void next_stage(void)
{
	void (*next_entry)(unsigned long arg0,unsigned long arg1,unsigned long arg2);

	next_entry	= (void (*))(_load_start);
	ulong hartid = csr_read(CSR_MHARTID);

	atomic_sub_return(&_harts_count, 1);
	/*
	 * set $a0 = hartid
	 * set $a1 = $dtb_addr
	 * set $a2 = $dyn_info_addr
	 */
	next_entry(hartid, _dtb_addr , _dyn_info_addr);
}

bool has_reset_sample(ulong dtb_addr)
{
	int node_offset;
	node_offset = fdt_path_offset(dtb_addr, "/soc/reset-sample");
	if (node_offset < 0) {
		printf("## fdt has no reset_sample\n");
		return false;
	} else {
		printf("## fdt has reset_sample\n");
		return true;
	}
}

static void reset_sample(void)
{
	ulong addr;
	uint addr_l, addr_h;

	// RESET ADDR
	addr = (unsigned long)(void *)secondary_entry;
	addr_h = (uint)(addr >> 32);
	addr_l = (uint)(addr & 0xFFFFFFFF);
	// writel(addr_h, (volatile void *)REG_C910_CORE0_RVBA_H);
	// writel(addr_l, (volatile void *)REG_C910_CORE0_RVBA_L);
	writel(addr_h, (volatile void *)REG_C910_CORE1_RVBA_H);
	writel(addr_l, (volatile void *)REG_C910_CORE1_RVBA_L);
	writel(addr_h, (volatile void *)REG_C910_CORE2_RVBA_H);
	writel(addr_l, (volatile void *)REG_C910_CORE2_RVBA_L);
	writel(addr_h, (volatile void *)REG_C910_CORE3_RVBA_H);
	writel(addr_l, (volatile void *)REG_C910_CORE3_RVBA_L);

	// RESET
	writel(0x1F, (volatile void *)REG_C910_SWRST);
	writel(0x1, (volatile void *)REG_PLIC_DELEGATE);
}

static void boot_jump_linux(bootm_headers_t *images, int flag)
{
	void (*kernel)(ulong hart, void *dtb, struct fw_dynamic_info *p);
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);
#ifdef CONFIG_SMP
	int ret;
#endif

	kernel = (void (*)(ulong, void *, struct fw_dynamic_info*))simple_strtol(env_get("opensbi_addr"), NULL, 0);

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	debug("## Transferring control to kernel (at address %08lx) ...\n",
	      (ulong)kernel);

	announce_and_cleanup(fake);

	_load_start = kernel;
	_dtb_addr = images->ft_addr;
	_dyn_info_addr = (ulong)&opensbi_info;
	if (!has_reset_sample(_dtb_addr)) {
		opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
		opensbi_info.version = 0x2;
		opensbi_info.next_addr = images->os.start;
		opensbi_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
		opensbi_info.options = 0;
		opensbi_info.boot_hart = 0;
		reset_sample();
	} else {
		opensbi_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
		opensbi_info.version = 0x1;
		opensbi_info.next_addr = images->os.start;
		opensbi_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
		opensbi_info.options = 0;
		opensbi_info.boot_hart = 0;
	}

	if (!fake) {
		if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
#ifdef CONFIG_SMP
			ret = smp_call_function(images->ep,
						(ulong)images->ft_addr, 0, 0);
			if (ret)
				hang();
#endif
			kernel(gd->arch.boot_hart, images->ft_addr, &opensbi_info);
		}
	}
}

int do_bootm_linux(int flag, int argc, char * const argv[],
		   bootm_headers_t *images)
{
	/* No need for those on RISC-V */
	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP) {
		boot_prep_linux(images);
		return 0;
	}

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		boot_jump_linux(images, flag);
		return 0;
	}

	boot_prep_linux(images);
	boot_jump_linux(images, flag);
	return 0;
}

int do_bootm_vxworks(int flag, int argc, char * const argv[],
		     bootm_headers_t *images)
{
	return do_bootm_linux(flag, argc, argv, images);
}
