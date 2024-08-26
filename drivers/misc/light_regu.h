#ifndef __LIGHT_REGU_H__
#define __LIGHT_REGU_H__

typedef enum
{
	SOC_DVDD18_AON,		  /*da9063:  ldo-3 */
	SOC_AVDD33_USB3,	  /*da9063:  ldo-9 */
	SOC_DVDD08_AON,		  /*da9063:  ldo-2 */
	SOC_APCPU_DVDD_DVDDM, /*da9063:  vbcore1 & vbcore2*/
	SOC_DVDD08_DDR,		  /*da9063:  buckperi */
	SOC_VDD_DDR_1V8,	  /*da9063:  ldo-4  */
	SOC_VDD_DDR_1V1,	  /*da9063:  buckmem & buckio  */
	SOC_VDD_DDR_0V6,	  /*da9063:  buckpro */
	SOC_DVDD18_AP,		  /*da9063:  ldo-11 */
	SOC_DVDD08_AP,		  /*da9121:  da9121_ex */
	SOC_AVDD08_MIPI_HDMI, /*da9063:  ldo-1  */
	SOC_AVDD18_MIPI_HDMI, /*da9063:  ldo-5  */
	SOC_DVDD33_EMMC,	  /*da9063:  ldo-10 */
	SOC_DVDD18_EMMC,	  /*slg51000:ldo-3 */
	SOC_DOVDD18_SCAN,	  /*da9063:  ldo-6 */
	SOC_VEXT_2V8,		  /*da9063:  ldo-7 */
	SOC_DVDD12_SCAN,	  /*da9063:  ldo-8 */
	SOC_AVDD28_SCAN_EN,	  /*da9063:  gpio-4,SGM2019-ADJ */
	SOC_AVDD28_RGB,		  /*slg51000:ldo-1 */
	SOC_DOVDD18_RGB,	  /*slg51000:ldo-4 */
	SOC_DVDD12_RGB,		  /*slg51000:ldo-5 */
	SOC_AVDD25_IR,		  /*slg51000:ldo-2 */
	SOC_DOVDD18_IR,		  /*slg51000:ldo-7 */
	SOC_DVDD12_IR,		  /*slg51000:ldo-6 */
	SOC_ADC_VREF,
	SOC_LCD0_EN,
	SOC_VEXT_1V8,

	SOC_REGU_INVALID = 0xFF
} soc_virtual_id_en;

#define REGU_DTS_NAME "light-regu-reg"
#define AON_CONF_NAME "aon_pmic_config"
#define PMIC_DEV_DTS_NAME "pmic-dev"
#define PMIC_PARENT_CTRL_NAME "pmic_ctrl_info"
#define REGU_ID_CONF_NAME "regu_config"
#define REGU_ID_NAME "regu_id"
#define COUPLING_ID_INFO_NAME "coupling_info"

#define PMIC_DEV_ENABLE_WDT (1U << 0)
#define PMIC_DEV_ENABLE_ERR_IO (1U << 1)
#define PMIC_DEV_ENABLE_LPM_IO (1U << 2)

#define HW_ID_NO_SOFT_AUTO_ON (0xff)
#define HW_ID_NO_SOFT_AUTO_OFF (0xff)
#define HW_ID_INVALID (0xff)
#define PMIC_ID_INVALID (0xff)
#define REGU_SUB_ID_INVALID (0xff)

#define REGU_EXT_ID_NAME_LEN 30
#define PMIC_DEV_NAME_LEN 20
#define PMIC_DEV_VERSION_LEN 20

#define PMIC_MAX_HW_ID_NUM 3
#define PMIC_MAX_COUPLING_NUM 3

#define AON_WAKEUP_BY_GPIO (1 << 0)
#define AON_WAKEUP_BY_RTC (1 << 1)

#define AON_CONFIG_MAGIC "AON_CONFIG"
#define UBOOT_CONFIG_MAGIC "UBOOT_SET"
#define AON_CONFIG_VERSION "1.0.0"

typedef enum
{
	HW_ID_ACTIVATE_HIGH = 0U,
	HW_ID_ACTIVATE_LOW = 1U,
} hw_activate_status_en;

typedef struct __packed
{
	uint8_t pmic_id;
	uint8_t io_hw_id;
	uint8_t activate_status;
} pmic_parent_hw_io_ctrl_info_t;

typedef struct __packed
{
	uint8_t on_order;
	uint8_t on_delay_ms;
	uint32_t init_target_uv;
} regu_soft_power_ctrl_on_t;

typedef struct __packed
{
	uint8_t off_order;
	uint8_t off_delay_ms;
} regu_soft_power_ctrl_off_t;

typedef struct __packed
{
	regu_soft_power_ctrl_on_t on_info;
	regu_soft_power_ctrl_off_t off_info;
} regu_soft_power_ctrl_t;

typedef struct __packed
{
	uint8_t id0;
	uint8_t id1;
	int8_t max_spread; // mv/10
	int8_t min_spread; // mv/10
} coupling_desc_t;

typedef enum
{
	GPIO_IRQ_MODE_RISING_EDGE = 0, ///< Interrupt mode for rising edge
	GPIO_IRQ_MODE_FALLING_EDGE,	   ///< Interrupt mode for falling edge
	GPIO_IRQ_MODE_BOTH_EDGE,	   ///< Interrupt mode for both edge
	GPIO_IRQ_MODE_LOW_LEVEL,	   ///< Interrupt mode for low level
	GPIO_IRQ_MODE_HIGH_LEVEL,	   ///< Interrupt mode for high level
} csi_gpio_irq_mode_t;

typedef enum
{
	IIC_ADDRESS_7BIT = 0U, ///< 7-bit address mode
	IIC_ADDRESS_10BIT	   ///< 10-bit address mode
} csi_iic_addr_mode_t;

typedef enum
{
	IIC_BUS_SPEED_STANDARD = 0U, ///< Standard Speed  (<=100kHz)
	IIC_BUS_SPEED_FAST,			 ///< Fast Speed      (<=400kHz)
	IIC_BUS_SPEED_FAST_PLUS,	 ///< Fast plus Speed (<=  1MHz)
	IIC_BUS_SPEED_HIGH			 ///< High Speed      (<=3.4MHz)
} csi_iic_speed_t;

typedef struct __packed
{
	uint8_t pmic_id;
	uint8_t hw_id;
	uint8_t benable;
	pmic_parent_hw_io_ctrl_info_t parent_hw_info;
	regu_soft_power_ctrl_t soft_power_ctrl_info;
} pmic_hw_info_t;

typedef struct __packed
{
	coupling_desc_t coupling_list[PMIC_MAX_COUPLING_NUM];
	pmic_hw_info_t id[PMIC_MAX_HW_ID_NUM]; ///<  sub id1 for single-rail or first src of dual-rail
} pmic_hw_id_t;

typedef struct __packed
{
	uint8_t regu_ext_id;						 ///< virtual global regulator id
	char regu_ext_id_name[REGU_EXT_ID_NAME_LEN]; ///< vitual regu-id name
	pmic_hw_id_t sub;							 ///<  sub id set for dual-rail/single-rail regulator
} csi_regu_id_t;

typedef enum
{
	PMIC_CTRL_BY_AON_GPIO = 0U,
	PMIC_CTRL_BY_PMIC_GPIO = 1U,
	PMIC_CTRL_BY_NOTHINTG = 0xFF,
} pmic_ctrl_info_en;

typedef struct __packed
{
	uint8_t gpio_port;
	uint8_t pin;
	uint8_t activate_status;
} pmic_ctrl_by_aon_info_t;

typedef struct __packed
{
	uint8_t pmic_id;
	uint8_t io_hw_id;
	uint8_t activate_status;
} pmic_ctrl_by_pmic_info_t;

typedef struct __packed
{
	uint8_t pmic_ctrl_type;
	union
	{
		pmic_ctrl_by_aon_info_t aon_io;
		pmic_ctrl_by_pmic_info_t pmic_io;
	} info;
} pmic_parent_ctrl_info_t;

typedef struct __packed
{
	uint8_t gpio_port;
	uint8_t pin;
	uint8_t trigger_mode;
} pmic_interrupt_io_info_t;

typedef struct __packed
{
	char device_name[PMIC_DEV_NAME_LEN];
	char version_name[PMIC_DEV_VERSION_LEN];
	uint8_t pmic_id;
	uint8_t addr1;
	uint8_t addr2;
	uint8_t flag; /*support wdt|errio| lpm io*/
	uint8_t slew_rate;
	uint32_t wdt_len;
	pmic_interrupt_io_info_t err_io_info;
	pmic_interrupt_io_info_t lpm_io_info;
	pmic_parent_ctrl_info_t ctrl_info;
} pmic_dev_info_t;

typedef struct
{
	soc_virtual_id_en id;
	char virtual_id_name[REGU_EXT_ID_NAME_LEN];
	int min_uv;
	int max_uv;
} soc_virtual_id_t;

typedef struct
{
	int regu_num;
	soc_virtual_id_t *regu_list;
} virtual_regu_list_t;

typedef struct
{
	int pmic_num;
	pmic_dev_info_t *pmic_list;
} pmic_dev_list_t;

typedef struct
{
	int regu_id_num;
	csi_regu_id_t *regu_id_list;
} regu_id_list_t;

typedef struct __packed
{
	uint8_t iic_id;	   ///< iic id
	uint8_t addr_mode; ///< iic addr_mode  ---> csi_iic_addr_mode_t
	uint8_t speed;	   ///< iic speed type  ---> csi_iic_speed_t
	uint8_t reserved[1];
} csi_pmic_if_config_t;

struct mic_regu_platdata
{
	const char *name;
	uint32_t wakeup_flag;
	csi_pmic_if_config_t iic_config;
	virtual_regu_list_t regu_list;
	pmic_dev_list_t pmic_list;
	regu_id_list_t regu_id_list;
};

typedef struct __packed
{
	csi_pmic_if_config_t iic_config;
	uint8_t pmic_dev_num;
	uint8_t regu_num;
	uint32_t pmic_dev_list_offset;
	uint32_t regu_id_list_offset;
} aon_pmic_config_t;

typedef struct __packed
{
	const char magic[11];
	const char version[11];
	const char uboot_set_magic[11];
	uint8_t max_hw_id_num;
	uint64_t aon_config_partition_size;
	uint32_t wakeup_flag;
	aon_pmic_config_t aon_pmic;
} aon_config_t;

#endif