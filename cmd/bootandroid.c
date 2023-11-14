
/*
 * (C) Copyright 2018, Linaro Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <avb_verify.h>
#include <command.h>
#include <env.h>
#include <image.h>
#include <malloc.h>
#include <mmc.h>
#include <android_image.h>
#include <android_bootloader_message.h>
#include <xbc.h>

#define ENV_KERNEL_ADDR  "kernel_addr"
#define ENV_RAMDISK_ADDR "ramdisk_addr"
#define ENV_DTB_ADDR     "dtb_addr"
#define DEFAULT_KERNEL_ADDR  0x00200800
#define DEFAULT_RAMDISK_ADDR LIGHT_ROOTFS_ADDR
#define DEFAULT_DTB_ADDR     LIGHT_DTB_ADDR
#define ENV_RAMDISK_SIZE "ramdisk_size"
#define MISC_PARTITION "misc"
#define RECOVERY_PARTITION "recovery"
#define BOOT_PARTITION "boot"
#define VENDOR_BOOT_PARTITION "vendor_boot"

#define BOOTDEV_DEFAULT		0
#define BCB_BOOTONCE	"bootonce-bootloader"
#define BCB_BOOTRECOVERY	"boot-recovery"


/*
 * Knowing secure boot is enable or disable dependents on 
 * special data field in efuse and efuse control register.
 */
extern bool get_system_boot_type(void);
/*
 * The suffix for partition name is from the value of ENV_BOOTAB
 */
static const char *slot_name_suffix = NULL;;

/*
 * BOOT IMAGE HEADER V3/V4 PAGESIZE
 * Source code:system/tools/mkbootimg/unpack_bootimg.py
 */
#define BOOT_IMAGE_HEADER_V3_PAGESIZE 4096

static struct AvbOps *avb_ops = NULL;
static struct bootloader_message_ab *s_bcb = NULL;
static struct bootloader_control *boot_ctl = NULL;

static char *get_boot_partition_name_suffix(void)
{
#ifdef CONFIG_ANDROID_AB
    if (boot_ctl != NULL) {
		/* index 0 is _a, index 1 is _b*/
		if(boot_ctl->slot_info[0].priority < boot_ctl->slot_info[1].priority) {
			strcpy(boot_ctl->slot_suffix, "_b");
		} else {
			strcpy(boot_ctl->slot_suffix, "_a");
		}
	} else {
		printf("get_slot_suffix boot_ctl is null return _a");
		return "_a";
	}
	printf("get_slot_suffix boot_ctl->slot_suffix %s\r\n", boot_ctl->slot_suffix);
	return boot_ctl->slot_suffix;
#else
	return "";
#endif
}

static void get_partition_name(const char *partion, char *partion_name)
{
	strcpy(partion_name, partion);
	strcat(partion_name, get_boot_partition_name_suffix());
}

/*
 *format 4 chars/bytes to a int number
 */
static int byteToInt(uint8_t* data,int offset)
{
	return data[offset+0] + (data[offset+1] << 8) + (data[offset+2] << 16) + (data[offset+3] << 24);
}

static int get_number_of_pages(int image_size, int page_size)
{
    return (image_size + page_size - 1) / page_size;
}

/**
 * header_version >=3,get dtb data from vendor_boot.img ,else boot.img.
 *
 * header_version = 4,get bootconfig data from vendor_boot.img ,
 * and append bootconfig to the end of ramdisk(initrd)
 * doc:https://www.kernel.org/doc/html/next/translations/zh_CN/admin-guide/bootconfig.html#initrd
 */
static int prepare_data_from_vendor_boot(struct andr_img_hdr *hdr, int dtb_start, uint8_t** buf_bootconfig, int* vendor_bootconfig_size)
{
	int ret;
	disk_partition_t part_info;
	uint8_t* vendor_boot_data = NULL;
	struct blk_desc *dev_desc  = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	char vb_part_name[32] = {0};

	if (hdr == NULL) {
		printf("invalid hdr\n");
		return -1;
	}

	/* if the vendor boot partition name is beyond 32B, arise error */
	if ((32 - strlen(VENDOR_BOOT_PARTITION)) < 2)
		return -1;
	
    get_partition_name(VENDOR_BOOT_PARTITION, vb_part_name);

	printf("blk_get_dev %s\n", vb_part_name);
	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		printf("MMC err: invalid mmc device\n");
		return -1;
	}
	/* Get boot partition info */
	ret = part_get_info_by_name(dev_desc, vb_part_name, &part_info);
	if (ret < 0) {
		printf("MMC err: cannot find %s partition\n", vb_part_name);
		return -1;
	}

    if (part_info.size * part_info.blksz > CONFIG_FASTBOOT_BUF_SIZE) {
        return -1;
    }
    vendor_boot_data = (uint8_t*)CONFIG_FASTBOOT_BUF_ADDR;

	ret = blk_dread(dev_desc, part_info.start, part_info.size, vendor_boot_data);
	// vendor_boot.img
	//* +------------------------+
	//* | vendor boot header     | o pages
	//* +------------------------+
	//* | vendor ramdisk section | p pages
	//* +------------------------+
	//* | dtb                    | q pages
	//* +------------------------+
	//* | vendor ramdisk table   | r pages
	//* +------------------------+
	//* | bootconfig             | s pages
	//* +------------------------+
	//* o = (2124 + page_size - 1) / page_size
	//* p = (vendor_ramdisk_size + page_size - 1) / page_size
	//* q = (dtb_size + page_size - 1) / page_size
	//* r = (vendor_ramdisk_table_size + page_size - 1) / page_size
	//* s = (vendor_bootconfig_size + page_size - 1) / page_size

	// see system/tools/mkbootimg/unpack_bootimg.py
	// info.boot_magic = unpack('8s', args.boot_img.read(8))[0].decode()
	// info.header_version = unpack('I', args.boot_img.read(4))[0]
	// info.page_size = unpack('I', args.boot_img.read(4))[0]
	// info.kernel_load_address = unpack('I', args.boot_img.read(4))[0]
	// info.ramdisk_load_address = unpack('I', args.boot_img.read(4))[0]
	// info.vendor_ramdisk_size = unpack('I', args.boot_img.read(4))[0]
	// info.cmdline = cstr(unpack('2048s', args.boot_img.read(2048))[0].decode())
	// info.tags_load_address = unpack('I', args.boot_img.read(4))[0]
	// info.product_name = cstr(unpack('16s', args.boot_img.read(16))[0].decode())
	// info.header_size = unpack('I', args.boot_img.read(4))[0]
	// info.dtb_size = unpack('I', args.boot_img.read(4))[0]
	// info.dtb_load_address = unpack('Q', args.boot_img.read(8))[0]
	// info.vendor_ramdisk_table_size = unpack('I', args.boot_img.read(4))[0]
	// vendor_ramdisk_table_entry_num = unpack('I', args.boot_img.read(4))[0]
	// vendor_ramdisk_table_entry_size = unpack('I', args.boot_img.read(4))[0]
	// info.vendor_bootconfig_size = unpack('I', args.boot_img.read(4))[0]
	// num_vendor_ramdisk_table_pages = get_number_of_pages(
	// 	info.vendor_ramdisk_table_size, page_size)
	// vendor_ramdisk_table_offset = page_size * (
	// 	num_boot_header_pages + num_boot_ramdisk_pages + num_boot_dtb_pages)
	// bootconfig_offset = page_size * (num_boot_header_pages
	// 	+ num_boot_ramdisk_pages + num_boot_dtb_pages
	// 	+ num_vendor_ramdisk_table_pages)

	int vendor_boot_pagesize = byteToInt(vendor_boot_data,12);//offset 12
	int vendor_ramdisk_size = byteToInt(vendor_boot_data,24);//offset 24
	int dtb_size = byteToInt(vendor_boot_data,2100);//offset 2100
	int o = (2124 + vendor_boot_pagesize - 1) / vendor_boot_pagesize;
	int p = (vendor_ramdisk_size + vendor_boot_pagesize - 1) / vendor_boot_pagesize;
	int dtb_offset = vendor_boot_pagesize * (o + p);

	hdr->dtb_size=  dtb_size;
	memcpy((void *)(uint64_t)dtb_start, vendor_boot_data + dtb_offset, hdr->dtb_size);

	int q=(hdr->dtb_size + vendor_boot_pagesize - 1) / vendor_boot_pagesize;
	int vendor_ramdisk_table_size=byteToInt(vendor_boot_data,2112);//offset 2112
	
	int r=(vendor_ramdisk_table_size + vendor_boot_pagesize - 1) / vendor_boot_pagesize;
	*vendor_bootconfig_size=byteToInt(vendor_boot_data,2124);//offset 2124
	
	*buf_bootconfig = avb_malloc(*vendor_bootconfig_size);
	if (*buf_bootconfig == NULL) {
		printf("vendor bootconfig malloc fail\n");
		return -1;
	}
	int bootconfig_offset=vendor_boot_pagesize * (o + p + q + r);
	memcpy(*buf_bootconfig, vendor_boot_data + bootconfig_offset, *vendor_bootconfig_size);

#ifdef CONFIG_ANDROID_AB
	char *find_str = NULL;
	char *slot_suffix = get_boot_partition_name_suffix();
    char *slot_suffx_pre = "androidboot.slot_suffix=";
	printf("prepare_data_from_vendor_boot slot_suffix:%s\n", slot_suffix);
    printf("prepare_data_from_vendor_boot slot_suffx_pre:%s\n", slot_suffx_pre);

	find_str = strstr((char *)*buf_bootconfig, slot_suffx_pre);
    if (find_str != NULL) {
		memcpy(find_str + strlen(slot_suffx_pre), slot_suffix, strlen(slot_suffix));
	}
#endif

	return 0;
}

static void prepare_loaded_parttion_data(const uint8_t* data)
{
	struct andr_img_hdr *hdr = (struct andr_img_hdr *)map_sysmem((phys_addr_t)data, 0);

	if (IMAGE_FORMAT_ANDROID == genimg_get_format(hdr)) {
		int dtb_start = env_get_hex(ENV_DTB_ADDR, DEFAULT_DTB_ADDR);
		uint8_t* buf_bootconfig = NULL;
		int size_bootconfig=0;

		printf("Boot image header_version:%d\n", hdr->header_version);
		if (hdr->header_version >= 3) {
			// see system/tools/mkbootimg/unpack_bootimg.py
			hdr->kernel_size = byteToInt((uint8_t *)data, 8);
			hdr->ramdisk_size = byteToInt((uint8_t *)data, 12);
			hdr->page_size = BOOT_IMAGE_HEADER_V3_PAGESIZE;
			prepare_data_from_vendor_boot(hdr,dtb_start,&buf_bootconfig,&size_bootconfig);
		}

		int kernel_start = env_get_hex(ENV_KERNEL_ADDR, DEFAULT_KERNEL_ADDR);
		int ramdisk_start = env_get_hex(ENV_RAMDISK_ADDR, DEFAULT_RAMDISK_ADDR);
		// see system/tools/mkbootimg/unpack_bootimg.py
 		int page_size = hdr->page_size;
		int num_header_pages = 1;
		int num_kernel_pages = get_number_of_pages(hdr->kernel_size, page_size);
		int num_ramdisk_pages = get_number_of_pages(hdr->ramdisk_size, page_size);
		int kernel_offset = page_size * num_header_pages;
		int ramdisk_offset = page_size * (num_header_pages + num_kernel_pages);
		int dtb_offset = page_size * (num_header_pages + num_kernel_pages + num_ramdisk_pages);

		printf("Boot image kernel_start:%x, kernel_offset:%x, kernel_size:%d\n", kernel_start, kernel_offset, hdr->kernel_size);
		printf("Boot image ramdisk_start:%x, ramdisk_offset:%x, ramdisk_size:%d\n", ramdisk_start, ramdisk_offset, hdr->ramdisk_size);
		printf("Boot image page_size:%d\n", hdr->page_size);
		printf("dtb_offset:%x, dtb_size:%d\n", dtb_offset, hdr->dtb_size);

 		if (kernel_start + hdr->kernel_size > ramdisk_start || kernel_start + hdr->kernel_size > dtb_start) {
			printf("boot.img kernel space and ramdis space are overlaped !!!\n");
		} else {
			memcpy((void *)(uint64_t)kernel_start, data + kernel_offset, hdr->kernel_size);
			memcpy((void *)(uint64_t)ramdisk_start, data + ramdisk_offset, hdr->ramdisk_size);
			if( hdr->header_version < 3) {
				//set ramdisk size for bootm
				env_set_hex(ENV_RAMDISK_SIZE, hdr->ramdisk_size);
				memcpy((void *)(uint64_t)dtb_start, data + dtb_offset, hdr->dtb_size);
			} else {
				//get bootconfig form vendor_boot.img and append bootconfig to ramdisk
				char* bootconfig_params=(char*)buf_bootconfig;
				int ret = addBootConfigParameters(bootconfig_params, size_bootconfig,
                            						ramdisk_start + hdr->ramdisk_size , 0);
				if (ret == -1) {
					printf("Bootconfig Err: add BootConfig Parameters error!!!\n");
				} else {
					printf("ramdisk size is updated to new value is:%d\n",hdr->ramdisk_size + ret);
					//set ramdisk size for bootm
					env_set_hex(ENV_RAMDISK_SIZE, hdr->ramdisk_size + ret);
				}
			}
		}
		if (buf_bootconfig != NULL) {
			avb_free(buf_bootconfig);
		}
	}
	unmap_sysmem(hdr);
}

static int prepare_boot_data(const AvbSlotVerifyData *out_data)
{
	int res = CMD_RET_FAILURE;
	int i = 0;
	int num_loaded_partition = out_data->num_loaded_partitions;

	printf("@@@@ prepare loaded partition (%d) data start\n", num_loaded_partition);
	for (i = 0; i < num_loaded_partition; i++) {
		const AvbPartitionData *loaded_partition = &out_data->loaded_partitions[i];
		
		if (loaded_partition->partition_name != NULL) {
			printf("partition_name=%s, data_size=%ld\n", \
					loaded_partition->partition_name, loaded_partition->data_size);
			prepare_loaded_parttion_data(loaded_partition->data);
		}
	}
	return res;
}

static void prepare_partition_data(const char *name)
{
	int ret = 0;
	disk_partition_t part_info;
	struct blk_desc *dev_desc  = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	uint8_t *data = NULL;

	printf("prepare_partition_data %s\n", name);
	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		printf("MMC err: invalid mmc device\n");
		return;
	}
	/* Get boot partition info */
	ret = part_get_info_by_name(dev_desc, name, &part_info);
	if (ret < 0) {
		printf("MMC err: cannot find %s partition\n", name);
		return;
	}

	data = avb_malloc(part_info.size * part_info.blksz);
	if (data == NULL) {
		printf("avb malloc(%ldKB) fails\n", part_info.size * part_info.blksz / 1024);
		return;
	}

	ret = blk_dread(dev_desc, part_info.start, part_info.size, data);
	prepare_loaded_parttion_data(data);
	
	printf("prepare_partition_data %s, read=%d, start:%lx, size:%ld, blksize:%lx\n", \
			name, ret, part_info.start, part_info.size, part_info.blksz);

	avb_free(data);
}

static void clear_bcb(void)
{
	int ret;
	disk_partition_t part_info;
	struct blk_desc *dev_desc  = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);

	//bcb clear and store
	memset(s_bcb, 0, sizeof(struct bootloader_message_ab));

	if (!dev_desc || dev_desc->type == DEV_TYPE_UNKNOWN) {
		printf("BootAndriod bcb err: invalid mmc device\n");
		return;
	}
	/* Get boot partition info */
	ret = part_get_info_by_name(dev_desc, MISC_PARTITION, &part_info);
	if (ret < 0) {
		printf("BootAndriod bcb err: cannot find misc partition\n");
		return;
	}

	ret = blk_dwrite(dev_desc, part_info.start, part_info.size, s_bcb);
	printf("BootAndriod bcb info :clear_bcb write=%d, %ld,%ld,%ld\n", ret, part_info.start, part_info.size, part_info.blksz);
}

static int do_andriod_bcb_business(void)
{
	AvbIOResult ret = AVB_IO_RESULT_OK;
	size_t bytes_read = 0;
	int res = CMD_RET_FAILURE;

    if (avb_ops != NULL) {
        avb_ops_free(avb_ops);
        avb_ops = NULL;
    }

    avb_ops = avb_ops_alloc(BOOTDEV_DEFAULT);
    if (avb_ops == NULL) {
        goto _bcb_err;
    }

    if (s_bcb != NULL) {
        avb_free(s_bcb);
        s_bcb = NULL;
    }

	s_bcb = avb_malloc(sizeof(struct bootloader_message_ab));
	if (s_bcb == NULL) {
		goto _bcb_err;
	}

    if (boot_ctl != NULL) {
        avb_free(boot_ctl);
        boot_ctl = NULL;
    }

    boot_ctl = malloc(sizeof(struct bootloader_control));
	if (boot_ctl == NULL)
	{
        ret = -2;
		goto _bcb_err;
	}

	ret = avb_ops->read_from_partition(avb_ops,
										MISC_PARTITION,
										0,
										sizeof(struct bootloader_message_ab),
										s_bcb,
										&bytes_read);
	if (ret != AVB_IO_RESULT_OK) {
		printf("BootAndriod Err: Bcb read failed\n");
		goto _bcb_err;
	}

	/* Enter into fastboot mode if bcb string is bootonce or bootrecovery */
	if (0 == strncmp(s_bcb->message.command, BCB_BOOTONCE, strlen(BCB_BOOTONCE))|| \
	    0 == strncmp(s_bcb->message.command, BCB_BOOTRECOVERY, strlen(BCB_BOOTRECOVERY))) {
		printf("BootAndriod Info: Bcb read %ld bytes, %s\n", bytes_read, s_bcb->message.command);
		printf("BootAndriod Info: Enter fastboot mode\n");
		clear_bcb();
		run_command("fastboot usb 0", 0);
	}

    memset(boot_ctl, 0, sizeof(struct bootloader_control));
    memcpy(boot_ctl, (struct bootloader_control*)s_bcb->slot_suffix, sizeof(struct bootloader_control));

	res = CMD_RET_SUCCESS;

_bcb_err:
	if (res != CMD_RET_SUCCESS) {
        if (avb_ops != NULL) {
            avb_ops_free(avb_ops);
            avb_ops = NULL;
        }

		if (boot_ctl != NULL) {
            avb_free(boot_ctl);
            boot_ctl = NULL;
        }

        if (s_bcb != NULL) {
            avb_free(s_bcb);
            s_bcb = NULL;
        }
    }

	return res;
}

static int do_bootandroid(struct cmd_tbl_s *cmdtp, int flag, int argc,
						char * const argv[]) {
	
	const char * const requested_partitions[] = {"vbmeta", "boot", "vbmeta_system", NULL};
	AvbSlotVerifyResult slot_result = AVB_SLOT_VERIFY_RESULT_OK;
	AvbSlotVerifyData *slot_data = NULL;
	AvbIOResult ret = AVB_IO_RESULT_OK;
	AvbSlotVerifyFlags slotflags = AVB_SLOT_VERIFY_FLAGS_NONE;
	AvbHashtreeErrorMode htflags = AVB_HASHTREE_ERROR_MODE_RESTART_AND_INVALIDATE;
	int res = CMD_RET_FAILURE;
    char bp_name[32] = {0};

    res = do_andriod_bcb_business();
    if (res != CMD_RET_SUCCESS) {
        goto exit;
    }

	/* Retieve boot partition 's name suffix */
	slot_name_suffix = get_boot_partition_name_suffix();

	/* Start with slot verification in secure boot */
	if (get_system_boot_type()) {
		/* Verify boot partition requested in vbmeta.img */
		slot_result = avb_slot_verify(avb_ops,
									  requested_partitions,
									  slot_name_suffix,
									  slotflags,
									  htflags,
									&slot_data);

		if (slot_result == AVB_SLOT_VERIFY_RESULT_OK) {
			printf("BootAndriod Info: Request Partition are verified successfully\n");
			printf("BootAndriod cmdline: slot_data.cmdline:%s\n", slot_data->cmdline);
			prepare_boot_data(slot_data);
			if (ret == 0) {
				if (slot_data != NULL)
					avb_slot_verify_data_free(slot_data);
			}
		} else {
			/* In case of avb slot verification failure, Force system reset */
			run_command("reset", 0);
		}
	} else {
	/* Go to load BOOT partition directly in non-secure boot */		
        get_partition_name(BOOT_PARTITION, bp_name);
		prepare_partition_data(bp_name);
	}
    
exit:
	return res;
}

U_BOOT_CMD(
	bootandroid, 2,	1, do_bootandroid,
	"bootandroid   - boot android bootimg from device\n",
	"mmc0 | mmc1 | mmc2 | mmcX]\n    "
	"- boot application image stored in storage device like mmc\n"
);

