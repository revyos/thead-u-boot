/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/types.h>
#include <asm/asm.h>
#include <asm/csr.h>
#include <common.h>
#include <cpu_func.h>
#include <fdt_support.h>

#ifdef CONFIG_TARGET_ICE_C910
static phys_addr_t opensbi_addr, dtb_addr;
void (*image_entry)(u32, phys_addr_t);

extern bootm_headers_t images;
static inline void boot_core_vector(void)
{
	/* Set pmp regs */
	csr_write(pmpaddr0, 0x0 >> 2 | ((0x100000000 - 1) >> 3));
	csr_write(pmpaddr1, 0x3f0000000 >> 2 | ((0x10000000 - 1) >> 3));
	csr_write(pmpaddr6, 0x00000000 >> 2 | ((0x10000000 - 1) >> 3));
	csr_write(pmpaddr7, 0xffffffffff >> 2);
	csr_write(pmpcfg0, 0x8898000000001b1f);

	/* Set cpu regs */
	csr_write(CSR_MCOR, 0x70013);
	csr_write(CSR_MCCR2, 0xe0410009);
	csr_write(CSR_MHCR, 0x11ff);
	csr_write(CSR_MXSTATUS, 0x638000);
	csr_write(CSR_MHINT, 0x16e30c);

	image_entry(0xdeadbeef, dtb_addr);
}

static void set_vector_cpu(void)
{
	int node, cpu;
	const void *blob = (const void *)dtb_addr;

	node = fdt_path_offset(blob, "/cpus");
	if (node < 0)
		return;

	for (cpu = fdt_first_subnode(blob, node);
		cpu >= 0; cpu = fdt_next_subnode(blob, cpu)) {
		if (!strcmp("okay", fdt_getprop(blob, cpu, "status", NULL)))
			fdt_status_fail((void *)blob, cpu);
		else if (!strcmp("fail", fdt_getprop(blob, cpu, "status", NULL)))
			fdt_status_okay((void *)blob, cpu);
	}
}

static void prep_core_vector(void)
{
	printf("Boot vector core(cpu2) only\n");

	dtb_addr = (phys_addr_t)images.ft_addr;

	opensbi_addr = simple_strtol(env_get("opensbi_addr"), NULL, 0);
	image_entry = (void (*)(u32, phys_addr_t))(opensbi_addr);

	set_vector_cpu();

	csr_write(mrvbr, &boot_core_vector);
	csr_write(mrmr, 0x5);

	while (1);
}
#endif

/*
 * cleanup_before_linux() is called just before we call linux
 * it prepares the processor for linux
 *
 * we disable interrupt and caches.
 */
int cleanup_before_linux(void)
{
	cache_flush();

#ifdef CONFIG_TARGET_ICE_C910
	if ((simple_strtol(env_get("boot_vector"), NULL, 0)) == 1)
		prep_core_vector();
#endif

	return 0;
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		asm volatile(".long 0x0295000b");  /* dcache.cpa a0 */

	sync_is();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		asm volatile(".long 0x02b5000b");  /* dcache.cipa a0 */

	sync_is();
}

void invalid_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(CONFIG_SYS_CACHELINE_SIZE - 1);

	for (; i < end; i += CONFIG_SYS_CACHELINE_SIZE)
		asm volatile(".long 0x02a5000b");  /* dcache.ipa a0 */

	sync_is();
}

void icache_enable(void)
{
#ifdef CONFIG_SPL_BUILD
#ifdef CONFIG_SPL_RISCV_MMODE
#ifdef CONFIG_TARGET_LIGHT_C910
	asm volatile (
        "csrr x29, mhcr\n\t"
        "ori x28, x29, 0x1\n\t"
        "csrw mhcr, x28\n\t"
    );
#endif
#endif
#endif
}

void dcache_enable(void)
{
#ifdef CONFIG_SPL_BUILD
#ifdef CONFIG_SPL_RISCV_MMODE
#ifdef CONFIG_TARGET_LIGHT_C910
	asm volatile (
        "li x29, 0x11ff\n\t"
        "csrw mhcr, x29\n\t"
    );
#endif
#endif
#endif
}
