// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright(C) 2021 Alibaba Communications Inc.
 * Author: David Li <liyong.li@alibaba-inc.com>
 */
/*
 * Memory Bits Scan Functions
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

#define ONE 0x00000001L
#define rand() get_timer(0)*100000
#define rand32() ((unsigned int) rand() | ( (unsigned int) rand() << 16))
#if (ULONG_MAX == 4294967295UL)
    #define rand_ul() rand32()
    #define UL_ONEBITS 0xffffffff
    #define UL_LEN 32
    #define CHECKERBOARD1 0x55555555
    #define CHECKERBOARD2 0xaaaaaaaa
    #define UL_BYTE(x) ((x | x << 8 | x << 16 | x << 24))
#elif (ULONG_MAX == 18446744073709551615ULL)
    #define rand64() (((ul) rand32()) << 32 | ((ul) rand32()))
    #define rand_ul() rand64()
    #define ul ulong
    #define UL_ONEBITS 0xffffffffffffffffUL
    #define UL_LEN 64
    #define CHECKERBOARD1 0x5555555555555555
    #define CHECKERBOARD2 0xaaaaaaaaaaaaaaaa
    #define UL_BYTE(x) (((ul)x | (ul)x<<8 | (ul)x<<16 | (ul)x<<24 | (ul)x<<32 | (ul)x<<40 | (ul)x<<48 | (ul)x<<56))
#else
    #error long on this platform is not 32 or 64 bits
#endif

#define DISPLAY_COUNT 1000000

#define BYTE_PER_CACHELINE 64
#define BIT_PER_BYTE 8

typedef unsigned long volatile ulv;
int use_phys = 1;
off_t physaddrbase = 0;

extern unsigned long get_ddr_density(void);
extern char* get_ddr_type(void);
extern void lp4_mrw(int addr, int wdata, int dch, int rank);
extern int lp4_mrr(int addr, int dch, int rank);
extern int lp4_set_ron(int ron);
extern void ddr_sysreg_wr(unsigned long int addr,unsigned int wr_data);
extern void ddr_phy0_reg_wr(unsigned long int addr,unsigned int wr_data);
extern void ddr_phy1_reg_wr(unsigned long int addr,unsigned int wr_data);
extern unsigned int ddr_sysreg_rd(unsigned long int addr);
extern unsigned int ddr_phy0_reg_rd(unsigned long int addr);
extern unsigned int ddr_phy1_reg_rd(unsigned long int addr);
#ifdef CONFIG_CMD_MEMTEST
extern ulong mem_test_alt(vu_long *buf, ulong start_addr, ulong end_addr,
			  vu_long *dummy);
extern ulong mem_test_quick(vu_long *buf, ulong start_addr, ulong end_addr,
			    vu_long pattern, int iteration);
#endif
extern void flush_dcache_range(unsigned long start, unsigned long end);
extern void invalidate_dcache_range(unsigned long start, unsigned long end);
extern void invalid_dcache_range(unsigned long start, unsigned long end);

#ifdef CONFIG_CMD_MEMTEST
int test_stuck_address(ulv *bufa, ulong count);
int test_random_value(ulv *bufa, ulv *bufb, ulong count);
int test_xor_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_sub_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_mul_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_div_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_or_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_and_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_seqinc_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_solidbits_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_blockseq_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_checkerboard_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_bitspread_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_bitflip_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_walkbits0_comparison(ulv *bufa, ulv *bufb, ulong count);
int test_walkbits1_comparison(ulv *bufa, ulv *bufb, ulong count);

enum test_index {
    IDX_STUCK = 0,
    IDX_RANDOM,
    IDX_XOR,
    IDX_SUB,
    IDX_MUL,
    IDX_DIV,
    IDX_OR,
    IDX_AND,
    IDX_SEQINC,
    IDX_SOLIDBITS,
    IDX_BLOCKSEQ,
    IDX_CHECKBOARD,
    IDX_BITSPREAD,
    IDX_BITFLIP,
    IDX_WALKONE,
    IDX_WALKZERO,
    IDX_MTEST = 20,
    IDX_MTESTALT,
};
#endif

static inline void _l2cache_ciall(void)
{
    asm volatile (".long 0x0170000b");
}
static inline void _l2cache_iall(void)
{
    asm volatile (".long 0x0160000b");
}
            
static int print_heartbeat(ulong cnt)
{
    if(cnt % DISPLAY_COUNT == 0)
    {
        printf("\r> \r");
    }
    else if(cnt % DISPLAY_COUNT == DISPLAY_COUNT/2)
    {
        printf("\r>>\r");
    }
    return 0;
}

#ifdef CONFIG_CMD_MEMTEST
int compare_regions(ulv *bufa, ulv *bufb, ulong count) {
    int r = 0;
    ulong i;
    ulong cnt;
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    off_t physaddr;

    cnt = 0;
    for (i = 0; i < count; i++, p1++, p2++) {
        if (*p1 != *p2) {
            if (use_phys) {
                physaddr = physaddrbase + (i * sizeof(ulong));
                fprintf(stderr,
                        "FAILURE: 0x%08lx != 0x%08lx at physical address "
                        "0x%08lx.\n",
                        (ulong) *p1, (ulong) *p2, physaddr);
            } else {
                fprintf(stderr,
                        "FAILURE: 0x%08lx != 0x%08lx at offset 0x%08lx.\n",
                        (ulong) *p1, (ulong) *p2, (ulong) (i * sizeof(ulong)));
            }
            r = -1;
        }
        print_heartbeat(cnt++);
    }
    return r;
}
static int do_bits_test(int argc, char * const argv[], enum test_index cmd)
{
	ulong start, end, count, size;
	ulv *bufa, *bufb;
	ulong iteration_limit = 0;
	ulong pattern = 0;
	int ret;
	ulong errs = 0;	/* number of errors, or -1 if interrupted */
	int iteration;

	start = CONFIG_SYS_MEMTEST_START;
	end = CONFIG_SYS_MEMTEST_START + 0x100000;

	if (argc > 1)
		if (strict_strtoul(argv[1], 16, &start) < 0)
			return CMD_RET_USAGE;
	if (argc > 2)
		if (strict_strtoul(argv[2], 16, &end) < 0)
			return CMD_RET_USAGE;
	if (argc > 3)
		if (strict_strtoul(argv[3], 16, &pattern) < 0)
			return CMD_RET_USAGE;
	if (argc > 4)
		if (strict_strtoul(argv[4], 16, &iteration_limit) < 0)
			return CMD_RET_USAGE;
	
	if ((start < CONFIG_SYS_MEMTEST_START) || (end > CONFIG_SYS_MEMTEST_END)) {
		printf("Refusing to do invalid region test\n");
		//return -1;
	}

	printf("Testing [0x%9lx ... 0x%9lx]\n", start, end);
	size = end-start+1;
	bufa = map_sysmem(start, size/2);
	bufb = map_sysmem(start+size/2, size/2);
	count = size / 2 / sizeof(ulv);
	for (iteration = 0;
			!iteration_limit || iteration < iteration_limit;
			iteration++) {
		if (ctrlc()) {
			errs = -1UL;
			break;
		}

		printf("Iteration: %6d\r", iteration + 1);
		debug("\n");
		switch(cmd) {
		case IDX_STUCK:
			printf("begin to stuck test\n");
			errs = test_stuck_address(bufa, count);
			break;
		case IDX_RANDOM:
			printf("begin to random test\n");
			errs = test_random_value(bufa, bufb, count);
			break;
		case IDX_XOR:
			printf("begin to xor test\n");
			errs = test_xor_comparison(bufa, bufb, count);
			break;
		case IDX_SUB:
			printf("begin to sub test\n");
			errs = test_sub_comparison(bufa, bufb, count);
			break;
		case IDX_MUL:
			printf("begin to mul test\n");
			errs = test_mul_comparison(bufa, bufb, count);
			break;
		case IDX_DIV:
			printf("begin to div test\n");
			errs = test_div_comparison(bufa, bufb, count);
			break;
		case IDX_OR:
			printf("begin to or test\n");
			errs = test_or_comparison(bufa, bufb, count);
			break;
		case IDX_AND:
			printf("begin to and test\n");
			errs = test_and_comparison(bufa, bufb, count);
			break;
		case IDX_SEQINC:
			printf("begin to seqinc test\n");
			errs = test_seqinc_comparison(bufa, bufb, count);
			break;
		case IDX_SOLIDBITS:
			printf("begin to solidbits test\n");
			errs = test_solidbits_comparison(bufa, bufb, count);
			break;
		case IDX_BLOCKSEQ:
			printf("begin to blockseq test\n");
			errs = test_blockseq_comparison(bufa, bufb, count);
			break;
		case IDX_WALKZERO:
			printf("begin to walkbits0 test\n");
			errs = test_walkbits0_comparison(bufa, bufb, count);
			break;
		case IDX_WALKONE:
			printf("begin to walkbits1 test\n");
			errs = test_walkbits1_comparison(bufa, bufb, count);
			break;
		case IDX_BITSPREAD:
			printf("begin to bitspread test\n");
			errs = test_bitspread_comparison(bufa, bufb, count);
			break;
		case IDX_BITFLIP:
			printf("begin to bitflip test\n");
			errs = test_bitflip_comparison(bufa, bufb, count);
			break;
		case IDX_CHECKBOARD:
			printf("begin to checkerboard test\n");
			errs = test_checkerboard_comparison(bufa, bufb, count);
			break;
		case IDX_MTESTALT:
			printf("begin to mtest_alt test\n");
			errs = mem_test_alt(bufa, start, start+size/2, bufb);
			break;
		case IDX_MTEST:
			printf("begin to mtest test\n");
			errs = mem_test_quick(bufa, start, end, pattern, 10);
			break;
		default:
			printf("default: begin to bitspread test\n");
			errs = test_bitspread_comparison(bufa, bufb, count);
			break;

		}

		if (errs == -1)
			break;
	}

	{
		void *vbufa = (void *)bufa;
		void *vbufb = (void *)bufb;

		unmap_sysmem(vbufa);
		unmap_sysmem(vbufb);
	}

	if (errs == -1) {
		/* Memory test was aborted - write a newline to finish off */
		putc('\n');
		ret = 1;
	} else {
		printf("Tested %d iteration(s) with %lu errors.\n",
			iteration, errs);
		ret = errs != 0;
	}

	return ret;
}

int test_stuck_address(ulv *bufa, ulong count) {
    ulv *p1 = bufa;
    unsigned int j;
    ulong i;
    off_t physaddr;

    for (j = 0; j < 16; j++) {
        p1 = (ulv *) bufa;
        for (i = 0; i < count; i++) {
            *p1 = ((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1);
            *p1++;
        }
        p1 = (ulv *) bufa;
        for (i = 0; i < count; i++, p1++) {
            if (*p1 != (((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1))) {
                if (use_phys) {
                    physaddr = physaddrbase + (i * sizeof(ul));
                    fprintf(stderr, 
                            "FAILURE: possible bad address line at physical "
                            "address 0x%08lx.\n", 
                            physaddr);
                } else {
                    fprintf(stderr, 
                            "FAILURE: possible bad address line at offset "
                            "0x%08lx.\n", 
                            (ul) (i * sizeof(ul)));
                }
                return -1;
            }
        }
    }
    return 0;
}

static int do_mem_stuck(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_STUCK);

	return ret;
}

int test_random_value(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = rand_ul();
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_random(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_RANDOM);

	return ret;
}

int test_xor_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ ^= q;
        *p2++ ^= q;
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_xor(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_XOR);

	return ret;
}

int test_sub_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ -= q;
        *p2++ -= q;
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_sub(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_SUB);

	return ret;
}

int test_mul_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ *= q;
        *p2++ *= q;
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_mul(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_MUL);

	return ret;
}

int test_div_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        if (!q) {
            q++;
        }
        *p1++ /= q;
        *p2++ /= q;
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_div(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_DIV);

	return ret;
}

int test_or_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ |= q;
        *p2++ |= q;
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_or(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_OR);

	return ret;
}

int test_and_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ &= q;
        *p2++ &= q;
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_and(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_AND);

	return ret;
}

int test_seqinc_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ulong i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = (i + q);
    }
    return compare_regions(bufa, bufb, count);
}

static int do_mem_seqinc(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_SEQINC);

	return ret;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ulong i;

    for (j = 0; j < 256; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (ul) UL_BYTE(j);
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    return 0;
}

static int do_mem_blockseq(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_BLOCKSEQ);

	return ret;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ulong i;
    ulong cnt;
    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
	cnt = 0;
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = ONE << j;
            } else { /* Walk it back down. */
                *p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
            }
            print_heartbeat(cnt++);
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    return 0;
}

static int do_mem_walkbits0(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_WALKZERO);

	return ret;
}


int test_walkbits1_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ulong i;
    ulong cnt;
    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        cnt = 0;
	for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
            } else { /* Walk it back down. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
            }
            print_heartbeat(cnt++);
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    return 0;
}
static int do_mem_walkbits1(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_WALKONE);

	return ret;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ulong i;
    ulong cnt;
    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        cnt = 0;
	for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << j) | (ONE << (j + 2))
                    : UL_ONEBITS ^ ((ONE << j)
                                    | (ONE << (j + 2)));
            } else { /* Walk it back down. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << (UL_LEN * 2 - 1 - j)) | (ONE << (UL_LEN * 2 + 1 - j))
                    : UL_ONEBITS ^ (ONE << (UL_LEN * 2 - 1 - j)
                                    | (ONE << (UL_LEN * 2 + 1 - j)));
            }
            print_heartbeat(cnt++);
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    return 0;
}
static int do_mem_bitspread(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_BITSPREAD);

	return ret;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j, k;
    ulong q;
    ulong i;
    ulong cnt;
    for (k = 0; k < UL_LEN; k++) {
        q = ONE << k;
	for (j = 0; j < 8; j++) {
            q = ~q;
            p1 = (ulv *) bufa;
            p2 = (ulv *) bufb;
            cnt = 0;
            for (i = 0; i < count; i++) {
                *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
                print_heartbeat(cnt++);
            }
            if (compare_regions(bufa, bufb, count)) {
                return -1;
            }
        }
    }
    return 0;
}

static int do_mem_bitflip(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	
	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_BITFLIP);

	return ret;
}

int test_solidbits_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ulong q;
    ulong i;
    ulong cnt;

    for (j = 0; j < 64; j++) {
        q = (j % 2) == 0 ? UL_ONEBITS : 0;
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        cnt=0;
    	for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            print_heartbeat(cnt++);
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    return 0;
}

static int do_mem_solidbits(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	
	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_SOLIDBITS);

	return ret;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, ulong count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ulong q;
    ulong cnt;
    ulong i;

    for (j = 0; j < 64; j++) {
        q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        cnt=0;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            print_heartbeat(cnt++);
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    return 0;
}

static int do_mem_checkerboard(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;

	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_CHECKBOARD);

	return ret;
}
static int do_mem_stress_test(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	printf("ddr stress test suite\n");
	
	_l2cache_ciall();
	ret = do_bits_test(argc, argv, IDX_MTEST);
	ret = do_bits_test(argc, argv, IDX_MTESTALT);
	ret = do_bits_test(argc, argv, IDX_STUCK);
	ret = do_bits_test(argc, argv, IDX_RANDOM);
	ret = do_bits_test(argc, argv, IDX_XOR);
	ret = do_bits_test(argc, argv, IDX_SUB);
	ret = do_bits_test(argc, argv, IDX_MUL);
	ret = do_bits_test(argc, argv, IDX_DIV);
	ret = do_bits_test(argc, argv, IDX_OR);
	ret = do_bits_test(argc, argv, IDX_AND);
	ret = do_bits_test(argc, argv, IDX_SEQINC);
	ret = do_bits_test(argc, argv, IDX_SOLIDBITS);
	ret = do_bits_test(argc, argv, IDX_BLOCKSEQ);
	ret = do_bits_test(argc, argv, IDX_CHECKBOARD);
	ret = do_bits_test(argc, argv, IDX_BITSPREAD);
	ret = do_bits_test(argc, argv, IDX_BITFLIP);
	ret = do_bits_test(argc, argv, IDX_WALKONE);
	ret = do_bits_test(argc, argv, IDX_WALKZERO);

	return ret;
}
#endif // CONFIG_CMD_MEMTEST


/*
 * 0x1 - Select rank 0 only
 * 0x2 - Select rank 1 only
 * 0x5 - Select rank 0 and 1
 */
static int get_device_mr(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	ulong dch = 0;
	ulong rank = 1;
	int data = -1;

	if (argc != 4) {
		printf("should be 3 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &dch) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[3], 16, &rank) < 0)
		return CMD_RET_USAGE;

	data = lp4_mrr((int)addr, (int)dch, (int)rank);

	printf("Get MR register: addr=%x, dch=%d, rank=%d, MR_vaule=0x%x\n",
			(int)addr, (int)dch, (int)rank, data);

	return ret;
}

/*
 * 0x1 - Select rank 0 only
 * 0x2 - Select rank 1 only
 * 0x5 - Select rank 0 and 1
 */
static int set_device_mr(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	ulong data = 0;
	ulong dch = 0;
	ulong rank = 1;

	if (argc != 5) {
		printf("should be 4 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &data) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[3], 16, &dch) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[4], 16, &rank) < 0)
		return CMD_RET_USAGE;

	printf("Set MR register: addr=%x, data=%x, dch=%d, rank=%d\n",
			(int)addr, (int)data, (int)dch, (int)rank);

	lp4_mrw((int)addr, (int)data, (int)dch, (int)rank);

	return ret;
}

static int set_device_ron(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong ron = 0;

	if (argc != 2) {
		printf("should be 1 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &ron) < 0)
		return CMD_RET_USAGE;

	printf("Set ron register: ron=%d\n",
			(int)ron);

	lp4_set_ron((int)ron);

	return ret;
}

static int do_ddr_sysreg_rd(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	unsigned int data = 0;

	if (argc != 2) {
		printf("should be 1 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;

	data = ddr_sysreg_rd(addr);

	printf("ddr_sysreg_rd: addr=%lx, data=%x\n", addr, data);

	return ret;
}
static int do_ddr_sysreg_wr(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	ulong data = 0;

	if (argc != 3) {
		printf("should be 2 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &data) < 0)
		return CMD_RET_USAGE;

	ddr_sysreg_wr(addr, (unsigned int)data);

	return ret;
}
static int do_ddr_phy0_reg_rd(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	unsigned int data = 0;

	if (argc != 2) {
		printf("should be 1 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;

	data = ddr_phy0_reg_rd(addr);

	printf("ddr_phy_reg_rd: addr=%lx, data=%x\n", addr, data);

	return ret;
}
static int do_ddr_phy0_reg_wr(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	ulong data = 0;

	if (argc != 3) {
		printf("should be 2 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &data) < 0)
		return CMD_RET_USAGE;

	ddr_phy0_reg_wr(addr, (unsigned int)data);

	return ret;
}
static int do_ddr_phy1_reg_rd(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	unsigned int data = 0;

	if (argc != 2) {
		printf("should be 1 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;

	data = ddr_phy1_reg_rd(addr);

	printf("ddr_phy1_reg_rd: addr=%lx, data=%x\n", addr, data);

	return ret;
}
static int do_ddr_phy1_reg_wr(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	int ret = 0;
	ulong addr = 0;
	ulong data = 0;

	if (argc != 3) {
		printf("should be 2 input parameters!!!");
		return CMD_RET_USAGE;
	}
	if (strict_strtoul(argv[1], 16, &addr) < 0)
		return CMD_RET_USAGE;
	if (strict_strtoul(argv[2], 16, &data) < 0)
		return CMD_RET_USAGE;

	ddr_phy1_reg_wr(addr, (unsigned int)data);

	return ret;
}

/**
  Clear&invalid L2cache
*/
static int do_l2cache_ciall(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
    _l2cache_ciall();
    return 0;
}

#ifdef CONFIG_CMD_MEMTEST
U_BOOT_CMD(
	stuck_address,	5,	1,	do_mem_stuck,
	"Stuck Address test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	random_value,	5,	1,	do_mem_random,
	"Random Value test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	compare_xor,	5,	1,	do_mem_xor,
	"Compare XOR test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	compare_sub,	5,	1,	do_mem_sub,
	"Compare SUB test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	compare_mul,	5,	1,	do_mem_mul,
	"Compare MUL test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	compare_div,	5,	1,	do_mem_div,
	"Compare DIV test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	compare_or,	5,	1,	do_mem_or,
	"Compare OR test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	compare_and,	5,	1,	do_mem_and,
	"Compare AND test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	seqinc,	5,	1,	do_mem_seqinc,
	"Sequential Increment test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	solidbits,	5,	1,	do_mem_solidbits,
	"Solid bits test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	blockseq,	5,	1,	do_mem_blockseq,
	"Block Sequential test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	checkerboard,	5,	1,	do_mem_checkerboard,
	"Checker Board test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	bitspread,	5,	1,	do_mem_bitspread,
	"Bit Spread test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	bitflip,	5,	1,	do_mem_bitflip,
	"Bit Flip test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	walkbits1,	5,	1,	do_mem_walkbits1,
	"Walking Ones test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	walkbits0,	5,	1,	do_mem_walkbits0,
	"Walking Zeroes test",
	"[start end [pattern [iterations]]]"
);
U_BOOT_CMD(
	stress_test,	5,	1,	do_mem_stress_test,
	"SolidBits, BitSpread, BitFlip, WalkBits Zero, WalkBits One, CheckerBoard, mtest_alt, mtest",
	"[start end [pattern [iterations]]]"
);
#endif // CONFIG_CMD_MEMTEST
U_BOOT_CMD(
	setmr,	5,	1,	set_device_mr,
	"set ddr MR register",
	"addr data dch(phy id) rank"
);
U_BOOT_CMD(
	getmr,	4,	1,	get_device_mr,
	"get ddr MR register",
	"addr dch(phy id) rank"
);
U_BOOT_CMD(
	setron,	2,	1,	set_device_ron,
	"set ddr ron register",
	"ron"
);
U_BOOT_CMD(
	ddr_sysreg_rd,	2,	1,	do_ddr_sysreg_rd,
	"read ddr sys reg",
	"addr"
);
U_BOOT_CMD(
	ddr_sysreg_wr,	3,	1,	do_ddr_sysreg_wr,
	"write ddr sys reg",
	"addr wdata"
);
U_BOOT_CMD(
	ddr_phy0_reg_rd,	2,	1,	do_ddr_phy0_reg_rd,
	"read ddr phy0 reg",
	"addr"
);
U_BOOT_CMD(
	ddr_phy0_reg_wr,	3,	1,	do_ddr_phy0_reg_wr,
	"write ddr phy0 reg",
	"addr wdata"
);
U_BOOT_CMD(
	ddr_phy1_reg_rd,	2,	1,	do_ddr_phy1_reg_rd,
	"read ddr phy1 reg",
	"addr"
);
U_BOOT_CMD(
	ddr_phy1_reg_wr,	3,	1,	do_ddr_phy1_reg_wr,
	"write ddr phy1 reg",
	"addr wdata"
);
U_BOOT_CMD(
	l2cache_ciall,	3,	1,	do_l2cache_ciall,
	"clear & invlid l2 cache",
	""
);


