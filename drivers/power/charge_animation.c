/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <rtc.h>
#include <pwm.h>
#include <power/charge_display.h>
#include <power/charge_animation.h>
#include <power/fuel_gauge.h>
#include <power/pmic.h>
#include <mcu/mcu-uclass.h>
#ifdef CONFIG_IRQ
#include <irq-generic.h>
#include <rk_timer_irq.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define IMAGE_RECALC_IDX				-1
#define IMAGE_SOC_100_IDX(n)			((n) - 2)
#define IMAGE_LOWPOWER_IDX(n)			((n) - 1)
#define SYSTEM_SUSPEND_DELAY_MS			5000
#define FUEL_GAUGE_POLL_MS			1000

#define LED_CHARGING_NAME			"battery_charging"
#define LED_CHARGING_FULL_NAME			"battery_full"

struct charge_image {
	const char *name;
	int soc;
	int period;	/* ms */
};

struct charge_animation_priv {
	struct udevice *fg;
	struct udevice *charger;
	struct udevice *mcu;
#ifdef CONFIG_LED
	struct udevice *led_charging;
	struct udevice *led_full;
#endif
	const struct charge_image *image;
	int image_num;

	int auto_wakeup_key_state;
	ulong auto_screen_off_timeout;	/* ms */
	ulong suspend_delay_timeout;	/* ms */
};

#ifdef CONFIG_LED
static int leds_update(struct udevice *dev, int soc)
{
	struct charge_animation_priv *priv = dev_get_priv(dev);
	static int old_soc = -1;
	int ret, ledst;

	if (old_soc == soc)
		return 0;

	old_soc = soc;
	if (priv->led_charging) {
		ledst = (soc < 100) ? LEDST_ON : LEDST_OFF;
		ret = led_set_state(priv->led_charging, ledst);
		if (ret) {
			printf("set charging led %s failed, ret=%d\n",
			       (ledst == LEDST_ON) ? "ON" : "OFF", ret);
			return ret;
		}
	}

	if (priv->led_full) {
		ledst = (soc == 100) ? LEDST_ON : LEDST_OFF;
		ret = led_set_state(priv->led_full, ledst);
		if (ret) {
			printf("set charging full led %s failed, ret=%d\n",
			       ledst == LEDST_ON ? "ON" : "OFF", ret);
			return ret;
		}
	}

	return 0;
}
#else
static int leds_update(struct udevice *dev, int soc) { return 0; }
#endif

static int charge_animation_ofdata_to_platdata(struct udevice *dev)
{
	struct charge_animation_pdata *pdata = dev_get_platdata(dev);
	pdata->low_power_voltage =
		dev_read_u32_default(dev, "uboot-low-power-voltage", 0);

	return 0;
}

static int fg_charger_get_chrg_online(struct udevice *dev)
{
	struct charge_animation_priv *priv = dev_get_priv(dev);
	struct udevice *charger;

	charger = priv->charger ? : priv->fg;

	return fuel_gauge_get_chrg_online(charger);
}

static int charge_animation_show(struct udevice *dev)
{
	int soc, voltage, ret, charging = 0;
	struct charge_animation_priv *priv = dev_get_priv(dev);
	struct charge_animation_pdata *pdata = dev_get_platdata(dev);
	struct udevice *fg = priv->fg;
	struct udevice *mcu = priv->mcu;

	voltage = fuel_gauge_get_voltage(fg);
	if (voltage < 0)
		return -EINVAL;

	while (voltage < pdata->low_power_voltage + 50) {
		soc = fuel_gauge_update_get_soc(fg);
		if (soc < 0 || soc > 100) {
			printf("get soc failed: %d\n", soc);
			continue;
		} else if (soc >= 1) {
			printf("soc is: %d\n", soc);
			break;
		}

		voltage = fuel_gauge_get_voltage(fg);
		if (voltage < 0) {
			printf("get voltage failed: %d\n", voltage);
			continue;
		}

		ret = leds_update(dev, soc);
		if (ret)
			printf("update led failed: %d\n", ret);

		printf("soc is: %d voltage is :%d\n", soc, voltage);

		charging = fg_charger_get_chrg_online(dev);
		if (charging <= 0) {
			mcu_shutdown(); // shutdown system power
		}
		mdelay(100);
	};
	return 0;
}

static int fg_charger_get_device(struct udevice **fuel_gauge,
				 struct udevice **charger)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret, cap;

	*fuel_gauge = NULL,
	*charger = NULL;

	ret = uclass_get(UCLASS_FG, &uc);
	if (ret)
		return ret;

	for (uclass_first_device(UCLASS_FG, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		cap = fuel_gauge_capability(dev);
		if (cap == (FG_CAP_CHARGER | FG_CAP_FUEL_GAUGE)) {
			*fuel_gauge = dev;
			*charger = NULL;
		} else if (cap == FG_CAP_FUEL_GAUGE) {
			*fuel_gauge = dev;
		} else if (cap == FG_CAP_CHARGER) {
			*charger = dev;
		}
	}

	return (*fuel_gauge) ? 0 : -ENODEV;
}

static const struct dm_charge_display_ops charge_animation_ops = {
	.show = charge_animation_show,
};

static int charge_animation_probe(struct udevice *dev)
{
	struct charge_animation_priv *priv = dev_get_priv(dev);
	int ret, soc;
	/* Get PMIC: used for power off system  */
	ret = uclass_get_device(UCLASS_MCU, 0, &priv->mcu);
	if (ret) {
		if (ret == -ENODEV)
			printf("Can't find MCU\n");
		else
			printf("Get UCLASS MCU failed: %d\n", ret);
	}

	/* Get fuel gauge and charger(If need) */
	ret = fg_charger_get_device(&priv->fg, &priv->charger);
	if (ret) {
		if (ret == -ENODEV)
			debug("Can't find FG\n");
		else
			debug("Get UCLASS FG failed: %d\n", ret);
		// return ret;
	}


#ifdef CONFIG_LED
	ret = led_get_by_label(LED_CHARGING_NAME, &priv->led_charging);
	if (!ret)
		printf("Found Charging LED \n");
		
	ret = led_get_by_label(LED_CHARGING_FULL_NAME, &priv->led_full);
	if (!ret)
		printf("Found Charging-Full LED\n");
#endif

	printf("Enable charge animation display\n");

	return 0;
}

static const struct udevice_id charge_animation_ids[] = {
	{ .compatible = "rockchip,uboot-charge" },
	{ },
};

U_BOOT_DRIVER(charge_animation) = {
	.name = "charge-animation",
	.id = UCLASS_CHARGE_DISPLAY,
	.probe = charge_animation_probe,
	.of_match = charge_animation_ids,
	.ops = &charge_animation_ops,
	.ofdata_to_platdata = charge_animation_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct charge_animation_pdata),
	.priv_auto_alloc_size = sizeof(struct charge_animation_priv),
};
