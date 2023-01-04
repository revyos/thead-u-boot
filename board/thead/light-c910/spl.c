/*
* Copyright (C) 2017-2020 Alibaba Group Holding Limited
*
* SPDX-License-Identifier: GPL-2.0+
*/

#include <linux/types.h>
#include <common.h>
#include <console.h>
#include <cpu_func.h>
#include <asm/csr.h>
#include <asm/io.h>
#include <asm/barrier.h>
#include <spl.h>
#include <asm/spl.h>
#include <asm/arch-thead/boot_mode.h>
#include <string.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include "../common/uart.h"
#include "../common/mini_printf.h"
#include "lpddr-regu/ddr_regu.h"

DECLARE_GLOBAL_DATA_PTR;

extern void init_ddr(void);
extern void cpu_clk_config(int cpu_freq);
extern void sys_clk_config(void);
extern void ddr_clk_config(int ddr_freq);
extern void show_sys_clk(void);
extern int riscv_get_time(u64 *time);
extern int pmic_reset_apcpu_voltage(void);

struct light_reset_list {
        u32 val;
        u64 reg;
};

static struct light_reset_list light_pre_reset_lists[] = {
	{0x00000037, 0xFFFFF4403C}, /* Aon: Audio sys rst */
};

static struct light_reset_list light_post_reset_lists[] = {
	{0x00000001, 0xFFFF0151B0}, /* AP rst_gen: NPU rst */
	{0xFFFFFFFF, 0xFFFF041028}, /* DSP sys_reg: DSP rst */
	{0x00000002, 0xFFEF528000}, /* VO sys_reg: GPU rst */
	{0x00000003, 0xFFEF528000}, /* VO sys_reg: GPU rst */
	{0x00000007, 0xFFFF529004}, /* VO sys_reg: DPU rst */
};

static void light_pre_reset_config(void)
{
	/* Reset VI/VO/VP/DSP/NPU/GPU/DPU */
	int i = 0;
	int entry_size;

	entry_size = ARRAY_SIZE(light_pre_reset_lists);

	while (i < entry_size) {
		writel(light_pre_reset_lists[i].val, (void *)(light_pre_reset_lists[i].reg));
		i++;
	}
}

static void light_post_reset_config(void)
{
	/* Reset VI/VO/VP/DSP/NPU/GPU/DPU */
	int i = 0;
	int entry_size;

	entry_size = ARRAY_SIZE(light_post_reset_lists);

	while (i < entry_size) {
		writel(light_post_reset_lists[i].val, (void *)(light_post_reset_lists[i].reg));
        udelay(2);
		i++;
	}
}

void setup_ddr_pmp(void)
{
	/* clear pmp entry0,entry1 setting in bootrom */
	writel(0x0 >> 12, (void *)(PMP_BASE_ADDR + 0x104));
	writel(0x0 >> 12, (void *)(PMP_BASE_ADDR + 0x100));
	writel(0x0 >> 12, (void *)(PMP_BASE_ADDR + 0x10c));
	writel(0x0 >> 12, (void *)(PMP_BASE_ADDR + 0x108));

	writel(0, (void *)(PMP_BASE_ADDR + 0x000));

	sync_is();
}

int get_rng(unsigned int *rng, int cnt)
{
	int i;
	u64 seed;
	riscv_get_time(&seed);
	srand((unsigned int)seed);
	for (i = 0; i < cnt; i++)
		rng[i] = rand();
	return 0;
}
struct axiscr_region {
	long start;
	long end;
};
#define AXISCR_MAX_REGION_CNT 8
#define OFFSET_AXISCR_LOCK 0x4
#define OFFSET_AXISCR_MISC 0x8
#define OFFSET_AXISCR_CYPHER 0x14
#define OFFSET_AXISCR_REGION 0x40
#define OFFSET_AXISCR_TRNG 0x100
void setup_ddr_scramble(void)
{
	int node, scr;
	unsigned int i, tmp;
	long base_addr, start, size, end;
	const fdt32_t *reg;
	const char *status;
	int lock_r, lock_w;
	const void *blob = (const void *)gd->fdt_blob;
	const char path[] = "/soc/axiscr";
	struct axiscr_region region[AXISCR_MAX_REGION_CNT] = {0};
	unsigned int rng[AXISCR_MAX_REGION_CNT*2] = {0}; // dual word per region
	int cnt = 0;

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		printf("found no %s node in fdt\n", path);
		return;
	}

	reg = fdt_getprop(blob, node, "reg", NULL);
	if (!reg) {
		printf("Warning: device tree node '%s' has no address.\n", path);
		return;
	}
	base_addr = fdt_translate_address(blob, node, reg);

	status = fdt_getprop(blob, node, "lock-read", NULL);
	lock_r = (!strcmp(status, "okay")) ? 1:0;
	status = fdt_getprop(blob, node, "lock-write", NULL);
	lock_w = (!strcmp(status, "okay")) ? 1:0;

	for (scr = fdt_first_subnode(blob, node);
		scr >= 0; scr = fdt_next_subnode(blob, scr)) {
		if (!strcmp("okay", fdt_getprop(blob, scr, "status", NULL))) {
			reg = fdt_getprop(blob, scr, "region", NULL);
			start = fdt_translate_address(blob, scr, reg);
			reg += 2;
			size = fdt_translate_address(blob, scr, reg);
			end = start + size;

			region[cnt].start = start;
			region[cnt++].end = end;
			// TODO, check overlap
		}
	}

	if (cnt > 0) {
		if (cnt > AXISCR_MAX_REGION_CNT) {
			printf("failed to setup ddr scramble, since illegal axiscr region cnt<%d>", cnt);
			return;
		}

		get_rng(rng, cnt*2);
		for (i=0; i< cnt; i++) {
			// config region
			writel(region[i].start >> 12, (void *)(base_addr + OFFSET_AXISCR_REGION + i*8));
			writel(region[i].end >> 12, (void *)(base_addr + OFFSET_AXISCR_REGION + i*8+4));

			// config rng
			writel(rng[i*2], (void *)(base_addr + OFFSET_AXISCR_TRNG + i*8));
			writel(rng[i*2+1], (void *)(base_addr + OFFSET_AXISCR_TRNG + i*8+4));
		}
		// enable axi scramble
		tmp = readl((void *)(base_addr + OFFSET_AXISCR_MISC));
		tmp |= 1 << 18;
		writel(tmp, (void *)(base_addr + OFFSET_AXISCR_MISC));

		writel(1 << 0, (void *)(base_addr + OFFSET_AXISCR_CYPHER));

		tmp = readl((void *)(base_addr + OFFSET_AXISCR_MISC));
		tmp &= ~(0xff << 24);
		tmp |= 1 << 24;
		writel(tmp, (void *)(base_addr + OFFSET_AXISCR_MISC));

		// lock r/w
		tmp = readl((void *)(base_addr + OFFSET_AXISCR_LOCK));
		if (lock_r) {
			tmp |= 1 << 7;
			writel(tmp, (void *)(base_addr + OFFSET_AXISCR_LOCK));
		}
		if (lock_w) {
			tmp |= 1 << 8;
			writel(tmp, (void *)(base_addr + OFFSET_AXISCR_LOCK));
		}
		sync_is();
	}
}

struct axiparity_region {
	long start;
	long size;
};
#define AXIPARITY_MAX_REGION_CNT 8
#define OFFSET_AXIPARITY_CFG 0x0
#define OFFSET_AXIPARITY_REGION_CFG0 0x4
#define OFFSET_AXIPARITY_REGION_CFG1 0x8
#define OFFSET_AXIPARITY_SLFT_CFG0 0x44
#define OFFSET_AXIPARITY_SLFT_CFG1 0x48
#define OFFSET_AXIPARITY_SLFT_CFG2 0x4C
void setup_ddr_parity(void)
{
	int node, parity;
	unsigned int i, tmp;
	long base_addr, start, size;
	const fdt32_t *reg;
	const char *status;
	int lock;
	const void *blob = (const void *)gd->fdt_blob;
	const char path[] = "/soc/axiparity";
	struct axiparity_region region[AXIPARITY_MAX_REGION_CNT] = {0};
	int cnt = 0;

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		printf("found no %s node in fdt\n", path);
		return;
	}

	reg = fdt_getprop(blob, node, "reg", NULL);
	if (!reg) {
		printf("Warning: device tree node '%s' has no address.\n", path);
		return;
	}
	base_addr = fdt_translate_address(blob, node, reg);

	status = fdt_getprop(blob, node, "lock", NULL);
	lock = (!strcmp(status, "okay")) ? 1:0;

	for (parity = fdt_first_subnode(blob, node);
		parity >= 0; parity = fdt_next_subnode(blob, parity)) {
		if (!strcmp("okay", fdt_getprop(blob, parity, "status", NULL))) {
			reg = fdt_getprop(blob, parity, "region", NULL);
			start = fdt_translate_address(blob, parity, reg);
			reg += 2;
			size = fdt_translate_address(blob, parity, reg);

			region[cnt].start = start;
			region[cnt++].size = size;
			// TODO, check overlap
		}
	}

	if (cnt > 0) {
		if (cnt > AXIPARITY_MAX_REGION_CNT) {
			printf("failed to setup ddr parity, since illegal axiparity region cnt<%d>", cnt);
			return;
		}

		for (i=0; i< cnt; i++) {
			// config region
			writel(region[i].start >> 12, (void *)(base_addr + OFFSET_AXIPARITY_REGION_CFG0 + i*8));
			writel(region[i].size >> 12, (void *)(base_addr + OFFSET_AXIPARITY_REGION_CFG1 + i*8));
		}

		// parity region number
		tmp = readl((void *)(base_addr + OFFSET_AXIPARITY_CFG));
		tmp |= cnt << 0;
		writel(tmp, (void *)(base_addr + OFFSET_AXIPARITY_CFG));

		for (i=0; i< cnt; i++) {
			// selftest config
			writel((region[i].start >> 12) << 8, (void *)(base_addr + OFFSET_AXIPARITY_SLFT_CFG0 + i*0xc));
			writel((region[i].size >> 12) << 8, (void *)(base_addr + OFFSET_AXIPARITY_SLFT_CFG1 + i*0xc));
			writel(1 << 0, (void *)(base_addr + OFFSET_AXIPARITY_SLFT_CFG2 + i*0xc));
		}
		mdelay(10); //4ms for 4GB SLFT

		// enable axi parity
		tmp = readl((void *)(base_addr + OFFSET_AXIPARITY_CFG));
		tmp &= ~(0xff << 24);
		tmp |= 1 << 24;
		writel(tmp, (void *)(base_addr + OFFSET_AXIPARITY_CFG));

		// lock
		if (lock) {
			tmp = readl((void *)(base_addr + OFFSET_AXIPARITY_CFG));
			tmp |= 1 << 8;
			writel(tmp, (void *)(base_addr + OFFSET_AXIPARITY_CFG));
		}
		sync_is();
	}
}

void cpu_performance_enable(void)
{
#define CSR_MHINT2_E	0x7cc
#define CSR_MHINT4	0x7ce
	csr_write(CSR_SMPEN, 0x1);
	csr_write(CSR_MHINT2_E, csr_read(CSR_MHINT2_E) | 0x20000);
	csr_write(CSR_MHINT4, csr_read(CSR_MHINT4) | 0x410);
	csr_write(CSR_MCCR2, 0xe2490009);
	csr_write(CSR_MHCR, 0x117f); // clear bit7 to disable indirect brantch prediction
	csr_write(CSR_MXSTATUS, 0x638000);
	csr_write(CSR_MHINT, 0x6e30c | (1<<21) | (1<<22)); // set bit21 & bit 22 to close tlb & fence broadcast
}

static int bl1_img_have_head(unsigned long img_src_addr)
{
    uint8_t *buffer = (uint8_t *)img_src_addr;

    if (memcmp(header_magic, &buffer[4], 4) == 0) {
        return 1;
    }

    return 0;
}

static void light_board_init_r(gd_t *gd, ulong dummy)
{
	void (*entry)(long, long);
	void *sram_uboot_start;

	cpu_performance_enable();

	sram_uboot_start = (void *)(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_MAX_SIZE);
	if (bl1_img_have_head((unsigned long)SRAM_BASE_ADDR) == 1) {
		uint32_t sign_en, encrypt_en;
		img_header_t *phead;

		phead = (img_header_t *)SRAM_BASE_ADDR;
		sign_en = phead->option_flag & 0x1;
		encrypt_en = phead->option_flag & 0x2;
		printf("image has header, sign %s, encrypt %s\n",
			sign_en ? "en" : "disabled",
			encrypt_en ? "en" : "disabled");
	} else {
		printf("image has no header\n");
	}
	memcpy((void *)CONFIG_SYS_TEXT_BASE, sram_uboot_start, CONFIG_SYS_MONITOR_LEN);
	entry = (void (*)(long, long))CONFIG_SYS_TEXT_BASE;
	invalidate_icache_all();
	flush_dcache_range(CONFIG_SYS_TEXT_BASE, CONFIG_SYS_TEXT_BASE + CONFIG_SYS_MONITOR_LEN);
	entry(0, 0);

	while (1);
}

void board_init_f(ulong dummy)
{
	int ret;

	light_pre_reset_config();
	sys_clk_config();

	ret = spl_early_init();
	if (ret) {
		printf("spl_early_init() failed: %d\n", ret);
		hang();
	}

	arch_cpu_init_dm();
	light_post_reset_config();
	preloader_console_init();

#ifdef CONFIG_PMIC_VOL_INIT
	ret = pmic_ddr_regu_init();
	if (ret) {
		printf("%s pmic init failed %d \n",__func__,ret);
		hang();
	}

	ret = pmic_ddr_set_voltage();
	if (ret) {
		printf("%s set ddr voltage failed \n",__func__);
		hang();
	}

	ret = pmic_reset_apcpu_voltage();
	if (ret) {
		printf("%s set apcpu voltage failed \n",__func__);
		hang();
	}

#endif
	ddr_clk_config(0);
	cpu_clk_config(0);

	init_ddr();
	setup_ddr_scramble();
	setup_ddr_parity();
	setup_ddr_pmp();

	printf("ddr initialized, jump to uboot\n");
	light_board_init_r(NULL, 0);
}

void board_boot_order(u32 *spl_boot_list)
{
#define SOC_OM_ADDRBASE        0xffef018010
	switch (readl((void *)SOC_OM_ADDRBASE) & 0x7) {
	case 0:
	case 1:
	case 2:
	case 3:
		/* usb boot */
		break;
	case 4:
		/* emmc boot */
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case 5:
		/* sd boot */
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	case 6:
		/* qspi-nand boot */
		spl_boot_list[0] = BOOT_DEVICE_NAND;
		break;
	case 7:
		/* spi-nor boot */
		spl_boot_list[0] = BOOT_DEVICE_SPI;
		break;
	default:
		spl_boot_list[0] = BOOT_DEVICE_NONE;
	}

	cpu_performance_enable();
}
