#include <common.h>
#include <asm/gpio.h>
#include <misc.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <command.h>
#include "light_regu.h"
#include <dm/device.h>

#define FDT32_TO_CPU(x) (fdt32_to_cpu(x))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (szieof(x) / sizeof(x[0]))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifdef AON_CONF_DEBUG
#define AON_CONF_D(fmt, args...) printf(fmt, ##args)
#else
#define AON_CONF_D(fmt, args...)
#endif

#define SOC_VIRTUAL_ID(virtual_id)      \
	{                                   \
		.id = virtual_id,               \
		.virtual_id_name = #virtual_id, \
	}

soc_virtual_id_t soc_base_virtual_id_list[] = {
	SOC_VIRTUAL_ID(SOC_DVDD18_AON),		  /*da9063:  ldo-3 */
	SOC_VIRTUAL_ID(SOC_AVDD33_USB3),	  /*da9063:  ldo-9 */
	SOC_VIRTUAL_ID(SOC_DVDD08_AON),		  /*da9063:  ldo-2 */
	SOC_VIRTUAL_ID(SOC_APCPU_DVDD_DVDDM), /*da9063:  vbcore1 & vbcore2*/
	SOC_VIRTUAL_ID(SOC_DVDD08_DDR),		  /*da9063:  buckperi */
	SOC_VIRTUAL_ID(SOC_VDD_DDR_1V8),	  /*da9063:  ldo-4  */
	SOC_VIRTUAL_ID(SOC_VDD_DDR_1V1),	  /*da9063:  buckmem & buckio  */
	SOC_VIRTUAL_ID(SOC_VDD_DDR_0V6),	  /*da9063:  buckpro */
	SOC_VIRTUAL_ID(SOC_DVDD18_AP),		  /*da9063:  ldo-11 */
	SOC_VIRTUAL_ID(SOC_DVDD08_AP),		  /*da9121:  da9121_ex */
	SOC_VIRTUAL_ID(SOC_AVDD08_MIPI_HDMI), /*da9063:  ldo-1  */
	SOC_VIRTUAL_ID(SOC_AVDD18_MIPI_HDMI), /*da9063:  ldo-5  */
	SOC_VIRTUAL_ID(SOC_DVDD33_EMMC),	  /*da9063:  ldo-10 */
	SOC_VIRTUAL_ID(SOC_DVDD18_EMMC),	  /*slg51000:ldo-3 */
	SOC_VIRTUAL_ID(SOC_DOVDD18_SCAN),	  /*da9063:  ldo-6 */
	SOC_VIRTUAL_ID(SOC_VEXT_2V8),		  /*da9063:  ldo-7 */
	SOC_VIRTUAL_ID(SOC_DVDD12_SCAN),	  /*da9063:  ldo-8 */
	SOC_VIRTUAL_ID(SOC_AVDD28_SCAN_EN),	  /*da9063:  gpio-4),SGM2019-ADJ */
	SOC_VIRTUAL_ID(SOC_AVDD28_RGB),		  /*slg51000:ldo-1 */
	SOC_VIRTUAL_ID(SOC_DOVDD18_RGB),	  /*slg51000:ldo-4 */
	SOC_VIRTUAL_ID(SOC_DVDD12_RGB),		  /*slg51000:ldo-5 */
	SOC_VIRTUAL_ID(SOC_AVDD25_IR),		  /*slg51000:ldo-2 */
	SOC_VIRTUAL_ID(SOC_DOVDD18_IR),		  /*slg51000:ldo-7 */
	SOC_VIRTUAL_ID(SOC_DVDD12_IR),		  /*slg51000:ldo-6 */
	SOC_VIRTUAL_ID(SOC_ADC_VREF),
	SOC_VIRTUAL_ID(SOC_LCD0_EN),
	SOC_VIRTUAL_ID(SOC_VEXT_1V8),
};

static int misc_regu_probe(struct udevice *dev)
{
	return 0;
}

static int misc_regu_remove(struct udevice *dev)
{
	return 0;
}

static soc_virtual_id_t *found_base_virtual_id(const char *name)
{
	for (int i = 0; i < ARRAY_SIZE(soc_base_virtual_id_list); i++)
	{
		if (!strcasecmp(soc_base_virtual_id_list[i].virtual_id_name, name))
		{
			return &soc_base_virtual_id_list[i];
		}
	}
	return NULL;
}

static inline char toupper(char str1)
{
	if (str1 >= 'a' && str1 <= 'z')
	{
		return str1 - 'a' + 'A';
	}
	return str1;
}

void string_to_upper(char *str)
{
	if (str == NULL)
		return;

	while (*str)
	{
		*str = toupper((unsigned char)*str);
		str++;
	}
}

static int misc_regu_get_virtual_regu_config(struct udevice *dev, ofnode parent_node, virtual_regu_list_t *regu_list)
{
	int regu_num = 0;
	int ret;
	ofnode child_node;
	soc_virtual_id_t *id_list;
	soc_virtual_id_t *soc_base_id;
	ofnode_for_each_subnode(child_node, parent_node)
	{
		// printf("sub node name: %s\n", ofnode_get_name(child_node));
		regu_num++;
	}

	if (!regu_num)
	{
		printf("regu list not found in dts\n");
		return -1;
	}

	id_list = devm_kcalloc(dev, 1, regu_num * sizeof(soc_virtual_id_t), GFP_KERNEL);
	if (!id_list)
	{
		printf("regu id malloc faild\n");
		return -ENOMEM;
	}

	int index = 0;
	int new_regu_index = ARRAY_SIZE(soc_base_virtual_id_list);

	ofnode_for_each_subnode(child_node, parent_node)
	{
		const char *virtual_id_name = ofnode_get_name(child_node);
	    uint32_t min_uv;
		uint32_t max_uv;
		soc_base_id = found_base_virtual_id(virtual_id_name);
		if (soc_base_id)
		{
			id_list[index].id = soc_base_id->id;
		}
		else
		{
			id_list[index].id = new_regu_index++;
		}

		int copy_size = MIN(sizeof(id_list[index].virtual_id_name) - 1, strlen(virtual_id_name));
		memcpy(id_list[index].virtual_id_name, virtual_id_name, copy_size);
		id_list[index].virtual_id_name[copy_size] = '\0';

		string_to_upper(id_list[index].virtual_id_name);

		ret = ofnode_read_u32(child_node, "regulator-min-microvolt", &min_uv);
		if (ret)
		{
			// printf("%s :regulator-min-microvolt not set, min_uv set to -1", virtual_id_name);
			id_list[index].min_uv = -1;
		}
		else
		{
			id_list[index].min_uv = min_uv;
		}

		ret = ofnode_read_u32(child_node, "regulator-max-microvolt", &max_uv);
		if (ret)
		{
			// printf("%s :regulator-max-microvolt not set, max_uv set to -1", virtual_id_name);
			id_list[index].max_uv = -1;
		}
		else
		{
			id_list[index].max_uv = max_uv;
		}
		if (id_list[index].max_uv < id_list[index].min_uv)
		{
			printf("id:%d regulator-max-microvolt:%d is smaller than regulator-min-microvolt:%d\n", index, id_list[index].max_uv, id_list[index].min_uv);
			return -1;
		}
		// printf("Get virtual regu_id:[%d]:%s min_uv:%dmv max_uv:%dmv\n", id_list[index].id, id_list[index].virtual_id_name,
		// 	   id_list[index].min_uv, id_list[index].max_uv);
		index++;
	}

	(*regu_list).regu_num = regu_num;
	(*regu_list).regu_list = id_list;

	return 0;
}

static int misc_grep_pmic_dev_name_info(const char *dev_name, pmic_dev_info_t *dev)
{
	int flag_num = 0;
	int version_flag = 0;
	int index = 0;
	const char *dev_name_orig = dev_name;
	while (*dev_name)
	{
		if (*dev_name == ',')
		{
			flag_num++;
			if (flag_num == 2)
			{
				version_flag = index;
			}
		}
		index++;
		dev_name++;
	}

	if (flag_num != 2 || *(dev_name - 1) == ',')
	{
		printf("pmic-name should set as pmic-name = \"vendor,type,version");
		return -1;
	}

	int len = MIN((version_flag), sizeof(dev->device_name) - 1);
	memcpy(dev->device_name, dev_name_orig, len);
	dev->device_name[len] = '\0';
	len = MIN((index - version_flag - 1), sizeof(dev->device_name) - 1);
	memcpy(dev->version_name, dev_name_orig + version_flag + 1, len);
	dev->version_name[len] = '\0';
	return 0;
}

static int get_node_index(const char *name)
{
	while (*name && *name != '@')
	{
		name++;
	}

	if (strlen(name) == 0)
	{
		return -1;
	}
	name++;
	return strtoul(name, NULL, 10);
}

static int misc_pmic_parent_io_ctrl_config(ofnode dev_node, pmic_parent_ctrl_info_t *ctrl_info)
{
	ofnode parent_ctrl_sub_node, parent_pmic_node;
	int get_aon_ctrl_info_flag = 0;
	int get_pmic_pin_ctrl_info_flag = 0;
	uint32_t prop_size;
	int prop_len;
	const uint32_t *prop_val;

	ofnode_for_each_subnode(parent_ctrl_sub_node, dev_node)
	{
		const char *node_name = ofnode_get_name(parent_ctrl_sub_node);
		if (!strncmp(node_name, PMIC_PARENT_CTRL_NAME, strlen(PMIC_PARENT_CTRL_NAME)))
		{
			if (ofnode_read_bool(parent_ctrl_sub_node, "aon_pin_ctrl_info"))
			{
				prop_val = ofnode_get_property(parent_ctrl_sub_node, "aon_pin_ctrl_info", &prop_len);
				if (!prop_val)
				{
					printf("aon_pin_ctrl_info property not found\n");
					return -1;
				}
				prop_size = prop_len / sizeof(uint32_t);
				if (prop_size != 3)
				{
					printf("invalid pmic-aon_pin_ctrl_info cell size %d, it should set format as aon_pin_ctrl_info = <<port> <pin> <activate_status>>\n", prop_size);
					return -1;
				}
				ctrl_info->pmic_ctrl_type = PMIC_CTRL_BY_AON_GPIO;
				ctrl_info->info.aon_io.gpio_port = FDT32_TO_CPU(prop_val[0]);
				ctrl_info->info.aon_io.pin = FDT32_TO_CPU(prop_val[1]);
				ctrl_info->info.aon_io.activate_status = FDT32_TO_CPU(prop_val[2]);
				if (ctrl_info->info.aon_io.activate_status != HW_ID_ACTIVATE_HIGH && ctrl_info->info.aon_io.activate_status != HW_ID_ACTIVATE_LOW)
				{
					printf("aon io activate should set to: ACTIVATE_HIGH:0 or ACTIVATE_LOW:1");
					return -1;
				}
				get_aon_ctrl_info_flag = 1;
			}

			if (ofnode_read_bool(parent_ctrl_sub_node, "pmic_pin_ctrl_info"))
			{
				prop_val = ofnode_get_property(parent_ctrl_sub_node, "pmic_pin_ctrl_info", &prop_len);
				if (!prop_val)
				{
					printf("pmic_pin_ctrl_info property not found\n");
					return -1;
				}
				prop_size = prop_len / sizeof(uint32_t);
				if (prop_size != 3)
				{
					printf("invalid pmic-pmic_pin_ctrl_info cell size %d, it should set format as pmic_pin_ctrl_info = <<&pmic_dev> <pin> <activate_status>>\n", prop_size);
					return -1;
				}

				parent_pmic_node = ofnode_get_by_phandle(FDT32_TO_CPU(prop_val[0]));
				if (!ofnode_valid(parent_pmic_node))
				{
					printf("parent_pmic_node  not found\n");
					return -1;
				}
				int parent_pmic_index = get_node_index(ofnode_get_name(parent_pmic_node));
				if (parent_pmic_index < 0)
				{
					printf("parent_pmic_node index not found\n");
					return -1;
				}

				ctrl_info->pmic_ctrl_type = PMIC_CTRL_BY_PMIC_GPIO;
				ctrl_info->info.pmic_io.pmic_id = parent_pmic_index;
				ctrl_info->info.pmic_io.io_hw_id = FDT32_TO_CPU(prop_val[1]);
				ctrl_info->info.pmic_io.activate_status = FDT32_TO_CPU(prop_val[2]);
				if (ctrl_info->info.pmic_io.activate_status != HW_ID_ACTIVATE_HIGH && ctrl_info->info.pmic_io.activate_status != HW_ID_ACTIVATE_LOW)
				{
					printf("pmic io activate should set to: HW_ID_ACTIVATE_HIGH:0 or HW_ID_ACTIVATE_LOW:1");
					return -1;
				}
				get_pmic_pin_ctrl_info_flag = 1;
			}

			if (get_aon_ctrl_info_flag && get_pmic_pin_ctrl_info_flag)
			{
				printf("aon_pin_ctrl_info or pmic_pin_ctrl_info should only support one for a pmic dev\n");
				return -1;
			}
			if (!get_aon_ctrl_info_flag && !get_pmic_pin_ctrl_info_flag)
			{
				printf("aon_pin_ctrl_info or pmic_pin_ctrl_info should only support one for a pmic dev\n");
				return -1;
			}
			return 0;
		}
	}
	ctrl_info->pmic_ctrl_type = PMIC_CTRL_BY_NOTHINTG;
	return 0;
}

static int misc_regu_get_pmic_dev_config(ofnode parent_node, pmic_dev_info_t *pmic_dev_info_list)
{
	int ret = 0;
	ofnode child_node;
	int index;
	const char *pmic_name;
	int pmic_wdt_flag = 0;
	int pmic_index = 0;
	int pmic_addr_len = 0, pmic_addr_size;
	int gpio_addr_len = 0, gpio_addr_size;
	char err_io_str[40] = "NOT_SUPPORT";
	char lpm_io_str[40] = "NOT_SUPPORT";
	char parent_pmic_str[60] = "NOT_SUPPORT";
	uint32_t port, pin, trigger_mode;
	const uint32_t *prop_val;

	ofnode_for_each_subnode(child_node, parent_node)
	{
		pmic_dev_info_t *dev = &(pmic_dev_info_list[pmic_index]);
		const char *node_name = ofnode_get_name(child_node);
		if (!strncmp(node_name, PMIC_DEV_DTS_NAME, strlen(PMIC_DEV_DTS_NAME)))
		{
			index = get_node_index(node_name);
			if (index < 0)
			{
				printf("get pmic_dev id faild");
				return -1;
			}
			pmic_name = ofnode_read_string(child_node, "pmic-name");
			if (!pmic_name)
			{
				printf("pmic_name property not set for %s %d", PMIC_DEV_DTS_NAME, index);
				return -1;
			}

			if (ofnode_read_bool(child_node, "pmic_wdt_on"))
			{
				if (pmic_wdt_flag)
				{
					printf("only one pmic dev support wdt\n");
					return -1;
				}
				dev->flag |= PMIC_DEV_ENABLE_WDT;
				pmic_wdt_flag = 1;
			}

			prop_val = ofnode_get_property(child_node, "pmic-addr", &pmic_addr_len);
			if (!prop_val)
			{
				printf("pmic-addr property not found\n");
				return -1;
			}
			pmic_addr_size = pmic_addr_len / sizeof(uint32_t);

			if (pmic_addr_size != 2 && pmic_addr_size != 1)
			{
				printf("invalid pmic-addr cell size %d\n", pmic_addr_size);
				return -1;
			}

			dev->addr1 = FDT32_TO_CPU(prop_val[0]);
			dev->addr2 = pmic_addr_size == 2 ? FDT32_TO_CPU(prop_val[1]) : dev->addr1;

			prop_val = ofnode_get_property(child_node, "errio_gpio", &gpio_addr_len);
			if (prop_val)
			{
				gpio_addr_size = gpio_addr_len / sizeof(uint32_t);
				if (gpio_addr_size != 3)
				{
					printf("invalid errio_gpio cell size %d\n", gpio_addr_size);
					return -1;
				}
				else
				{
					port = FDT32_TO_CPU(prop_val[0]);
					pin = FDT32_TO_CPU(prop_val[1]);
					trigger_mode = FDT32_TO_CPU(prop_val[2]);
					dev->flag |= PMIC_DEV_ENABLE_ERR_IO;
					dev->err_io_info.gpio_port = port;
					dev->err_io_info.pin = pin;
					dev->err_io_info.trigger_mode = trigger_mode;
					if (dev->err_io_info.trigger_mode > GPIO_IRQ_MODE_HIGH_LEVEL)
					{
						printf("io trigger_mode support max:%d, it is set to %d\n", GPIO_IRQ_MODE_HIGH_LEVEL, dev->err_io_info.trigger_mode);
						return -1;
					}
					sprintf(err_io_str, "port:%d pin:%d trigger:%d", port, pin, trigger_mode);
				}
			}
			else
			{
				sprintf(err_io_str, "NOT_SUPPORT");
			}

			prop_val = ofnode_get_property(child_node, "lpm_gpio", &gpio_addr_len);
			if (prop_val)
			{
				gpio_addr_size = gpio_addr_len / sizeof(uint32_t);
				if (gpio_addr_size != 3)
				{
					printf("invalid lpm_gpio cell size %d\n", gpio_addr_size);
					return -1;
				}
				else
				{
					port = FDT32_TO_CPU(prop_val[0]);
					pin = FDT32_TO_CPU(prop_val[1]);
					trigger_mode = FDT32_TO_CPU(prop_val[2]);
					dev->flag |= PMIC_DEV_ENABLE_LPM_IO;
					dev->lpm_io_info.gpio_port = port;
					dev->lpm_io_info.pin = pin;
					dev->lpm_io_info.trigger_mode = trigger_mode;
					if (dev->lpm_io_info.trigger_mode > GPIO_IRQ_MODE_HIGH_LEVEL)
					{
						printf("io trigger_mode support max:%d, it is set to %d\n", GPIO_IRQ_MODE_HIGH_LEVEL, dev->lpm_io_info.trigger_mode);
						return -1;
					}
					sprintf(lpm_io_str, "port:%d pin:%d trigger:%d", port, pin, trigger_mode);
				}
			}
			else
			{
				sprintf(lpm_io_str, "NOT_SUPPORT");
			}
			ret = misc_pmic_parent_io_ctrl_config(child_node, &dev->ctrl_info);
			if (ret)
			{
				printf("grep pmic parent io ctrl faild %d\n", ret);
				return -1;
			}
			else
			{
				if (dev->ctrl_info.pmic_ctrl_type == PMIC_CTRL_BY_AON_GPIO)
				{
					sprintf(parent_pmic_str, "ctrl by aon io: port:%d pin:%d activate_status:%d", dev->ctrl_info.info.aon_io.gpio_port, dev->ctrl_info.info.aon_io.pin, dev->ctrl_info.info.aon_io.activate_status);
				}
				else if (dev->ctrl_info.pmic_ctrl_type == PMIC_CTRL_BY_PMIC_GPIO)
				{
					sprintf(parent_pmic_str, "ctrl by pmic io: pmic:%d hw_id:%d activate_status:%d", dev->ctrl_info.info.pmic_io.pmic_id, dev->ctrl_info.info.pmic_io.io_hw_id, dev->ctrl_info.info.pmic_io.activate_status);
				}
				else
				{
					sprintf(parent_pmic_str, "NOT_SUPPORT");
				}
			}

			dev->pmic_id = index;
			ret = misc_grep_pmic_dev_name_info(pmic_name, dev);
			pmic_index++;
			AON_CONF_D("Get pmic dev:[%d]:%s|%s addr1:0x%02x addr2:0x%02x wdt:{%s} errio:{%s} lpm_io:{%s} pmic_ctrl_info:{%s}\n", index, dev->device_name, dev->version_name, dev->addr1, dev->addr2, (dev->flag & PMIC_DEV_ENABLE_WDT ? "SUPPORT" : "NOT_SUPPORT"), err_io_str, lpm_io_str, parent_pmic_str);
		}
	}
	if (!pmic_index)
	{
		printf("No %s node found\n", PMIC_DEV_DTS_NAME);
		return -1;
	}
	return 0;
}

static int misc_regu_get_pmic_dev_by_name(const char *name, int pmic_dev_num, pmic_dev_info_t *pmic_dev_info_list)
{
	int pmic_id = get_node_index(name);
	if (pmic_id < 0)
	{
		return -1;
	}

	int pmic_index = 0;
	for (; pmic_index < pmic_dev_num; pmic_index++)
	{
		if (pmic_dev_info_list[pmic_index].pmic_id == pmic_id)
		{
			break;
		}
	}

	if (pmic_index == pmic_dev_num)
	{
		printf("%s not found in pmic list\n", name);
		return -1;
	}

	return pmic_index;
}

static int misc_regu_get_each_regu_hw_id_config(ofnode regu_id_node, int pmic_dev_num, pmic_dev_info_t *pmic_dev_info_list, soc_virtual_id_t *virtual_id_info, pmic_hw_info_t *id)
{
	ofnode pmic_node, pmic_parent_node;

	int prop_len, prop_size;
	int pmic_index;
	const uint32_t *prop_val;

	/*get pmic_dev = <&pmic_dev_0 DA9063_ID_BCORE1>*/
	prop_val = ofnode_get_property(regu_id_node, "pmic_dev", &prop_len);
	if (!prop_val)
	{
		printf("pmic-addr property not found\n");
		return -1;
	}

	prop_size = prop_len / sizeof(uint32_t);
	if (prop_size != 2)
	{
		printf("pmic_dev property should set in format as  pmic_dev = <&pmic_dev_num HW_ID>;\n");
		return -1;
	}

	pmic_node = ofnode_get_by_phandle(FDT32_TO_CPU(prop_val[0]));
	if (!ofnode_valid(pmic_node))
	{
		printf("pmic node not found\n");
		return -1;
	}

	pmic_index = misc_regu_get_pmic_dev_by_name(ofnode_get_name(pmic_node), pmic_dev_num, pmic_dev_info_list);
	if (pmic_index < 0)
	{
		return -1;
	}

	(*id).pmic_id = pmic_dev_info_list[pmic_index].pmic_id;
	(*id).hw_id = FDT32_TO_CPU(prop_val[1]);

	/*get auto_on_info = <0 1 800000>*/
	prop_val = ofnode_get_property(regu_id_node, "auto_on_info", &prop_len);
	if (!prop_val)
	{
		(*id).soft_power_ctrl_info.on_info.on_order = HW_ID_NO_SOFT_AUTO_ON;
	}
	else
	{
		prop_size = prop_len / sizeof(uint32_t);
		if (prop_size != 3 && prop_size != 2)
		{
			printf("auto_on_info property should set in format as auto_on_info = <on_order on_delay_ms [on_uv_mv]>\n");
			return -1;
		}
		if (virtual_id_info->min_uv != -1 && FDT32_TO_CPU(prop_val[2]) < virtual_id_info->min_uv)
		{
			printf("virtual regu %s voltage shoud larger than %dmv, it is %dmv\n", virtual_id_info->virtual_id_name, virtual_id_info->min_uv, FDT32_TO_CPU(prop_val[2]));
			return -1;
		}
		if (virtual_id_info->max_uv != -1 && FDT32_TO_CPU(prop_val[2]) > virtual_id_info->max_uv)
		{
			printf("virtual regu %s voltage shoud less than %dmv, it is %dmv\n", virtual_id_info->virtual_id_name, virtual_id_info->max_uv, FDT32_TO_CPU(prop_val[2]));
			return -1;
		}
		(*id).soft_power_ctrl_info.on_info.on_order = FDT32_TO_CPU(prop_val[0]);
		(*id).soft_power_ctrl_info.on_info.on_delay_ms = FDT32_TO_CPU(prop_val[1]);
		if (prop_size == 3)
		{
			(*id).soft_power_ctrl_info.on_info.init_target_uv = FDT32_TO_CPU(prop_val[2]);
		}
		else
		{
			(*id).soft_power_ctrl_info.on_info.init_target_uv = 0;
		}
	}

	/*get auto_off_info = <1 1>*/
	prop_val = ofnode_get_property(regu_id_node, "auto_off_info", &prop_len);
	if (!prop_val)
	{
		(*id).soft_power_ctrl_info.off_info.off_order = HW_ID_NO_SOFT_AUTO_OFF;
	}
	else
	{
		prop_size = prop_len / sizeof(uint32_t);
		if (prop_size != 2)
		{
			printf("auto_off_info property should set in format as auto_off_info = <off_order off_delay_ms>\n");
			return -1;
		}
		(*id).soft_power_ctrl_info.off_info.off_order = FDT32_TO_CPU(prop_val[0]);
		(*id).soft_power_ctrl_info.off_info.off_delay_ms = FDT32_TO_CPU(prop_val[1]);
	}

	/*get parent_pmic_dev = <&pmic_dev_0 2 1>*/
	prop_val = ofnode_get_property(regu_id_node, "parent_pmic_dev", &prop_len);
	if (!prop_val)
	{
		(*id).parent_hw_info.pmic_id = PMIC_ID_INVALID;
	}
	else
	{
		prop_size = prop_len / sizeof(uint32_t);
		if (prop_size != 3)
		{
			printf("parent_pmic_dev property should set in format as  parent_pmic_dev = <&pmic_dev_num IO_ID ACTIVATE_STATUS>;\n");
			return -1;
		}

		pmic_parent_node = ofnode_get_by_phandle(FDT32_TO_CPU(prop_val[0]));
		if (!ofnode_valid(pmic_parent_node))
		{
			printf("pmic_parent node not found\n");
			return -1;
		}

		pmic_index = misc_regu_get_pmic_dev_by_name(ofnode_get_name(pmic_parent_node), pmic_dev_num, pmic_dev_info_list);
		if (pmic_index < 0)
		{
			return -1;
		}

		(*id).parent_hw_info.pmic_id = pmic_dev_info_list[pmic_index].pmic_id;
		(*id).parent_hw_info.io_hw_id = FDT32_TO_CPU(prop_val[1]);
		(*id).parent_hw_info.activate_status = FDT32_TO_CPU(prop_val[2]);
		if (FDT32_TO_CPU(prop_val[2]) != HW_ID_ACTIVATE_HIGH && FDT32_TO_CPU(prop_val[2]) != HW_ID_ACTIVATE_LOW)
		{
			printf("parent hw io activate should set to: HW_ID_ACTIVATE_HIGH:0 or HW_ID_ACTIVATE_LOW:1");
			return -1;
		}
	}

	return 0;
}

static int misc_regu_get_each_regu_config(ofnode regu_config_node, int pmic_dev_num, pmic_dev_info_t *pmic_dev_info_list, soc_virtual_id_t *regu_info, csi_regu_id_t *pmic_regu_id_info)
{
	int ret = 0;
	ofnode hw_id_node;
	ofnode coupling_node;
	int index = 0;
	const uint32_t *prop_val;
	int prop_len = 0;
	int prop_size = 0;
	int coupling_num = 0;
	uint16_t hw_id_used_flag = 0x0;

	ofnode_for_each_subnode(hw_id_node, regu_config_node)
	{
		const char *node_name = ofnode_get_name(hw_id_node);
		if (!strncmp(node_name, REGU_ID_NAME, strlen(REGU_ID_NAME)))
		{
			index = get_node_index(node_name);
			if (index < 0)
			{
				printf("get hw_id faild");
				return -1;
			}

			if (index >= PMIC_MAX_HW_ID_NUM || index >= 8 * 2)
			{
				printf("regu_id index should less than %d\n", MIN(PMIC_MAX_HW_ID_NUM, 8 * 2));
				return -1;
			}

			if ((hw_id_used_flag >> index) & 0x01)
			{
				printf("%s@%d already exist\n", REGU_ID_NAME, index);
				return -1;
			}
			else
			{
				hw_id_used_flag |= 0x01 << index;
			}

			ret = misc_regu_get_each_regu_hw_id_config(hw_id_node, pmic_dev_num, pmic_dev_info_list, regu_info, &pmic_regu_id_info->sub.id[index]);
			if (ret)
			{
				printf("get hw_id@%d config faild %d\n", index, ret);
				return -1;
			}
		}
	}

	for (int i = PMIC_MAX_HW_ID_NUM - 1; i >= 0; i--)
	{
		if ((hw_id_used_flag & (0x01 << i)) == 0x0)
		{
			(*pmic_regu_id_info).sub.id[i].pmic_id = PMIC_ID_INVALID;
		}
	}

	ofnode_for_each_subnode(coupling_node, regu_config_node)
	{
		const char *node_name = ofnode_get_name(coupling_node);

		if (!strncmp(node_name, COUPLING_ID_INFO_NAME, strlen(COUPLING_ID_INFO_NAME)))
		{
			/*get info = <0 1 -5 30>;*/
			prop_val = ofnode_get_property(coupling_node, "info", &prop_len);
			if (!prop_val)
			{
				printf("no info property set for %s", node_name);
				return -1;
			}
			else
			{
				prop_size = prop_len / sizeof(uint32_t);
				if (prop_size != 4)
				{
					printf("coupling info property should set in format as info = <id0 id1 max_spread min_spread)>\n");
					return -1;
				}

				int id0 = FDT32_TO_CPU(prop_val[0]);
				int id1 = FDT32_TO_CPU(prop_val[1]);
				int8_t max_spread = FDT32_TO_CPU(prop_val[2]);
				int8_t min_spread = FDT32_TO_CPU(prop_val[3]);

				if (ofnode_read_bool(coupling_node, "negative-min"))
				{
					min_spread = -min_spread;
				}
				if (ofnode_read_bool(coupling_node, "negative-max"))
				{
					max_spread = -max_spread;
				}
				if (id0 == id1)
				{
					printf("coupling info: id0 id1 should not be equal");
					return -1;
				}
				if (min_spread > max_spread)
				{
					printf("coupling info: min_spread:%d is higher than max_spread:%d", min_spread, max_spread);
					return -1;
				}
				if (id0 >= PMIC_MAX_HW_ID_NUM || id1 >= PMIC_MAX_HW_ID_NUM)
				{
					printf("coupling info: id0:%d id1:%d is higher than max_id:%d", id0, id1, PMIC_MAX_HW_ID_NUM - 1);
					return -1;
				}

				if ((*pmic_regu_id_info).sub.id[id0].pmic_id == PMIC_ID_INVALID || (*pmic_regu_id_info).sub.id[id1].pmic_id == PMIC_ID_INVALID)
				{
					printf("coupling info:id0:%d id1:%d is invalid", id0, id1);
					return -1;
				}
				(*pmic_regu_id_info).sub.coupling_list[coupling_num].id0 = id0;
				(*pmic_regu_id_info).sub.coupling_list[coupling_num].id1 = id1;
				(*pmic_regu_id_info).sub.coupling_list[coupling_num].max_spread = max_spread;
				(*pmic_regu_id_info).sub.coupling_list[coupling_num].min_spread = min_spread;
				coupling_num++;
				if (coupling_num > PMIC_MAX_COUPLING_NUM)
				{
					printf("coupling info should no more than %d\n", coupling_num);
					return -1;
				}
			}
		}
	}

	for (int i = PMIC_MAX_COUPLING_NUM - 1; i >= coupling_num; i--)
	{
		(*pmic_regu_id_info).sub.coupling_list[i].id0 = REGU_SUB_ID_INVALID;
		(*pmic_regu_id_info).sub.coupling_list[i].id1 = REGU_SUB_ID_INVALID;
	}

	return 0;
}

static int misc_regu_get_regu_config(ofnode parent_node, int pmic_dev_num, pmic_dev_info_t *pmic_dev_info_list, int virtual_id_num, soc_virtual_id_t *regu_list, csi_regu_id_t *pmic_regu_id_list)
{
	ofnode child_node;
	uint32_t phandle = 0;
	int ret = 0;
	ofnode regu_virtual_node;
	const char *regu_id_name;
	int virtual_id_index = 0;
	uint16_t virtual_id_config_flag = 0;
	int regu_config_index = 0;

	ofnode_for_each_subnode(child_node, parent_node)
	{
		virtual_id_index = 0;
		const char *node_name = ofnode_get_name(child_node);
		if (!strncmp(node_name, REGU_ID_CONF_NAME, strlen(REGU_ID_CONF_NAME)))
		{

			if (ofnode_read_u32(child_node, "reg_info", &phandle))
			{
				printf("reg_info property not found\n");
				return -1;
			}

			regu_virtual_node = ofnode_get_by_phandle(phandle);
			if (!ofnode_valid(regu_virtual_node))
			{
				printf("virtual_regu_node  not found\n");
				return -1;
			}

			regu_id_name = ofnode_get_name(regu_virtual_node);

			for (; virtual_id_index < virtual_id_num; virtual_id_index++)
			{
				if (!strcasecmp(regu_list[virtual_id_index].virtual_id_name, regu_id_name))
				{
					break;
				}
			}

			if (virtual_id_index == virtual_id_num)
			{
				printf("virtual regu id %s not found\n", regu_id_name);
				return -1;
			}

			int virtual_id = regu_list[virtual_id_index].id;

			if ((virtual_id_config_flag >> virtual_id) & 0x01)
			{
				printf("%s config for %s already exist\n!", REGU_ID_CONF_NAME, regu_list[virtual_id_index].virtual_id_name);
				return -1;
			}
			else
			{
				virtual_id_config_flag |= 0x01 << virtual_id;
			}

			csi_regu_id_t *regu_conf = &pmic_regu_id_list[regu_config_index];

			regu_conf->regu_ext_id = virtual_id;
			int copy_size = MIN(sizeof(regu_conf->regu_ext_id_name) - 1, strlen(regu_list[virtual_id_index].virtual_id_name));
			memcpy(regu_conf->regu_ext_id_name, regu_list[virtual_id_index].virtual_id_name, copy_size);

			ret = misc_regu_get_each_regu_config(child_node, pmic_dev_num, pmic_dev_info_list, &regu_list[virtual_id_index], regu_conf);
			if (ret)
			{
				return -1;
			}

			AON_CONF_D("Get regu config, virtual_regu_id:[%d]:%s min_uv:%dmv max_uv:%dmv\n", virtual_id, regu_list[virtual_id_index].virtual_id_name, regu_list[virtual_id_index].min_uv, regu_list[virtual_id_index].max_uv);
			for (int i = 0; i < ARRAY_SIZE(regu_conf->sub.id); i++)
			{
				pmic_hw_info_t *sub = &regu_conf->sub.id[i];
				if (sub->pmic_id != PMIC_ID_INVALID)
				{
					char parent_info[50];
					char auto_on_info[50];
					char auto_off_info[50];

					if (sub->parent_hw_info.pmic_id == PMIC_ID_INVALID)
					{
						sprintf(parent_info, "{NO_PARENT_PMIC}");
					}
					else
					{
						sprintf(parent_info, "{parent_pmic_dev:%d io_hw_id:%d activate_status:%d}", sub->parent_hw_info.pmic_id, sub->parent_hw_info.io_hw_id, sub->parent_hw_info.activate_status);
					}

					if (sub->soft_power_ctrl_info.on_info.on_order == HW_ID_NO_SOFT_AUTO_ON)
					{
						sprintf(auto_on_info, "{NOT_SUPPORT}");
					}
					else
					{
						sprintf(auto_on_info, "{on_order:%d on_delay:%d on_uv:%dmv}", sub->soft_power_ctrl_info.on_info.on_order, sub->soft_power_ctrl_info.on_info.on_delay_ms, sub->soft_power_ctrl_info.on_info.init_target_uv);
					}
					if (sub->soft_power_ctrl_info.off_info.off_order == HW_ID_NO_SOFT_AUTO_OFF)
					{
						sprintf(auto_off_info, "{NOT_SUPPORT}");
					}
					else
					{
						sprintf(auto_off_info, "{off_order:%d off_delay:%d}", sub->soft_power_ctrl_info.off_info.off_order, sub->soft_power_ctrl_info.off_info.off_delay_ms);
					}

					AON_CONF_D(">>>>>>%s@%d:{pmic_dev:%d hw_id:%d} parent_info:%s auto_on_info:%s auto_off_info:%s\n", REGU_ID_NAME, i, sub->pmic_id, sub->hw_id, parent_info, auto_on_info, auto_off_info);
				}
			}
			int temp_flag = 0;
			for (int i = 0; i < ARRAY_SIZE(regu_conf->sub.coupling_list); i++)
			{
				coupling_desc_t *coupling_info = &regu_conf->sub.coupling_list[i];
				if (coupling_info->id0 != REGU_SUB_ID_INVALID)
				{
					if (!temp_flag)
					{
						AON_CONF_D(">>>>>>");
						temp_flag = 1;
					}
					AON_CONF_D("%s@%d:{id0:%d id1:%d max_spreed:%dmv min_spreed:%dmv}   ", COUPLING_ID_INFO_NAME, i, coupling_info->id0, coupling_info->id1, coupling_info->max_spread * 10, coupling_info->min_spread * 10);
				}
			}
			if (temp_flag)
			{
				AON_CONF_D("\n");
			}
			regu_config_index++;
		}
	}
	if (!regu_config_index)
	{
		printf("No %s node found\n", REGU_ID_CONF_NAME);
		return -1;
	}
	return 0;
}

static int misc_regu_get_aon_pmic_config(struct udevice *dev, ofnode parent_node, int virtual_id_num, soc_virtual_id_t *regu_list, pmic_dev_list_t *pmic_list, regu_id_list_t *regu_id_list)
{
	ofnode child_node;
	int pmic_dev_num = 0;
	int regu_id_conf_num = 0;
	pmic_dev_info_t *pmic_dev_info_list;
	csi_regu_id_t *pmic_regu_id_list;

	const char *node_name;
	int ret = 0;

	ofnode_for_each_subnode(child_node, parent_node)
	{
		node_name = ofnode_get_name(child_node);
		if (!strncmp(node_name, PMIC_DEV_DTS_NAME, strlen(PMIC_DEV_DTS_NAME)))
		{
			pmic_dev_num++;
		}
	}

	if (!pmic_dev_num)
	{
		printf("No %s node in dts\n", PMIC_DEV_DTS_NAME);
		return -1;
	}

	pmic_dev_info_list = devm_kcalloc(dev, 1, pmic_dev_num * sizeof(pmic_dev_info_t), GFP_KERNEL);
	if (!pmic_dev_info_list)
	{
		printf("pmic dev list malloc faild\n");
		return -ENOMEM;
	}

	ret = misc_regu_get_pmic_dev_config(parent_node, pmic_dev_info_list);
	if (ret)
	{
		printf("pmic dev config get faild %d", ret);
		free(pmic_dev_info_list);
		return -1;
	}

	ofnode_for_each_subnode(child_node, parent_node)
	{
		node_name = ofnode_get_name(child_node);
		if (!strncmp(node_name, REGU_ID_CONF_NAME, strlen(REGU_ID_CONF_NAME)))
		{
			regu_id_conf_num++;
		}
	}

	if (!regu_id_conf_num)
	{
		printf("No %s node in dts\n", REGU_ID_CONF_NAME);
		return -1;
	}

	pmic_regu_id_list = devm_kcalloc(dev, 1, regu_id_conf_num * sizeof(csi_regu_id_t), GFP_KERNEL);
	if (!pmic_regu_id_list)
	{
		printf("pmic regu list malloc faild\n");
		return -ENOMEM;
	}

	ret = misc_regu_get_regu_config(parent_node, pmic_dev_num, pmic_dev_info_list, virtual_id_num, regu_list, pmic_regu_id_list);
	if (ret)
	{
		printf("get regu config faild %d\n", ret);
		free(pmic_dev_info_list);
		free(pmic_regu_id_list);
		return -1;
	}

	(*pmic_list).pmic_num = pmic_dev_num;
	(*pmic_list).pmic_list = pmic_dev_info_list;

	(*regu_id_list).regu_id_num = regu_id_conf_num;
	(*regu_id_list).regu_id_list = pmic_regu_id_list;

	return 0;
}

extern int device_bind_ofnode(struct udevice *parent, const struct driver *driver,
                       const char *name, const void *platdata, ofnode node,
                       struct udevice **devp);

static int misc_regu_bind(struct udevice *dev)
{
	struct mic_regu_platdata *plat = dev_get_platdata(dev);
	ofnode parent_node = dev->node;
	const uint32_t *prop_val;
	int prop_len = 0;
	int prop_size = 0;
	int ret;
	ofnode child_node, aon_conf_node = {0}, regu_node = {0};
	struct udevice *dev_1;

	/* If this is a child device, there is nothing to do here */
	if (plat)
	{
		return 0;
	}

	if (!ofnode_valid(parent_node))
	{
		printf("aon node not ok\n");
		return -1;
	}

	int get_regu_dts_flag = 0;
	int get_aon_conf_dst_flag = 0;

	ofnode_for_each_subnode(child_node, parent_node)
	{
		/* Increment base_id for all subnodes, also the disabled ones */
		// printf("sub node name: %s\n", ofnode_get_name(child_node));
		if (!strncmp(ofnode_get_name(child_node), REGU_DTS_NAME, strlen(REGU_DTS_NAME)))
		{
			regu_node = child_node;
			get_regu_dts_flag = 1;
		}
		if (!strncmp(ofnode_get_name(child_node), AON_CONF_NAME, strlen(AON_CONF_NAME)))
		{
			aon_conf_node = child_node;
			get_aon_conf_dst_flag = 1;
		}
	}

	if (!get_regu_dts_flag)
	{
		printf("No %s node in dts\n", REGU_DTS_NAME);
		return -1;
	}

	if (!get_aon_conf_dst_flag)
	{
		printf("No %s node in dts\n", AON_CONF_NAME);
		return -1;
	}

	plat = devm_kcalloc(dev, 1, sizeof(struct mic_regu_platdata), GFP_KERNEL);
	if (!plat)
	{
		return -ENOMEM;
	}

	plat->wakeup_flag = 0;

	if (ofnode_read_bool(parent_node, "wakeup-by-gpio-on"))
	{
		plat->wakeup_flag |= AON_WAKEUP_BY_GPIO;
		printf("aon wakeup by gpio enabled\n");
	}

	if (ofnode_read_bool(parent_node, "wakeup-by-rtc-on"))
	{
		plat->wakeup_flag |= AON_WAKEUP_BY_RTC;
		printf("aon wakeup by rtc enabled\n");
	}

	/*grep ii-config info*/
	prop_val = ofnode_get_property(aon_conf_node, "iic-config", &prop_len);
	if (!prop_val)
	{
		printf("pmic-addr property not found\n");
		return -1;
	}
	prop_size = prop_len / sizeof(uint32_t);
	if (prop_size != 3)
	{
		printf("invalid iic-config cell size %d,it should set in format as iic-config = <<id> <addr_mode> <speed>>\n", prop_size);
		return -1;
	}
	plat->iic_config.iic_id = FDT32_TO_CPU(prop_val[0]);
	plat->iic_config.addr_mode = FDT32_TO_CPU(prop_val[1]);
	plat->iic_config.speed = FDT32_TO_CPU(prop_val[2]);
	if (plat->iic_config.addr_mode > IIC_ADDRESS_10BIT)
	{
		printf("iic addr_mode only support IIC_ADDRESS_7BIT:0 IIC_ADDRESS_10BIT:1\n");
		return -1;
	}
	if (plat->iic_config.speed > IIC_BUS_SPEED_HIGH)
	{
		printf("iic speed max support IIC_BUS_SPEED_HIGH:%d\n", IIC_BUS_SPEED_HIGH);
		return -1;
	}
	printf("iic id:%d addr_mode:%d speed:%d\n", plat->iic_config.iic_id, plat->iic_config.addr_mode, plat->iic_config.speed);

	ret = misc_regu_get_virtual_regu_config(dev, regu_node, &plat->regu_list);
	if (ret)
	{
		printf("get virtual regu config failed %d\n", ret);
		return -1;
	}

	ret = misc_regu_get_aon_pmic_config(dev, aon_conf_node, plat->regu_list.regu_num, plat->regu_list.regu_list, &plat->pmic_list, &plat->regu_id_list);
	if (ret)
	{
		printf("get aon config failed %d\n", ret);
		return -1;
	}

	plat->name = ofnode_get_name(parent_node);

	ret = device_bind_ofnode(dev, dev->driver, plat->name, plat, parent_node, &dev_1);
	if (ret)
	{
		printf("bind device faild %d", ret);
		return ret;
	}
	/*fix me err usage*/
	dev->platdata = plat;

	return 0;
}

static const struct udevice_id misc_regu_ids[] = {
	{.compatible = "thead,light-aon"},
	{}};

U_BOOT_DRIVER(light_regu) = {
	.name = "light_regu,misc",
	.id = UCLASS_MISC,
	.of_match = misc_regu_ids,
	.probe = misc_regu_probe,
	.bind = misc_regu_bind,
	.remove = misc_regu_remove,
};
