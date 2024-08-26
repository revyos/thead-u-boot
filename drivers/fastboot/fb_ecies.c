#include <sec_ecies_session.h>
#include <common.h>
#include <command.h>
#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <part.h>
#include <stdlib.h>
#include <csi_sec_img_verify.h>

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
extern void fasboot_uboot_write_process(void *buf, char *response);
#define MAX_ECIES_IMAGE_SIZE (9*1024*1024)
#define ECIES_MALLOC_START (MAX_ECIES_IMAGE_SIZE * 2)

typedef struct {
	uint32_t magic;
	uint32_t file_size;
	char partition_name[64];
} __attribute__((__packed__)) send_file_info_t;

static uint32_t ecies_file_pos = 0;
static int slave_init = 0;

static int current_pos = 0;

void *csi_ecies_malloc(uint32_t size)
{
    void * ptr = NULL;

    if (current_pos + size >= CONFIG_FASTBOOT_BUF_SIZE) {
        current_pos = 0;
    }

    ptr = (void *)(long)(CONFIG_FASTBOOT_BUF_ADDR + ECIES_MALLOC_START + current_pos);
    current_pos += size;
    return ptr;
};

void csi_ecies_free(void *buffer)
{
    return;
};

static int ecies_data_write(uint8_t *buf,uint8_t *data,int data_len)
{
    char response[FASTBOOT_RESPONSE_LEN];

    if (data_len == 0  || buf == NULL || data == NULL) {
        return -1;
    }

    memcpy(buf + ecies_file_pos,data,data_len);
    ecies_file_pos += data_len;
    if (ecies_file_pos == sizeof(send_file_info_t) + ((send_file_info_t *)buf)->file_size) {
        if (strcmp(((send_file_info_t *)buf)->partition_name, TEE_PART_NAME) == 0) {
            memcpy((void *)LIGHT_TEE_FW_ADDR, buf + sizeof(send_file_info_t), ((send_file_info_t *)buf)->file_size);
            #if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
            fastboot_mmc_flash_write(((send_file_info_t *)buf)->partition_name, buf + sizeof(send_file_info_t),  ((send_file_info_t *)buf)->file_size,
                        response);
            #endif
            #if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
            fastboot_nand_flash_write(((send_file_info_t *)buf)->partition_name,  buf + sizeof(send_file_info_t), ((send_file_info_t *)buf)->file_size,
                        response);
            #endif
        } else if (strcmp(((send_file_info_t *)buf)->partition_name, UBOOT_PART_NAME) == 0) {
            fasboot_uboot_write_process(buf + sizeof(send_file_info_t),response);
        } else {
            printf("unknown partition name\n");
            return -2;
        }

        ecies_file_pos = 0;
    } else if(ecies_file_pos > sizeof(send_file_info_t) + ((send_file_info_t *)buf)->file_size) {
        return -3;
    }

    return 0;
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

int ecies_process_data(uint8_t * data, int data_size,char *response)
{
    static ecies_session_t ss_slave;
    char apduResponse[ECIES_INIT_RESPONSE_LEN] = {0};
    uint8_t *plaintext = csi_ecies_malloc(MAX_ECIES_IMAGE_SIZE);
    uint32_t plaintextLen = 0;
    uint32_t apduResponseLen = 0;
    int ret = 0;
    uint8_t cla = 0;
    uint8_t errcode = 0;

    if (plaintext == NULL) {
        ret = -1;
        goto end;
    }

    if (!slave_init) {
        csi_sec_library_init();
        ret = hal_ecies_slave_init(&ss_slave);
        if (ret != 0) {
            strcpy(response,"hal_ecies_slave_init ERROR");
            ret = -2;
            goto end;
        }
    }
    ret = hal_ecies_slave_session_comm(&ss_slave, data, data_size, (uint8_t *)apduResponse, &apduResponseLen, plaintext, &plaintextLen);
    if (ret != 0) {
        ret = hal_ecies_status_get((uint8_t *)apduResponse, apduResponseLen, &cla, &errcode);
        if (ret != 0) {
            strcpy(response,"hal_ecies_errcode_get ERROR");
            ret = -3;
            goto end;
        }

        if (cla == ECIES_CLA_INITIALIZE_UPDATE_RESPONSE && errcode == ECIES_RESPONSE_SESSION_OPENED_ERROR) {
            slave_init = 0;
            ecies_file_pos = 0;
            hal_ecies_slave_uninit(&ss_slave);
            ret = hal_ecies_slave_init(&ss_slave);
            if (ret != 0) {
                strcpy(response,"hal_ecies_slave_init ERROR");
                ret = -4;
                goto end;
            }
            ret = hal_ecies_slave_session_comm(&ss_slave, data, data_size,  (uint8_t*)apduResponse, &apduResponseLen, plaintext, &plaintextLen);
            if (ret != 0) {
                strcpy(response,"hal_ecies_slave_session_comm ERROR");
                ret = -5;
                goto end;
            }
        } else if(errcode != ECIES_RESPONSE_OK) {
            if (errcode == ECIES_CLA_SDATA_SEND_RESPONSE) {
                strcpy(response,"ECIES_CLA_SDATA_SEND_RESPONSE ERROR");
                ret = -6;
                goto end;
            } else if (errcode == ECIES_CLA_SESSION_CLOSE) {
                strcpy(response,"ECIES_CLA_SESSION_CLOSE ERROR");
                ret = -7;
                goto end;
            } else if (errcode == ECIES_CLA_INITIALIZE_UPDATE) {
                strcpy(response,"ECIES_CLA_INITIALIZE_UPDATE ERROR");
                ret = -8;
                goto end;
            }
        }
    }

    if (slave_init && plaintextLen) {
        ret = ecies_data_write((uint8_t*)(long)(CONFIG_FASTBOOT_BUF_ADDR + MAX_ECIES_IMAGE_SIZE),plaintext,plaintextLen);
        if (ret != 0) {
            strcpy(response,"ecies_data_write ERROR");
            ret = -8;
            goto end;
        }
    }

    slave_init = 1;

    strcpy(response,"SUCCESS:");
    hex_to_str(response + strlen(response),(uint8_t*)apduResponse,apduResponseLen);
    ret = 0;
end:
    if (plaintext) {
        csi_ecies_free(plaintext);
        plaintext = NULL;
    }

    return ret;
}

#endif