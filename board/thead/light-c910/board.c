// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <asm/io.h>
#include <dwc3-uboot.h>
#include <usb.h>
#include <usb/xhci.h>
#include <cpu_func.h>
#include <asm/gpio.h>
#include <abuf.h>
#include "sec_library.h"

#ifdef CONFIG_LIGHT_AON_CONF
#include "../../../drivers/misc/light_regu.h"
#include "dm/device.h"
#include "dm/uclass.h"
#endif

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

	if (init == USB_INIT_DEVICE) {
		dwc3_device_data.dr_mode = USB_DR_MODE_PERIPHERAL;
	} else {
		dwc3_device_data.dr_mode = USB_DR_MODE_HOST;
	}

	return dwc3_uboot_init(&dwc3_device_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	dwc3_uboot_exit(index);
	return 0;
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{


	int ret = board_usb_init(index, USB_INIT_HOST);
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)dwc3_device_data.base;
	*hcor = (struct xhci_hcor *)(dwc3_device_data.base +
			HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));;

	return ret;
}

void xhci_hcd_stop(int index)
{
	board_usb_cleanup(index, USB_INIT_HOST);
}

int g_dnl_board_usb_cable_connected(void)
{
	return 1;
}
#endif

#ifdef CONFIG_CMD_BOOT_SLAVE
#ifdef CONFIG_LIGHT_AON_CONF
#define E902_AON_CONFIG_SIZE 0xC00
#else
#define E902_AON_CONFIG_SIZE 0x000
#endif
#define E902_SYSREG_START	0xfffff48044
#define E902_SYSREG_RESET	0xfffff44024
#define E902_START_ADDRESS	(0xFFEF8000 + E902_AON_CONFIG_SIZE)
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

#ifdef CONFIG_LIGHT_AON_CONF

int get_and_set_aon_config_data(void)
{
	int ret =0;
    struct udevice *dev;
	struct mic_regu_platdata *config_data =NULL;

	ret = uclass_first_device_err(UCLASS_MISC, &dev);
	if(ret){
        printf("get light aon config faild %d\n", ret);
		return ret;
	}

	config_data = (struct mic_regu_platdata *)(dev->platdata);

    volatile aon_config_t* read_config = (aon_config_t* )C910_E902_START_ADDRESS;
	if(strncmp((const char*)read_config->magic , AON_CONFIG_MAGIC, strlen(AON_CONFIG_MAGIC))) {
        printf("No aon config magic found in aon bin, please check the aon bin\n");
		return -1;
	}

	if(strncmp((const char*)read_config->version, AON_CONFIG_VERSION, strlen(AON_CONFIG_VERSION))) {
       printf("Err aon config version, aon bin is:%s, u-boot is:%s\n", read_config->version, AON_CONFIG_VERSION);
	   return -1;
	}

	if(PMIC_MAX_HW_ID_NUM >  read_config->max_hw_id_num) {
        printf("Invald max hw id num, aon bin support %d , u-boot is %d\n",read_config->max_hw_id_num, PMIC_MAX_HW_ID_NUM);
		return -1;
	}

	/*set pmic dev info */
	int pmic_dev_num =  config_data->pmic_list.pmic_num;
    int pmic_dev_list_offset   = sizeof(aon_config_t);
	uint64_t pmic_dev_start_addr  =  C910_E902_START_ADDRESS + pmic_dev_list_offset;

	int  regu_num            = config_data->regu_id_list.regu_id_num;
	int  regu_id_list_offset = pmic_dev_list_offset + pmic_dev_num * sizeof(pmic_dev_info_t);
	uint64_t regu_start_addr  = C910_E902_START_ADDRESS + regu_id_list_offset;
    int aon_bin_size  =  regu_id_list_offset + regu_num* sizeof(csi_regu_id_t);
    if( aon_bin_size > read_config->aon_config_partition_size) {
         printf("Invalid aon partition size, aon bin support:%lld, u-boot is %d\n", read_config->aon_config_partition_size, aon_bin_size);
		 return -1;
	}

	printf("pmic_dev_num:%d offset:%d addr:%lld\n",pmic_dev_num, pmic_dev_list_offset, pmic_dev_start_addr);

	memcpy((void*)pmic_dev_start_addr, config_data->pmic_list.pmic_list, pmic_dev_num * sizeof(pmic_dev_info_t));
    printf("regu_num:%d offset:%d addr:%lld\n",regu_num,regu_id_list_offset, regu_start_addr);

	memcpy((void*)regu_start_addr, config_data->regu_id_list.regu_id_list, regu_num * sizeof(csi_regu_id_t));
    
	read_config->wakeup_flag = config_data->wakeup_flag;
	read_config->aon_pmic.iic_config.iic_id = config_data->iic_config.iic_id;
	read_config->aon_pmic.iic_config.addr_mode = config_data->iic_config.addr_mode;
	read_config->aon_pmic.iic_config.speed  = config_data->iic_config.speed;
	read_config->aon_pmic.pmic_dev_num =  pmic_dev_num;
    read_config->aon_pmic.pmic_dev_list_offset   = pmic_dev_list_offset;

	/*set regu list info*/
	read_config->aon_pmic.regu_num =  regu_num;
	read_config->aon_pmic.regu_id_list_offset = regu_id_list_offset;

	memcpy((void*)read_config->uboot_set_magic, UBOOT_CONFIG_MAGIC, strlen(UBOOT_CONFIG_MAGIC));

    flush_cache((uintptr_t)C910_E902_START_ADDRESS, aon_bin_size);

	printf("-->pmic_dev_num:%d offset:%d\n",read_config->aon_pmic.pmic_dev_num, read_config->aon_pmic.pmic_dev_list_offset);
	printf("-->regu_num:%d offset:%d\n",read_config->aon_pmic.regu_num,read_config->aon_pmic.regu_id_list_offset);
	return 0;
}
#endif

int do_boot_aon(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_LIGHT_AON_CONF
    int ret = 0;
	ret = get_and_set_aon_config_data();
	if(ret) {
        printf("aon config and set faild %d", ret);
		hang();
		return ret;
	}
#endif
	writel(0xffffffff, (void *)(E902_IOPMP_BASE + 0xc0));
	disable_slave_cpu();
	set_slave_cpu_entry(E902_START_ADDRESS);
	flush_cache((uintptr_t)C910_E902_START_ADDRESS, 0x10000);
	enable_slave_cpu();
	return 0;
}

U_BOOT_CMD(
	bootaon, CONFIG_SYS_MAXARGS, 0, do_boot_aon,
	"Boot aon from memory  ",
	" "
);

int do_bootslave(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	boot_audio();
	return 0;
}
#endif

static void light_c910_set_gpio_output_high(void)
{
	ofnode node;
	struct gpio_desc select_gpio;

	printf("%s: trying to set gpio output high\n", __func__);

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		printf("%s: no /config node?\n", __func__);
		return;
	}

	if (gpio_request_by_name_nodev(node, "select-gpio", 0,
				       &select_gpio, GPIOD_IS_OUT)) {
		printf("%s: could not find a /config/select-gpio\n", __func__);
		return;
	}

	dm_gpio_set_value(&select_gpio, 1);
}

int misc_init_r(void)
{
	light_c910_set_gpio_output_high();

	return 0;
}

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
