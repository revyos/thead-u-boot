// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright(C) 2021 Alibaba Communications Inc.
 * Author: David Li <liyong.li@alibaba-inc.com>
 */
/*
 * PRBS test pattern
 *
 */

#include <common.h>
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <hash.h>
#include <mapmem.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <asm/asm.h>
#include <asm/csr.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "queue.h"

//#define DEBUG
#ifdef DEBUG
#define CHECK_RESULT
#endif

#ifdef CHECK_RESULT
static bool checked = true;
#else
static bool checked = false;
#endif

#define FIXED_RANGE 0x100000 // write fixed range for prbs_31 only

#define BYTE_PER_CACHELINE 64
#define BIT_PER_BYTE 8

#define TIMER_FREQ (3*1000*1000)
#define TIMER_TICK_MS (TIMER_FREQ/1000)

u64 t_begin;
u64 t_end;

extern void flush_dcache_range(unsigned long start, unsigned long end);
extern void invalidate_dcache_range(unsigned long start, unsigned long end);
extern void invalid_dcache_range(unsigned long start, unsigned long end);

extern unsigned long get_ddr_density(void);
extern int riscv_get_time(u64 *time);

struct PRBS_ELE {
    char *name;
    char *init_value;
    unsigned int init_cnt;
    char *expression;
    unsigned int exp_cnt;
};

char prbs7_init_value[] = {1,1,1,1,1,0,1};
char prbs7_exp[] = {7,3};
struct PRBS_ELE prbs7 = {
    "prbs_7",
    prbs7_init_value,
    7,
    prbs7_exp,
    2
};

char prbs15_init_value[] = {1,1,1,1,1,0,1,0,1,1,0,1,1,1,0};
char prbs15_exp[] = {15,1};
struct PRBS_ELE prbs15 = {
    "prbs_15",
    prbs15_init_value,
    15,
    prbs15_exp,
    2
};

char prbs23_init_value[] = {1,1,1,1,1,0,1,0,1,1,0,1,1,1,0,0,1,0,1,1,1,0,1};
char prbs23_exp[] = {23,5};
struct PRBS_ELE prbs23 = {
    "prbs_23",
    prbs23_init_value,
    23,
    prbs23_exp,
    2
};

char prbs31_init_value[] = {1,1,1,1,1,0,1,0,1,1,0,1,1,1,0,0,1,0,1,1,1,0,1,0,0,0,1,0,0,1,1};
char prbs31_exp[] = {31,3};
struct PRBS_ELE prbs31 = {
    "prbs_31",
    prbs31_init_value,
    31,
    prbs31_exp,
    2
};


unsigned int _generate_prbs_pos(unsigned int pos, unsigned int bit_value) {
    unsigned int pat = 0;
    int i;
    for(i=0; i<sizeof(pat)*BIT_PER_BYTE; i++)
    {
        pat |= bit_value << i;
    }
    return pat;
}
unsigned int _generate_prbs_neg(unsigned int pos, unsigned int bit_value) {
    unsigned int pat = 0;
    pat = bit_value;
    
    // reverse bit
    if((bit_value & (1 << pos)) == 0) {
        pat |= (unsigned int)0x1 << pos;
    }
    else {
        pat &= ~(bit_value & (1 << pos));
    }

    return pat;
}
unsigned int _generate_prbs_neg_all(unsigned int pos, unsigned int bit_value) {
    unsigned int pat = 0;
    int i;
    for(i=0; i<sizeof(pat)*BIT_PER_BYTE; i++)
    {
        pat |= bit_value << i;
    }
    pat = ~pat;
    return pat;
}

#define BYTE_PER_INTERLEAVE 512

#ifdef CONFIG_DDR_CHBIT_256B
//#define DDR_PHY0_ONLY
#define DDR_PHY1_ONLY
#define MULTIPLY 2
#else
#define MULTIPLY 1
#endif

int prbs_test(struct PRBS_ELE *prbs, unsigned int *buf, int pos, bool random_dq, bool need_check) {
    unsigned int i,j,k;
    unsigned int pattern;
    unsigned int bit_value;
    char *init_value = prbs->init_value;
    char *exp = prbs->expression;
    unsigned int init_cnt = prbs->init_cnt;
    unsigned int bit_len = (1 << init_cnt) -1;
    unsigned int exp_cnt = prbs->exp_cnt;
    unsigned int *p1;
    unsigned int tmp;
    unsigned int dq_cnt = 32;
    bool fixed_range = false;
    ulong ddr_density = get_ddr_density();

    printf("prbs name: %s\n", prbs->name);
    printf("prbs bit_len: %d\n", bit_len);
    printf("prbs init_cnt: %d\n", init_cnt);
    printf("prbs exp_cnt: %d\n", exp_cnt);

    if (strcmp(prbs->name, "prbs_31") == 0)
        fixed_range = true; // not enough space for prbs_31 incr range mode

    //Queue* queue = NULL;
    Queue* (queue[dq_cnt]);
    for (i = 0; i < dq_cnt; i++) {
        queue[i] = Queue_Init(OnQueueIncreasedEvent);
        if (queue[i] == NULL) {
            printf("ERROR: fail to Queue_Init queue[%d]\n", i);
            return -1;
        }
    }

    // fill queue with init_value
    for (i = 0; i < init_cnt; i++) {
        printf("prbs init_value[%d]: %d\n", init_cnt-i-1, init_value[init_cnt-i-1]);
        for (j = 0; j < dq_cnt; j++) {
            if ((j == pos) || (!random_dq)) {
                if (Queue_AddToHead(queue[j], init_value[init_cnt-i-1]) == -1)
                    printf("fail to Queue_AddToHead queue[%d], vect[%d]\n", j, i);
            }
            else {
                //ulong tmp = rand_ul() % 2;
                u64 tmp;
                riscv_get_time(&tmp);
                tmp = tmp % 2;
                if (Queue_AddToHead(queue[j], (tmp==0)?0:1) == -1)
                    printf("fail to Queue_AddToHead queue[%d], vect[%d]\n", j, i);
            }
        }
    }

    for (i = 0; i < dq_cnt; i++) {
        printf("prbs queue_value[%d]: [", i);
        for (j = 0; j < init_cnt; j++) {
            char data = Queue_QueryAt(queue[i], j);
            printf(" %d,", data);
        }
        printf("]\n");
    }

    //prbs31 sequence write
    p1 = buf;
    u64 bit_cnt = 0;
    for (i = 0; i < bit_len; i++) {
        bit_cnt++;
        if (fixed_range && ((bit_cnt % FIXED_RANGE) == 0))
            p1 = buf; // reset ptr to beginning
        else if ((bit_cnt % (ddr_density/4/2)) == 0)
            p1 = buf; // reset ptr to beginning
        bit_value = 0;
        for (j = 0; j < dq_cnt; j++) {
            unsigned int sum = 0;
            for (k = 0; k < exp_cnt; k++) {
                char data = Queue_QueryAt(queue[j], exp[k]-1);
                sum += data;
            }
	    sum = sum % 2;
	    Queue_AddToHead(queue[j], sum);
            bit_value |= Queue_GetFromTail(queue[j]) << j;
        }
        pattern = bit_value;
        //pattern = _generate_prbs_pos(pos, bit_value);
#ifdef DEBUG
        printf("bit[%d]: val<0x%x>, addr<0x%lx>, write pos pattern: 0x%x\n", bit_len-i-1, bit_value, (ulong)p1, pattern);
#endif
	*p1++ = pattern;
	// flush cache after write
	if(((ulong)p1 - (ulong)buf) % BYTE_PER_CACHELINE == 0)
	{
	    flush_dcache_range((ulong)p1-BYTE_PER_CACHELINE, (ulong)p1-1);
#ifdef DEBUG
            printf("flush cache start<0x%lx>, end<0x%lx>\n", (ulong)p1-BYTE_PER_CACHELINE, (ulong)p1-1);
#endif
	}
#if defined(DDR_PHY0_ONLY) || defined(DDR_PHY1_ONLY)
	if(((ulong)p1 - (ulong)buf) % (BYTE_PER_INTERLEAVE/2) == 0)
	{
	    // jump to next PHY1's interleave space
	    p1+=BYTE_PER_INTERLEAVE/2/sizeof(pattern);
	}
#endif
        
    }
    for (i = 0; i < bit_len; i++) {
        bit_cnt++;
        if (fixed_range && ((bit_cnt % FIXED_RANGE) == 0))
            p1 = buf; // reset ptr to beginning
        else if ((bit_cnt % (ddr_density/4/2)) == 0)
            p1 = buf; // reset ptr to beginning
        bit_value = 0;
        for (j = 0; j < dq_cnt; j++) {
            unsigned int sum = 0;
            for (k = 0; k < exp_cnt; k++) {
                char data = Queue_QueryAt(queue[j], exp[k]-1);
                sum += data;
            }
	    sum = sum % 2;
	    Queue_AddToHead(queue[j], sum);
            bit_value |= Queue_GetFromTail(queue[j]) << j;
        }
        pattern = _generate_prbs_neg(pos, bit_value);
#ifdef DEBUG
        printf("bit[%d]: val<0x%x>, addr<0x%lx>, write neg pattern: 0x%x\n", bit_len-i-1, bit_value, (ulong)p1, pattern);
#endif
        *p1++ = pattern;
	// flush cache after write
	if(((ulong)p1 - (ulong)buf) % BYTE_PER_CACHELINE == 0)
	{
	    flush_dcache_range((ulong)p1-BYTE_PER_CACHELINE, (ulong)p1-1);
#ifdef DEBUG
            printf("flush cache start<0x%lx>, end<0x%lx>\n", (ulong)p1-BYTE_PER_CACHELINE, (ulong)p1-1);
#endif
	}
#if defined(DDR_PHY0_ONLY) || defined(DDR_PHY1_ONLY)
	if(((ulong)p1 - (ulong)buf) % (BYTE_PER_INTERLEAVE/2) == 0)
	{
	    // jump to next PHY1's interleave space
	    p1+=BYTE_PER_INTERLEAVE/2/sizeof(pattern);
	}
#endif
        
    }
    if((ulong)p1 % BYTE_PER_CACHELINE != 0)
    {
        flush_dcache_range((ulong)p1&(~(BYTE_PER_CACHELINE-1)), (ulong)p1-1);
#ifdef DEBUG
        printf("flush cache start<0x%lx>, end<0x%lx>\n", (ulong)p1&(~(BYTE_PER_CACHELINE-1)), (ulong)p1-1);
#endif
    }

    if (!need_check)
        goto ret;

    // compare result
    // invalid cache before read
    mdelay(100);
    invalid_dcache_range((ulong)buf, (ulong)buf+(bit_len*4*2*2));
    p1 = buf;
    bit_cnt = 0;
    for (i = 0; i < bit_len; i++) {
        bit_cnt++;
        if (fixed_range && ((bit_cnt % FIXED_RANGE) == 0))
            p1 = buf; // reset ptr to beginning
        else if ((bit_cnt % (ddr_density/4/2)) == 0)
            p1 = buf; // reset ptr to beginning
        bit_value = 0;
        for (j = 0; j < dq_cnt; j++) {
            unsigned int sum = 0;
            for (k = 0; k < exp_cnt; k++) {
                char data = Queue_QueryAt(queue[j], exp[k]-1);
                sum += data;
            }
	    sum = sum % 2;
	    Queue_AddToHead(queue[j], sum);
            bit_value |= Queue_GetFromTail(queue[j]) << j;
        }
        pattern = bit_value;
        //pattern = _generate_prbs_pos(pos, bit_value);
	tmp = *p1;
        if (tmp != pattern) {
	    printf("prbs test failure at addr<0x%lx>, expect<0x%x>, actual<0x%x>\n", (ulong)p1, pattern, tmp);
	}
	p1++;
#if defined(DDR_PHY0_ONLY) || defined(DDR_PHY1_ONLY)
	if(((ulong)p1 - (ulong)buf) % (BYTE_PER_INTERLEAVE/2) == 0)
	{
	    // jump to next PHY1's interleave space
	    p1+=BYTE_PER_INTERLEAVE/2/sizeof(pattern);
	}
#endif
    }
    for (i = 0; i < bit_len; i++) {
        bit_cnt++;
        if (fixed_range && ((bit_cnt % FIXED_RANGE) == 0))
            p1 = buf; // reset ptr to beginning
        else if ((bit_cnt % (ddr_density/4/2)) == 0)
            p1 = buf; // reset ptr to beginning
        bit_value = 0;
        for (j = 0; j < dq_cnt; j++) {
            unsigned int sum = 0;
            for (k = 0; k < exp_cnt; k++) {
                char data = Queue_QueryAt(queue[j], exp[k]-1);
                sum += data;
            }
	    sum = sum % 2;
	    Queue_AddToHead(queue[j], sum);
            bit_value |= Queue_GetFromTail(queue[j]) << j;
        }
        pattern = _generate_prbs_neg(pos, bit_value);
	tmp = *p1;
        if (tmp != pattern) {
	    printf("prbs test failure at addr<0x%lx>, expect<0x%x>, actual<0x%x>\n", (ulong)p1, pattern, tmp);
	}
	p1++;
#if defined(DDR_PHY0_ONLY) || defined(DDR_PHY1_ONLY)
	if(((ulong)p1 - (ulong)buf) % (BYTE_PER_INTERLEAVE/2) == 0)
	{
	    // jump to next PHY1's interleave space
	    p1+=BYTE_PER_INTERLEAVE/2/sizeof(pattern);
	}
#endif
    }

ret:
    for (i = 0; i < dq_cnt; i++)
        Queue_Free(queue[i], true);
    return 0;
}

static int do_prbs7_sequence_test(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	unsigned int *buf, *p1;
	ulong addr = 0;
	ulong pos = 0;
	ulong iter = 1;
	ulong random_dq = 1;
	ulong i;
	ulong range = (1 << 7) *4 *2 *MULTIPLY; // occupys 4byte per bit, positive & negative, PHY0 & PHY1 space

	if (argc != 4 && argc != 5) {
		printf("should be 4 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &pos) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[3], 16, &iter) < 0)
		return CMD_RET_USAGE;
        if ((argc > 4) && strict_strtoul(argv[4], 16, &random_dq) < 0)
		return CMD_RET_USAGE;
	if ((addr < CONFIG_SYS_MEMTEST_START) || (addr > CONFIG_SYS_MEMTEST_END)) {
		printf("Refusing to do invalid region test 0x%lx\n", addr);
		//return -1;
	}
	if (pos >= 32) {
		printf("Refusing to do invalid bit position %ld\n", pos);
		return -1;
	}
	addr = addr & (~(BYTE_PER_INTERLEAVE - 1)); //addr align with ddr interleave(cacheline)
#if defined(CONFIG_DDR_CHBIT_256B) && defined(DDR_PHY1_ONLY)
        addr += BYTE_PER_INTERLEAVE/2; // only access PHY1
#endif
	printf("PRBS7 test: addr=0x%lx, pos=%d, iter=%d\n",
			addr, (int)pos, (int)iter);

	buf = map_sysmem(addr, range);
	p1 = buf;

        //clear buf and assure all buf allocated into cache entry
        for(i = 0; i < range; i+=4) {
	    *p1++ = 0;
        }
        mdelay(100);
	for(i=0; i<iter; i++)
	{
	    printf("prbs7 test itration<%ld>\n", i);
	    riscv_get_time(&t_begin);
	    prbs_test(&prbs7, buf, (int)pos, (random_dq!=0), checked);
	    riscv_get_time(&t_end);
	    printf("it spends %lld ms to do prbs7 sequence\n", (t_end - t_begin)/TIMER_TICK_MS);
	}

	return ret;
}

static int do_prbs15_sequence_test(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	unsigned int *buf, *p1;
	ulong addr = 0;
	ulong pos = 0;
	ulong iter = 1;
	ulong random_dq = 1;
	ulong i;
	ulong range = (1 << 15) *4 *2 *MULTIPLY; // occupys 4byte per bit, positive & negative, PHY0 & PHY1 space

	if (argc != 4 && argc != 5) {
		printf("should be 4 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &pos) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[3], 16, &iter) < 0)
		return CMD_RET_USAGE;
        if ((argc > 4) && strict_strtoul(argv[4], 16, &random_dq) < 0)
		return CMD_RET_USAGE;
	if ((addr < CONFIG_SYS_MEMTEST_START) || (addr > CONFIG_SYS_MEMTEST_END)) {
		printf("Refusing to do invalid region test 0x%lx\n", addr);
		//return -1;
	}
	if (pos >= 32) {
		printf("Refusing to do invalid bit position %ld\n", pos);
		return -1;
	}
	addr = addr & (~(BYTE_PER_INTERLEAVE - 1)); //addr align with ddr interleave(cacheline)
#if defined(CONFIG_DDR_CHBIT_256B) && defined(DDR_PHY1_ONLY)
        addr += BYTE_PER_INTERLEAVE/2; // only access PHY1
#endif
	printf("PRBS15 test: addr=0x%lx, pos=%d, iter=%d\n",
			addr, (int)pos, (int)iter);

	buf = map_sysmem(addr, range);
	p1 = buf;

        //clear buf and assure all buf allocated into cache entry
        for(i = 0; i < range; i+=4) {
	    *p1++ = 0;
        }
        mdelay(100);
	for(i=0; i<iter; i++)
	{
	    printf("prbs15 test itration<%ld>\n", i);
	    riscv_get_time(&t_begin);
	    prbs_test(&prbs15, buf, (int)pos, (random_dq!=0), checked);
	    riscv_get_time(&t_end);
	    printf("it spends %lld ms to do prbs15 sequence\n", (t_end - t_begin)/TIMER_TICK_MS);
	}

	return ret;
}
static int do_prbs23_sequence_test(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	unsigned int *buf, *p1;
	ulong addr = 0;
	ulong pos = 0;
	ulong iter = 1;
	ulong random_dq = 1;
	ulong i;
	ulong range = (1 << 23) *4 *2 *MULTIPLY; // occupys 4byte per bit, positive & negative, PHY0 & PHY1 space

	if (argc != 4 && argc != 5) {
		printf("should be 4 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &pos) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[3], 16, &iter) < 0)
		return CMD_RET_USAGE;
        if ((argc > 4) && strict_strtoul(argv[4], 16, &random_dq) < 0)
		return CMD_RET_USAGE;
	if ((addr < CONFIG_SYS_MEMTEST_START) || (addr > CONFIG_SYS_MEMTEST_END)) {
		printf("Refusing to do invalid region test 0x%lx\n", addr);
		//return -1;
	}
	if (pos >= 32) {
		printf("Refusing to do invalid bit position %ld\n", pos);
		return -1;
	}
	addr = addr & (~(BYTE_PER_INTERLEAVE - 1)); //addr align with ddr interleave(cacheline)
#if defined(CONFIG_DDR_CHBIT_256B) && defined(DDR_PHY1_ONLY)
        addr += BYTE_PER_INTERLEAVE/2; // only access PHY1
#endif
	printf("PRBS23 test: addr=0x%lx, pos=%d, iter=%d\n",
			addr, (int)pos, (int)iter);

	buf = map_sysmem(addr, range);
	p1 = buf;

        //clear buf and assure all buf allocated into cache entry
        for(i = 0; i < range; i+=4) {
	    *p1++ = 0;
        }
        mdelay(100);
	for(i=0; i<iter; i++)
	{
	    printf("prbs23 test itration<%ld>\n", i);
	    riscv_get_time(&t_begin);
	    prbs_test(&prbs23, buf, (int)pos, (random_dq!=0), checked);
	    riscv_get_time(&t_end);
	    printf("it spends %lld ms to do prbs23 sequence\n", (t_end - t_begin)/TIMER_TICK_MS);
	}

	return ret;
}
static int do_prbs31_sequence_test(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	unsigned int *buf, *p1;
	ulong addr = 0;
	ulong pos = 0;
	ulong iter = 1;
	ulong random_dq = 1;
	ulong i;
	ulong ddr_density = get_ddr_density();
	ulong range = (ulong)(1 << 31) *4; // occupys 4byte per bit, positive & negative, PHY0 & PHY1 space
	// too large to store prbs31 patterns in DDR space, mmap 4GB and wrap write is allowed
	if (range > ddr_density)
		range = ddr_density;

	range = FIXED_RANGE; // FIXED_RANGE for prbs_31

	if (argc != 4 && argc != 5) {
		printf("should be 4 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &pos) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[3], 16, &iter) < 0)
		return CMD_RET_USAGE;
	
        if ((argc > 4) && strict_strtoul(argv[4], 16, &random_dq) < 0)
		return CMD_RET_USAGE;
	if ((addr < CONFIG_SYS_MEMTEST_START) || (addr > CONFIG_SYS_MEMTEST_END)) {
		printf("Refusing to do invalid region test 0x%lx\n", addr);
		//return -1;
	}
	if (pos >= 32) {
		printf("Refusing to do invalid bit position %ld\n", pos);
		return -1;
	}
	addr = addr & (~(BYTE_PER_INTERLEAVE - 1)); //addr align with ddr interleave(cacheline)
#if defined(CONFIG_DDR_CHBIT_256B) && defined(DDR_PHY1_ONLY)
        addr += BYTE_PER_INTERLEAVE/2; // only access PHY1
#endif
	printf("PRBS31 test: addr=0x%lx, pos=%d, iter=%d\n",
			addr, (int)pos, (int)iter);

	buf = map_sysmem(addr, range);
	p1 = buf;

        //clear buf and assure all buf allocated into cache entry
        for(i = 0; i < range; i+=4) {
	    *p1++ = 0;
        }
        mdelay(100);
	for(i=0; i<iter; i++)
	{
	    printf("prbs31 test itration<%ld>\n", i);
	    riscv_get_time(&t_begin);
	    prbs_test(&prbs31, buf, (int)pos, (random_dq!=0), false); // too large to check result
	    riscv_get_time(&t_end);
	    printf("it spends %lld ms to do prbs31 sequence\n", (t_end - t_begin)/TIMER_TICK_MS);
	}

	return ret;
}

U_BOOT_CMD(
	prbs_7,	5,	1,	do_prbs7_sequence_test,
	"PRBS_7 sequence test",
	"[addr bit [iterations [is_random_dq]]]"
);
U_BOOT_CMD(
	prbs_15,	5,	1,	do_prbs15_sequence_test,
	"PRBS_15 sequence test",
	"[addr bit [iterations [is_random_dq]]]"
);
U_BOOT_CMD(
	prbs_23,	5,	1,	do_prbs23_sequence_test,
	"PRBS_23 sequence test",
	"[addr bit [iterations [is_random_dq]]]"
);
U_BOOT_CMD(
	prbs_31,	5,	1,	do_prbs31_sequence_test,
	"PRBS_31 sequence test",
	"[addr bit [iterations [is_random_dq]]]"
);

