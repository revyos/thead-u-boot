
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mcu/mcu-uclass.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

#define HC32FX_POWEROFF_20    0x20
#define HC32FX_POWERON_30    0x30
#define POWER_OFF   0x55
#define POWER_ON   0x01

struct hc32fx_info {
	struct udevice *dev;
};

static u8 hc32fx_read(struct hc32fx_info *hc32fx, u8 reg)
{
	u8 val;
	int ret;

	ret = dm_i2c_read(hc32fx->dev, reg, &val, 1);
	if (ret) {
		printf("write error to device: %p register: %#x!",
		      hc32fx->dev, reg);
		return ret;
	}

	return val;
}

static int hc32fx_write(struct hc32fx_info *hc32fx, u8 reg, u8 val)
{
	int ret;

	ret = dm_i2c_write(hc32fx->dev, reg, &val, 1);
	if (ret) {
		printf("write error to device: %p register: %#x!",
		      hc32fx->dev, reg);
		return ret;
	}

	return 0;
}

static int mcu_hc32fx_poweron(struct udevice *dev)
{
	struct hc32fx_info *hc32fx = dev_get_priv(dev);
	int ret;

	ret = hc32fx_write(hc32fx, HC32FX_POWERON_30, POWER_ON);
	if(ret)
		printf("set mcu POWERON fail\n");

	return ret;
}

static int mcu_hc32fx_shutdown(struct udevice *dev)
{
	struct hc32fx_info *hc32fx = dev_get_priv(dev);
	int ret;

	ret = hc32fx_write(hc32fx, HC32FX_POWEROFF_20, POWER_OFF);
	if(ret)
		printf("set mcu POWEROFF fail\n");

	return ret;
}

static int mcu_gpio_probe(struct udevice *dev)
{
	struct hc32fx_info *priv = dev_get_priv(dev);
	priv->dev = dev;

	return 0;
}

static const struct mcu_ops mcu_hc32fx_ops = {
	.poweron	= mcu_hc32fx_poweron,
	.shutdown	= mcu_hc32fx_shutdown,
};

static const struct udevice_id hc32fx_ops_ids[] = {
	{ .compatible = "mcu_hc32fx" },
	{ }
};

U_BOOT_DRIVER(mcu_gpio) = {
	.name	= "hc32fx-mcu",
	.id	= UCLASS_MCU,
	.of_match = hc32fx_ops_ids,
	.ops	= &mcu_hc32fx_ops,
	.priv_auto_alloc_size = sizeof(struct hc32fx_info),
	.probe	= mcu_gpio_probe,
};
