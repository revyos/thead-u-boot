// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Radxa Limited
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 *
 * Author:
 * - Jagan Teki <jagan@amarulasolutions.com>
 * - Stephen Chen <stephen@radxa.com>
 */

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>

struct jadard_panel_desc {
	const struct display_timing *timing;
	unsigned long mode_flags;
	enum mipi_dsi_pixel_format format;
	unsigned int lanes;
};

struct panel_info {
	const struct jadard_panel_desc *desc;
	struct gpio_desc	reset;
	struct gpio_desc	hsvcc;
	struct gpio_desc	vspn3v3;
	bool   prepared;
	bool   enabled;
};

static int jd9365_get_display_timing(struct udevice *dev,
		struct display_timing *timings)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(dev);
	struct mipi_dsi_device *device = plat->device;
	struct panel_info *pinfo = dev_get_priv(dev);

	memcpy(timings, pinfo->desc->timing, sizeof(*timings));

	device->lanes	   = pinfo->desc->lanes;
	device->format	   = pinfo->desc->format;
	device->mode_flags = pinfo->desc->mode_flags;

	return 0;
}

static int jadard_prepare(struct udevice *panel)
{
	struct panel_info *pinfo = dev_get_priv(panel);
	int ret;

	if (pinfo->prepared)
		return 0;
	dm_gpio_set_value(&pinfo->reset, false);

	/* Power the panel */
	ret = dm_gpio_set_value(&pinfo->hsvcc, true);
	if (ret) {
		return ret;
	}

	mdelay(1);
	ret = dm_gpio_set_value(&pinfo->vspn3v3, true);
	if (ret) {
		return ret;
	}
	mdelay(1);

	dm_gpio_set_value(&pinfo->reset, true);
	mdelay(10);

	pinfo->prepared = true;

	return 0;
}

static int jadard_enable(struct udevice *panel)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;
	struct panel_info *pinfo = dev_get_priv(panel);
	u8 power_mode;
	int ret;

	if (pinfo->enabled)
		return 0;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	/* sanity test for connection */
	ret = mipi_dsi_dcs_get_power_mode(dsi, &power_mode);
	if (ret) {
		dev_warn(dsi->dev, "%s: failed to get power mode: %d\n", __func__, ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret)
		return ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret)
	{
		return ret;
	}

	mdelay(10);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret){
		return ret;
	}

	pinfo->enabled = true;

	return 0;
}

static int jd9365_panel_enable(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat   = dev_get_platdata(dev);
	struct mipi_dsi_device     *device = plat->device;
	int ret;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	ret = jadard_enable(dev);
	if (ret)
		return ret;

	return 0;
}

static const struct display_timing txd_jd9365_timing = {
	.pixelclock.typ		= 74250000,
	.hactive.typ		= 800,
	.hfront_porch.typ	= 60,
	.hback_porch.typ	= 60,
	.hsync_len.typ		= 40,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 16,
	.vback_porch.typ	= 16,
	.vsync_len.typ		= 8,
	.flags			=  DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW,
};

static const struct jadard_panel_desc jd9365_panel_desc = {
	.timing = &txd_jd9365_timing,
	.mode_flags = MIPI_DSI_MODE_VIDEO |  MIPI_DSI_MODE_VIDEO_BURST,
	.format = MIPI_DSI_FMT_RGB888,
	.lanes = 4,
};

static int jd9365_panel_ofdata_to_platdata(struct udevice *dev)
{
	struct panel_info *pinfo = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpio", 0,
			&pinfo->reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get reset GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	ret = gpio_request_by_name(dev, "hsvcc-gpio", 0,
			&pinfo->hsvcc, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get hsvcc GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	ret = gpio_request_by_name(dev, "vspn3v3-gpio", 0,
			&pinfo->vspn3v3, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get vspn3v3 GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	return 0;
}

static int jadard_dsi_probe(struct udevice *panel)
{
	int ret;
	struct panel_info *pinfo = dev_get_priv(panel);

	pinfo->desc = (const struct jadard_panel_desc*)dev_get_driver_data(panel);

	ret = jadard_prepare(panel);
	if (ret) {
		dev_err(panel, "failed to prepare panel : %d\n", ret);
		return ret;
	}

	return 0;
}

static int jadard_dsi_remove(struct udevice *panel)
{
	return 0;
}

static const struct panel_ops jd9365_panel_ops = {
	.enable_backlight	= jd9365_panel_enable,
	.get_display_timing	= jd9365_get_display_timing,
};

static const struct udevice_id panel_of_match[] = {
	{
		.compatible = "jadard,jd9365da-h3",
		.data = (ulong)&jd9365_panel_desc,
	},
	{
		/* sentinel */
	}
};

U_BOOT_DRIVER(jadard_jd9365da) = {
	.name 			  = "jadard_jd9365da",
	.id			      = UCLASS_PANEL,
	.of_match		  = panel_of_match,
	.ops			  = &jd9365_panel_ops,
	.ofdata_to_platdata	  = jd9365_panel_ofdata_to_platdata,
	.probe			  = jadard_dsi_probe,
	.remove			  = jadard_dsi_remove,
	.platdata_auto_alloc_size = sizeof(struct mipi_dsi_panel_plat),
	.priv_auto_alloc_size	  = sizeof(struct panel_info),
};

