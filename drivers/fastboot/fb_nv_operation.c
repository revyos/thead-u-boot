#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <common.h>
#include <fb_mmc.h>
#include <command.h>
#include <asm/io.h>
#include <asm/types.h>
#include <configs/light-c910.h>
#include <thead/clock_config.h>
#include <linux/bitops.h>
#include <asm/arch-thead/light-iopmp.h>
#include "../lib/sec_library/include/soc.h"

#define MAX_NV_NUMBER_SIZE 32
#define MAX_NV_DATA_SIZE 128
#define NV_BLOCK_SIZE 512

extern int32_t wj_efuse_get_lc(long unsigned int, int *lc);

typedef enum {
	FB_SYS_ACTION_FACTORY_RECOVER,
	FB_SYS_ACTION_READ_EFUSE,
} fb_sys_action_t;

static int nv_get(uint8_t *data,int offset,int data_len)
{
	struct blk_desc *dev_desc;
	struct disk_partition part_info;
	ulong block_start;
	uint8_t data_nv[NV_BLOCK_SIZE] = {0};
	int ret;
	int n;

	dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (dev_desc == NULL) {
		return -1;
	}

	ret = part_get_info_by_name(dev_desc, NV_PARTITION_NAME, &part_info);
	if (ret < 0) {
                return -2;
	}

        if(NV_BLOCK_SIZE != dev_desc->blksz) {
                return -4;
        }

	block_start = part_info.start + offset / dev_desc->blksz;
	n = blk_dread(dev_desc, block_start, 1, data_nv);
	if (n != 1) {
                return -5;
	}

        memcpy(data,data_nv + offset % NV_BLOCK_SIZE,data_len);

        return 0;
}

static int nv_set(uint8_t *data,int offset,int data_len)
{
        struct blk_desc *dev_desc;
	struct disk_partition part_info;
	ulong block_start;
        uint8_t data_nv[NV_BLOCK_SIZE] = {0};
        int ret;
        int n;


	dev_desc = blk_get_dev("mmc", CONFIG_FASTBOOT_FLASH_MMC_DEV);
	if (dev_desc == NULL) {
		return -1;
	}

	ret = part_get_info_by_name(dev_desc, NV_PARTITION_NAME, &part_info);
	if (ret < 0) {
                return -2;
	}

	if (NV_BLOCK_SIZE != dev_desc->blksz) {
                return -3;
        }

	block_start = part_info.start + offset / dev_desc->blksz;

	n = blk_dread(dev_desc, block_start, 1, data_nv);
        if (n != 1) {
                return -4;
        }
        memcpy(data_nv + offset % NV_BLOCK_SIZE,data,data_len);
	n = blk_dwrite(dev_desc, block_start, 1, data_nv);
	if (n != 1) {
                return -5;
	}

    return 0;
}

static int factory_recovery(char *response)
{
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
        fastboot_mmc_erase("metadata", response);
        if (strcmp(response, "OKAY") != 0) {
                return -1;
        }
        fastboot_mmc_erase("misc", response);
        if (strcmp(response, "OKAY") != 0) {
                return -2;
        }
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
        fastboot_nand_erase("metadata", response);
        if (strcmp(response, "OKAY") != 0) {
                return -3;
        }
        fastboot_nand_erase("misc", response);
        if (strcmp(response, "OKAY") != 0) {
                return -4;
        }
#endif

        strcpy(response,"OKAY");

        return 0;
}

static int read_efuse_status(char *response)
{
        int ret;
        int lc = 0;
        ret = wj_efuse_get_lc(WJ_EFUSE_BASE, &lc);
        if (ret) {
                return -1;
        }

        if(lc == 0) {
                strcpy(response,"LC_INIT");
        } else {
                strcpy(response,"LC_BLOWNED");
        }

        return 0;

}

static int sys_action(int flag,char *response)
{
        if (flag == FB_SYS_ACTION_FACTORY_RECOVER) {
                return factory_recovery(response);
        } else if (flag == FB_SYS_ACTION_READ_EFUSE) {
                return read_efuse_status(response);
        } else {
                return -1;
        }

        return 0;
}

static int toupper(int c)
{
        if (c >= 'a' && c <= 'z') {
                return c - 32;
        }

        return c;
}

static void str_to_hex(uint8_t *dest, const char *src, int len)
{
        char h1, h2;
        unsigned char s1, s2;

        for (int i = 0; i < len; i++) {
                h1 = src[2 * i];
                h2 = src[2 * i + 1];
                s1 = toupper(h1) - 48;
                s2 = toupper(h2) - 48;
                if (s1 > 9) {
                        s1 = s1 - 7;
                }

                if (s2 > 9) {
                        s2 = s2 - 7;
                }

                dest[i] = s1 * 16 + s2;
        }
}

static void hex_to_str(char *dest, const uint8_t *src, int len)
{
        char ddl = 0;
        char ddh = 0;
        int i = 0;

        for (i = 0; i < len; i++) {
                ddh = 48 + src[i] / 16;
                ddl = 48 + src[i] % 16;

                if (ddh > 57) {
                        ddh = ddh + 7;
                }

                if (ddl > 57) {
                        ddl = ddl + 7;
                }

                dest[i * 2] = ddh;
                dest[i * 2 + 1] = ddl;
        }

        dest[len * 2] = '\0';
}

static int char_to_int(char *data,int *out)
{
        *out = 0;
        while(*data) {
                if(*data < '0' || *data > '9')
                return -1;

                *out = *out * 10 + (*data - '0');
                ++data;
        }

	return 0;
}
#if CONFIG_FASTBOOT_CMD_OEM_NV_OPERATION
void oem_nv_factory_recovery_process(char *cmd_parameter, char *response)
{
        int ret = 0;
        int flag = 0;

        ret = char_to_int(cmd_parameter,&flag);
        if (ret != 0) {
                strcpy(response,"ERROR FLAG INVALID");
                return;
        }

        ret = sys_action(flag,response);
        if (ret != 0) {
                strcpy(response,"ERROR SYS ACTION FAILED");
                return;
        }
}


void oem_nv_get_proccess(char *cmd_parameter, char *response)
 {
        char *sep = NULL;
        char offset[MAX_NV_NUMBER_SIZE] = {0};
        char len[MAX_NV_NUMBER_SIZE] = {0};
        int sep_pos = 0;
        uint8_t nv_data[MAX_NV_DATA_SIZE + 32] = {0};
        char nv_data_str[MAX_NV_DATA_SIZE * 2 + 1] = {0};
        int ret;
        int offset_int = 0;
        int len_int = 0;

        sep = strstr(cmd_parameter, ":");
        if (sep == NULL) {
                strcpy(response,"ERROR INVALID PARAM");
                return;
        }

        sep_pos = sep - cmd_parameter;
        memcpy(offset,cmd_parameter,sep - cmd_parameter);
        memcpy(len,cmd_parameter + (sep - cmd_parameter) + 1,strlen(cmd_parameter) - sep_pos - 1);

        ret = char_to_int(offset,&offset_int);
        if (ret != 0) {
                strcpy(response,"ERROR OFFSET INVALID");
                return;
        }

        ret = char_to_int(len,&len_int);
        if (ret != 0) {
                strcpy(response,"ERROR LEN INVALID");
                return;
        }

        if (len_int > MAX_NV_DATA_SIZE) {
                strcpy(response,"ERROR NV SIZE TOO LARGE");
                return;
        }

        ret = nv_get(nv_data, offset_int, len_int);
        if (ret != 0) {
                printf("nv_get failed:%d\n",ret);
                strcpy(response,"ERROR NV GET FAILED");
                return;
        }

        hex_to_str(nv_data_str,nv_data,len_int);
        strcpy(response,"SUCCESS:");
        strcat(response,nv_data_str);
 }

void oem_nv_set_proccess(char *cmd_parameter, char *response)
{
        char *sep = NULL;
        char offset[MAX_NV_NUMBER_SIZE] = {0};
        uint8_t nv_data[MAX_NV_DATA_SIZE + 32] = {0};
        char *nv_data_str;
        int data_len;
        int ret;
        int offset_int = 0;

        sep = strstr(cmd_parameter, ":");
        if (sep == NULL) {
                strcpy(response,"ERROR INVALID PARAM");
                return;
        }

        memcpy(offset,cmd_parameter,sep - cmd_parameter);
        nv_data_str = cmd_parameter + (sep - cmd_parameter) + 1;

        data_len = strlen(nv_data_str) / 2;

        ret = char_to_int(offset,&offset_int);
        if (ret != 0) {
                strcpy(response,"ERROR OFFSET INVALID");
                return;
        }

        if (data_len > MAX_NV_DATA_SIZE) {
                strcpy(response,"ERROR NV SIZE TOO LARGE");
                return;
        }

        str_to_hex(nv_data,nv_data_str,data_len);

        ret = nv_set(nv_data, offset_int, data_len);
        if (ret != 0) {
                strcpy(response,"ERROR NV SET FAILED");
                return;
        }

        strcpy(response,"OKAY");
 }
 #endif