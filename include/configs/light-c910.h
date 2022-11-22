/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CONFIG_SYS_SPL_MALLOC_START     0xffe0110000
#define CONFIG_SYS_SPL_MALLOC_SIZE      0x000000a000
#define CONFIG_SPL_STACK                0xffe011d000
#define CONFIG_SPL_BSS_START_ADDR       0xffe011d000
#define CONFIG_SPL_BSS_MAX_SIZE         0x0000002000

#define CONFIG_SYS_MONITOR_LEN                         (898 * 1024) /* Assumed U-Boot size */
#define CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION  1
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR        0x178

#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_TEXT_BASE + SZ_1M)
#define CONFIG_SYS_LOAD_ADDR        (CONFIG_SYS_TEXT_BASE + SZ_1M)
#define CONFIG_SYS_MALLOC_LEN       SZ_1M
#define CONFIG_SYS_BOOTM_LEN        SZ_64M
#define CONFIG_SYS_CACHELINE_SIZE   64

#define SRAM_BASE_ADDR	 0xffe0000000
#define PLIC_BASE_ADDR   0xffd8000000
#define PMP_BASE_ADDR    0xffdc020000

/* Network Configuration */
#define CONFIG_DW_ALTDESCRIPTOR
#define CONFIG_RGMII            1
#define CONFIG_PHY_MARVELL      1
#define CONFIG_NET_RETRY_COUNT  20

#define CONFIG_SYS_FLASH_BASE       0x0
#define CONFIG_SYS_MAX_FLASH_BANKS  1
#define CONFIG_ENV_ADDR             (CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV      0

#define CONFIG_SYS_SDRAM_BASE           0
#define MEMTEST_MAX_SIZE                0x200000000 /* 8GB DDR */
#define CONFIG_SYS_MEMTEST_START        0x00000000 // larger than Uboot end addr
#define CONFIG_SYS_MEMTEST_END          CONFIG_SYS_SDRAM_BASE + MEMTEST_MAX_SIZE
#define CONFIG_SYS_MEMTEST_SCRATCH      CONFIG_SYS_MEMTEST_END/2

/* SEC Configuration */
//#define LIGHT_ROOTFS_SEC_CHECK	1
#define SBI_ENTRY_ADDR			0x100000
#define AON_DDR_ADDR			0x80000
#define AON_SRAM_ADDR			0xffffef8000

/* USB fastboot non_sec configs */
#define THEAD_LIGHT_FASTBOOT	1
#define LIGHT_FW_ADDR		0x0
#define LIGHT_KERNEL_ADDR	0x200000
#define LIGHT_DTB_ADDR		0x1f00000
#define LIGHT_ROOTFS_ADDR	0x2000000
#define LIGHT_AON_FW_ADDR	0xffffef8000
#define LIGHT_TEE_FW_ADDR	0x1c000000
#define LIGHT_TF_FW_ADDR	LIGHT_FW_ADDR
#define LIGHT_TF_FW_TMP_ADDR	0x100000
#define LIGHT_KERNEL_ADDR_CMD	"0x200000"
#define LIGHT_DTB_ADDR_CMD	"0x1f00000"


/* trust image name string */
#define TF_IMG_UPD_NAME		"stashtf"
#define TEE_IMG_UPD_NAME	"stashtee"
#define UBOOT_IMG_UPD_NAME	"stashuboot"
#define TF_PART_NAME		"tf"
#define TEE_PART_NAME		"tee"
#define UBOOT_PART_NAME		"uboot"
#define STASH_PART_NAME 	"stash"
#define KERNEL_PART_NAME	"kernel"

#define UBOOT_STAGE_ADDR	SRAM_BASE_ADDR

/* Video configs */
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_32BPP

/* security upgrade flag */
#define TF_SEC_UPGRADE_FLAG	0x5555aaaa
#define TEE_SEC_UPGRADE_FLAG 0x5a5aa5a5
#define UBOOT_SEC_UPGRADE_FLAG	0xa5a5aa55

#define UBOOT_MAX_VER		64
#define CONFIG_SYS_CBSIZE 	512
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE /* Boot args buffer */
#define CONFIG_SYS_MAXARGS	64		/* max number of command args */
/* List of different env in debug/release version  */
#if defined (U_BUILD_DEBUG)
#define ENV_KERNEL_LOGLEVEL "kernel_loglevel=7\0"
#define ENV_STR_BOOT_DELAY  
#else
#define ENV_KERNEL_LOGLEVEL "kernel_loglevel=4\0"
#define ENV_STR_BOOT_DELAY	"bootdelay=0\0"
#endif

#if defined (CONFIG_LIGHT_SEC_BOOT_WITH_VERIFY_VAL_A)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffffffffffff\0" \
	"tf_addr=0x100000\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"tee_addr=0x1c000000\0" \
	"sec_upgrade_mode=0\0"\
	"mmcdev=0\0" \
	"mmcpart=6\0" \
	"kdump_buf=1M\0" \
	"uboot_version=0x0000000000000000\0"\
	"fdt_file=light-a-val-sec.dtb\0" \
	"uuid_rootfs=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=tf,size=50MiB,type=boot;name=tee,size=50MiB,type=boot;name=stash,size=50MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfs};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc 0:2 $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc 0:2 $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run load_aon;run load_c906_audio; ext4load mmc 0:3 $tf_addr trust_firmware.bin; ext4load mmc 0:4 $tee_addr tee.bin;ext4load mmc 0:2 $dtb_addr ${fdt_file}; ext4load mmc 0:2 $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; secboot; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"

#elif defined (CONFIG_LIGHT_SEC_BOOT_WITH_VERIFY_VAL_B)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffffffffffff\0" \
	"tf_addr=0x100000\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"tee_addr=0x1c000000\0" \
	"sec_upgrade_mode=0\0"\
	"mmcdev=0\0" \
	"mmcpart=6\0" \
	"kdump_buf=1M\0" \
	"uboot_version=0x0000000000000000\0"\
	"fdt_file=light-b-product-sec.dtb\0" \
	"uuid_rootfs=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=tf,size=50MiB,type=boot;name=tee,size=50MiB,type=boot;name=stash,size=50MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfs};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc 0:2 $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc 0:2 $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run load_aon;run load_c906_audio; ext4load mmc 0:3 $tf_addr trust_firmware.bin; ext4load mmc 0:4 $tee_addr tee.bin;ext4load mmc 0:2 $dtb_addr ${fdt_file}; ext4load mmc 0:2 $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; secboot; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"

#elif defined (CONFIG_LIGHT_SEC_BOOT_WITH_VERIFY_ANT_REF)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffffffffffff\0" \
	"tf_addr=0x100000\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"tee_addr=0x1c000000\0" \
	"sec_upgrade_mode=0\0"\
	"mmcdev=0\0" \
	"mmcpart=6\0" \
	"kdump_buf=1M\0" \
	"uboot_version=0x0000000000000000\0"\
	"fdt_file=light-ant-ref-sec.dtb\0" \
	"uuid_rootfs=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=tf,size=50MiB,type=boot;name=tee,size=50MiB,type=boot;name=stash,size=50MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfs};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc 0:2 $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc 0:2 $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run load_aon;run load_c906_audio; ext4load mmc 0:3 $tf_addr trust_firmware.bin; ext4load mmc 0:4 $tee_addr tee.bin;ext4load mmc 0:2 $dtb_addr ${fdt_file}; ext4load mmc 0:2 $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; secboot; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"

#else
#if defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_A)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"mmcpart=3\0" \
	"kdump_buf=1M\0" \
	"fdt_file=light-a-product.dtb\0" \
	"uuid_rootfs=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfs};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc 0:2 $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc 0:2 $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run load_aon;run load_c906_audio; ext4load mmc 0:2 $opensbi_addr fw_dynamic.bin; ext4load mmc 0:2 $dtb_addr ${fdt_file}; ext4load mmc 0:2 $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_B)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	ENV_KERNEL_LOGLEVEL \
	"kdump_buf=1M\0" \
	ENV_STR_BOOT_DELAY \
	"findpart=rollback; if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-b-product.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=${kernel_loglevel} eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file};ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image;\0" \
	"bootcmd=run bootcmd_load; bootslave ; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_B_REF)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"findpart=if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-b-ref.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file};ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image;\0" \
	"bootcmd=run bootcmd_load; bootslave ; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_B_POWER)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"kdump_buf=1M\0" \
	"findpart=rollback; if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-b-power.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file}; ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_REF)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"kdump_buf=1M\0" \
	"findpart=rollback; if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-ant-ref.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file}; ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_VAL_ANT_DISCRETE)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"kdump_buf=1M\0" \
	"findpart=rollback; if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-ant-discrete.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file}; ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_BEAGLE)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"kdump_buf=500M\0" \
	"findpart=rollback; if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-beagle.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file}; ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#elif defined (CONFIG_TARGET_LIGHT_FM_C910_A_REF)
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"findpart=if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-a-ref.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file}; ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
        "\0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splashimage=0x30000000\0" \
	"splashpos=m,m\0" \
	"fdt_high=0xffffffffffffffff\0" \
	"opensbi_addr=0x0\0" \
	"dtb_addr=0x01f00000\0" \
	"kernel_addr=0x00200000\0" \
	"aon_ram_addr=0xffffef8000\0" \
	"audio_ram_addr=0xffc0000000\0" \
	"fwaddr=0x10000000\0"\
	"mmcdev=0\0" \
	"boot_partition=bootA\0" \
	"root_partition=rootfsA\0" \
	"kdump_buf=500M\0" \
	"findpart=rollback; if test ${boot_partition} = bootB; then mmcbootpart=4; else mmcbootpart=2; fi; if test ${root_partition} = rootfsB; then mmcpart=5; else mmcpart=3; fi;\0" \
	"fdt_file=light-a-val.dtb\0" \
	"uuid_rootfsA=80a5a8e9-c744-491a-93c1-4f4194fd690a\0" \
	"uuid_rootfsB=80a5a8e9-c744-491a-93c1-4f4194fd690b\0" \
	"partitions=name=table,size=2031KB;name=boot,size=200MiB,type=boot;name=root,size=4000MiB,type=linux,uuid=${uuid_rootfsA};name=bootB,size=200MiB,type=boot;name=rootB,size=4000MiB,type=linux,uuid=${uuid_rootfsB};name=data,size=-,type=linux\0" \
	"finduuid=part uuid mmc ${mmcdev}:${mmcpart} uuid\0" \
	"gpt_partition=gpt write mmc ${mmcdev} $partitions\0" \
	"set_bootargs=setenv bootargs console=ttyS0,115200 root=PARTUUID=${uuid} rootfstype=ext4 rdinit=/sbin/init rootwait rw earlycon clk_ignore_unused loglevel=7 eth=$ethaddr rootrw=PARTLABEL=data init=/init rootinit=/sbin/init rootrwoptions=rw,noatime rootrwreset=${factory_reset} crashkernel=${kdump_buf}\0" \
	"load_aon=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_aon_fpga.bin;cp.b $fwaddr $aon_ram_addr $filesize\0"\
	"load_c906_audio=ext4load mmc ${mmcdev}:${mmcbootpart} $fwaddr light_c906_audio.bin;cp.b $fwaddr $audio_ram_addr $filesize\0"\
	"bootcmd_load=run findpart;run load_aon;run load_c906_audio; ext4load mmc ${mmcdev}:${mmcbootpart} $opensbi_addr fw_dynamic.bin; ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file}; ext4load mmc ${mmcdev}:${mmcbootpart} $kernel_addr Image\0" \
	"bootcmd=run bootcmd_load; bootslave; run finduuid; run set_bootargs; booti $kernel_addr - $dtb_addr;\0" \
	"factory_reset=yes\0"\
        "\0"
#endif
#endif
#endif /* __CONFIG_H */
