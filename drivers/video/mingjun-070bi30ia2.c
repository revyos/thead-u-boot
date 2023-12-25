// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>

struct mingjun_panel_cmd {
	char cmdlen;
	char cmddata[0x40];
};

static const struct mingjun_panel_cmd mingjun_on_cmds[] = {
	// { .cmdlen = 4,	.cmddata = {0xB9, 0xFF, 0x83, 0x94} },
	// { .cmdlen = 11,	.cmddata = {0xB1, 0x48, 0x0A, 0x6A, 0x09, 0x33, 0x54,
	// 			0x71, 0x71, 0x2E, 0x45} },
	// { .cmdlen = 7,	.cmddata = {0xBA, 0x63, 0x03, 0x68, 0x6B, 0xB2, 0xC0} },
	// { .cmdlen = 7,	.cmddata = {0xB2, 0x00, 0x80, 0x64, 0x0C, 0x06, 0x2F} },
	// { .cmdlen = 22, .cmddata = {0xB4, 0x1C, 0x78, 0x1C, 0x78, 0x1C, 0x78, 0x01,
	// 			0x0C, 0x86, 0x75, 0x00, 0x3F, 0x1C, 0x78, 0x1C,
	// 			0x78, 0x1C, 0x78, 0x01, 0x0C, 0x86} },
	// { .cmdlen = 34, .cmddata = {0xD3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
	// 			0x08, 0x32, 0x10, 0x05, 0x00, 0x05, 0x32, 0x13,
	// 			0xC1, 0x00, 0x01, 0x32, 0x10, 0x08, 0x00, 0x00,
	// 			0x37, 0x03, 0x07, 0x07, 0x37, 0x05, 0x05, 0x37,
	// 			0x0C, 0x40} },
	// { .cmdlen = 45, .cmddata = {0xD5, 0x18, 0x18, 0x18, 0x18, 0x22, 0x23, 0x20,
	// 			0x21, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02,
	// 			0x03, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	// 			0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	// 			0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	// 			0x18, 0x19, 0x19, 0x19, 0x19} },
	// { .cmdlen = 45, .cmddata = {0xD6, 0x18, 0x18, 0x19, 0x19, 0x21, 0x20, 0x23,
	// 			0x22, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05,
	// 			0x04, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	// 			0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	// 			0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	// 			0x18, 0x19, 0x19, 0x18, 0x18} },
	// { .cmdlen = 59, .cmddata = {0xE0, 0x07, 0x08, 0x09, 0x0D, 0x10, 0x14, 0x16,
	// 			0x13, 0x24, 0x36, 0x48, 0x4A, 0x58, 0x6F, 0x76,
	// 			0x80, 0x97, 0xA5, 0xA8, 0xB5, 0xC6, 0x62, 0x63,
	// 			0x68, 0x6F, 0x72, 0x78, 0x7F, 0x7F, 0x00, 0x02,
	// 			0x08, 0x0D, 0x0C, 0x0E, 0x0F, 0x10, 0x24, 0x36,
	// 			0x48, 0x4A, 0x58, 0x6F, 0x78, 0x82, 0x99, 0xA4,
	// 			0xA0, 0xB1, 0xC0, 0x5E, 0x5E, 0x64, 0x6B, 0x6C,
	// 			0x73, 0x7F, 0x7F} },
	// { .cmdlen = 2, .cmddata = {0xCC, 0x03} },
	// { .cmdlen = 3, .cmddata = {0xC0, 0x1F, 0x73} },
	// { .cmdlen = 3, .cmddata = {0xB6, 0x90, 0x90} },
	// { .cmdlen = 2, .cmddata = {0xD4, 0x02} },
	// { .cmdlen = 2, .cmddata = {0xBD, 0x01} },
	// { .cmdlen = 2, .cmddata = {0xB1, 0x00} },
	// { .cmdlen = 2, .cmddata = {0xBD, 0x00} },
	// { .cmdlen = 8, .cmddn bata = {0xBF, 0x40, 0x81, 0x50, 0x00, 0x1A, 0xFC, 0x01} },

	// { .cmdlen = 2, .cmddata = {0x36, 0x02} },
{ .cmdlen =4, .cmddata = {0xFF,0x98,0x81,0x03} },
{ .cmdlen = 2, .cmddata = {0x01,0x00} },
{ .cmdlen = 2, .cmddata = {0x02,0x00} },
{ .cmdlen = 2, .cmddata = {0x03,0x73} },
{ .cmdlen = 2, .cmddata = {0x04,0x13} },
{ .cmdlen = 2, .cmddata = {0x05,0x00} },
{ .cmdlen = 2, .cmddata = {0x06,0x0A} },
{ .cmdlen = 2, .cmddata = {0x07,0x05} },
{ .cmdlen = 2, .cmddata = {0x11,0x00} },
{ .cmdlen = 2, .cmddata = {0x09,0x28} },
{ .cmdlen = 2, .cmddata = {0x0A,0x00} },
{ .cmdlen = 2, .cmddata = {0x0B,0x00} },
{ .cmdlen = 2, .cmddata = {0x0C,0x00} },
{ .cmdlen = 2, .cmddata = {0x0D,0x28} },
{ .cmdlen = 2, .cmddata = {0x0E,0x00} },
{ .cmdlen = 2, .cmddata = {0x0F,0x28} },
{ .cmdlen = 2, .cmddata = {0x10,0x28} },
{ .cmdlen = 2, .cmddata = {0x11,0x00} },
{ .cmdlen = 2, .cmddata = {0x12,0x00} },
{ .cmdlen = 2, .cmddata = {0x13,0x00} },
{ .cmdlen = 2, .cmddata = {0x14,0x00} },
{ .cmdlen = 2, .cmddata = {0x15,0x00} },
{ .cmdlen = 2, .cmddata = {0x16,0x00} },
{ .cmdlen = 2, .cmddata = {0x17,0x00} },
{ .cmdlen = 2, .cmddata = {0x18,0x00} },
{ .cmdlen = 2, .cmddata = {0x19,0x00} },
{ .cmdlen = 2, .cmddata = {0x1A,0x00} },
{ .cmdlen = 2, .cmddata = {0x1B,0x00} },
{ .cmdlen = 2, .cmddata = {0x1C,0x00} },
{ .cmdlen = 2, .cmddata = {0x1D,0x00} },
{ .cmdlen = 2, .cmddata = {0x1E,0x40} },
{ .cmdlen = 2, .cmddata = {0x1F,0x80} },
{ .cmdlen = 2, .cmddata = {0x20,0x06} },
{ .cmdlen = 2, .cmddata = {0x21,0x01} },
{ .cmdlen = 2, .cmddata = {0x22,0x00} },
{ .cmdlen = 2, .cmddata = {0x23,0x00} },
{ .cmdlen = 2, .cmddata = {0x24,0x00} },
{ .cmdlen = 2, .cmddata = {0x25,0x00} },
{ .cmdlen = 2, .cmddata = {0x26,0x00} },
{ .cmdlen = 2, .cmddata = {0x27,0x00} },
{ .cmdlen = 2, .cmddata = {0x28,0x33} },
{ .cmdlen = 2, .cmddata = {0x29,0x33} },
{ .cmdlen = 2, .cmddata = {0x2A,0x00} },
{ .cmdlen = 2, .cmddata = {0x2B,0x00} },
{ .cmdlen = 2, .cmddata = {0x2C,0x04} },
{ .cmdlen = 2, .cmddata = {0x2D,0x0C} },
{ .cmdlen = 2, .cmddata = {0x2E,0x05} },
{ .cmdlen = 2, .cmddata = {0x2F,0x05} },
{ .cmdlen = 2, .cmddata = {0x30,0x00} },
{ .cmdlen = 2, .cmddata = {0x31,0x00} },
{ .cmdlen = 2, .cmddata = {0x32,0x31} },
{ .cmdlen = 2, .cmddata = {0x33,0x00} },
{ .cmdlen = 2, .cmddata = {0x34,0x00} },
{ .cmdlen = 2, .cmddata = {0x35,0x0A} },
{ .cmdlen = 2, .cmddata = {0x36,0x00} },
{ .cmdlen = 2, .cmddata = {0x37,0x08} },
{ .cmdlen = 2, .cmddata = {0x70,0x00} },
{ .cmdlen = 2, .cmddata = {0x39,0x00} },
{ .cmdlen = 2, .cmddata = {0x3A,0x00} },
{ .cmdlen = 2, .cmddata = {0x3B,0x00} },
{ .cmdlen = 2, .cmddata = {0x3C,0x00} },
{ .cmdlen = 2, .cmddata = {0x3D,0x00} },
{ .cmdlen = 2, .cmddata = {0x3E,0x00} },
{ .cmdlen = 2, .cmddata = {0x3F,0x00} },
{ .cmdlen = 2, .cmddata = {0x40,0x00} },
{ .cmdlen = 2, .cmddata = {0x41,0x00} },
{ .cmdlen = 2, .cmddata = {0x42,0x00} },
{ .cmdlen = 2, .cmddata = {0x43,0x08} },
{ .cmdlen = 2, .cmddata = {0x44,0x00} },
{ .cmdlen = 2, .cmddata = {0xA0,0x02} },
{ .cmdlen = 2, .cmddata = {0x51,0x23} },
{ .cmdlen = 2, .cmddata = {0x52,0x44} },
{ .cmdlen = 2, .cmddata = {0x53,0x67} },
{ .cmdlen = 2, .cmddata = {0x54,0x89} },
{ .cmdlen = 2, .cmddata = {0x55,0xAB} },
{ .cmdlen = 2, .cmddata = {0x56,0x01} },
{ .cmdlen = 2, .cmddata = {0x57,0x23} },
{ .cmdlen = 2, .cmddata = {0x58,0x45} },
{ .cmdlen = 2, .cmddata = {0x59,0x67} },
{ .cmdlen = 2, .cmddata = {0x5A,0x89} },
{ .cmdlen = 2, .cmddata = {0x5B,0xAB} },
{ .cmdlen = 2, .cmddata = {0x5C,0xCD} },
{ .cmdlen = 2, .cmddata = {0x5D,0xEF} },
{ .cmdlen = 2, .cmddata = {0x5E,0x11} },
{ .cmdlen = 2, .cmddata = {0x5F,0x02} },
{ .cmdlen = 2, .cmddata = {0x60,0x08} },
{ .cmdlen = 2, .cmddata = {0x61,0x0E} },
{ .cmdlen = 2, .cmddata = {0x62,0x0F} },
{ .cmdlen = 2, .cmddata = {0x63,0x0C} },
{ .cmdlen = 2, .cmddata = {0x64,0x0D} },
{ .cmdlen = 2, .cmddata = {0x65,0x17} },
{ .cmdlen = 2, .cmddata = {0x66,0x01} },
{ .cmdlen = 2, .cmddata = {0x67,0x01} },
{ .cmdlen = 2, .cmddata = {0x68,0x02} },
{ .cmdlen = 2, .cmddata = {0x69,0x02} },
{ .cmdlen = 2, .cmddata = {0x6A,0x00} },
{ .cmdlen = 2, .cmddata = {0x6B,0x00} },
{ .cmdlen = 2, .cmddata = {0x6C,0x02} },
{ .cmdlen = 2, .cmddata = {0x6D,0x02} },
{ .cmdlen = 2, .cmddata = {0x6E,0x16} },
{ .cmdlen = 2, .cmddata = {0x6F,0x16} },
{ .cmdlen = 2, .cmddata = {0x70,0x06} },
{ .cmdlen = 2, .cmddata = {0x71,0x06} },
{ .cmdlen = 2, .cmddata = {0x72,0x07} },
{ .cmdlen = 2, .cmddata = {0x73,0x07} },
{ .cmdlen = 2, .cmddata = {0x74,0x02} },
{ .cmdlen = 2, .cmddata = {0x75,0x02} },
{ .cmdlen = 2, .cmddata = {0x76,0x08} },
{ .cmdlen = 2, .cmddata = {0x77,0x0E} },
{ .cmdlen = 2, .cmddata = {0x78,0x0F} },
{ .cmdlen = 2, .cmddata = {0x79,0x0C} },
{ .cmdlen = 2, .cmddata = {0x7A,0x0D} },
{ .cmdlen = 2, .cmddata = {0x7B,0x17} },
{ .cmdlen = 2, .cmddata = {0x7C,0x01} },
{ .cmdlen = 2, .cmddata = {0x7D,0x01} },
{ .cmdlen = 2, .cmddata = {0x7E,0x02} },
{ .cmdlen = 2, .cmddata = {0x7F,0x02} },
{ .cmdlen = 2, .cmddata = {0x80,0x00} },
{ .cmdlen = 2, .cmddata = {0x81,0x00} },
{ .cmdlen = 2, .cmddata = {0x82,0x02} },
{ .cmdlen = 2, .cmddata = {0x83,0x02} },
{ .cmdlen = 2, .cmddata = {0x84,0x16} },
{ .cmdlen = 2, .cmddata = {0x85,0x16} },
{ .cmdlen = 2, .cmddata = {0x86,0x06} },
{ .cmdlen = 2, .cmddata = {0x87,0x06} },
{ .cmdlen = 2, .cmddata = {0x88,0x07} },
{ .cmdlen = 2, .cmddata = {0x89,0x07} },
{ .cmdlen = 2, .cmddata = {0x8A,0x02} },
{ .cmdlen = 4, .cmddata = {0xFF,0x98,0x81,0x04} },
{ .cmdlen = 2, .cmddata = {0x6E,0x1A} },
{ .cmdlen = 2, .cmddata = {0x6F,0x37} },
{ .cmdlen = 2, .cmddata = {0x3A,0xA4} },
{ .cmdlen = 2, .cmddata = {0x8D,0x1F} },
{ .cmdlen = 2, .cmddata = {0x87,0xBA} },
{ .cmdlen = 2, .cmddata = {0xB2,0xD1} },
{ .cmdlen = 2, .cmddata = {0x88,0x0B} },
{ .cmdlen = 2, .cmddata = {0x38,0x01} },
{ .cmdlen = 2, .cmddata = {0x39,0x00} },
{ .cmdlen = 2, .cmddata = {0xB5,0x02} },
{ .cmdlen = 2, .cmddata = {0x31,0x25} },
{ .cmdlen = 2, .cmddata = {0x3B,0x98} },
{ .cmdlen = 4, .cmddata = {0xFF,0x98,0x81,0x01} },
{ .cmdlen = 2, .cmddata = {0x22,0x0A} },
{ .cmdlen = 2, .cmddata = {0x31,0x00} },
{ .cmdlen = 2, .cmddata = {0xA6,0xA6} },
{ .cmdlen = 2, .cmddata = {0x55,0x3D} },
{ .cmdlen = 2, .cmddata = {0x50,0x9E} },
{ .cmdlen = 2, .cmddata = {0x51,0x99} },
{ .cmdlen = 2, .cmddata = {0x60,0x06} },
{ .cmdlen = 2, .cmddata = {0x62,0x20} },
{ .cmdlen = 2, .cmddata = {0xA0,0x00} },
{ .cmdlen = 2, .cmddata = {0xA1,0x17} },
{ .cmdlen = 2, .cmddata = {0xA2,0x26} },
{ .cmdlen = 2, .cmddata = {0xA3,0x13} },
{ .cmdlen = 2, .cmddata = {0xA4,0x16} },
{ .cmdlen = 2, .cmddata = {0xA5,0x29} },
{ .cmdlen = 2, .cmddata = {0xA6,0x1E} },
{ .cmdlen = 2, .cmddata = {0xA7,0x1F} },
{ .cmdlen = 2, .cmddata = {0xA8,0x8B} },
{ .cmdlen = 2, .cmddata = {0xA9,0x1D} },
{ .cmdlen = 2, .cmddata = {0xAA,0x2A} },
{ .cmdlen = 2, .cmddata = {0xAB,0x7B} },
{ .cmdlen = 2, .cmddata = {0xAC,0x1A} },
{ .cmdlen = 2, .cmddata = {0xAD,0x19} },
{ .cmdlen = 2, .cmddata = {0xAE,0x4E} },
{ .cmdlen = 2, .cmddata = {0xAF,0x24} },
{ .cmdlen = 2, .cmddata = {0xB0,0x29} },
{ .cmdlen = 2, .cmddata = {0xB1,0x4F} },
{ .cmdlen = 2, .cmddata = {0xB2,0x5C} },
{ .cmdlen = 2, .cmddata = {0xB3,0x3E} },
{ .cmdlen = 2, .cmddata = {0xC0,0x00} },
{ .cmdlen = 2, .cmddata = {0xC1,0x17} },
{ .cmdlen = 2, .cmddata = {0xC2,0x26} },
{ .cmdlen = 2, .cmddata = {0xC3,0x13} },
{ .cmdlen = 2, .cmddata = {0xC4,0x16} },
{ .cmdlen = 2, .cmddata = {0xC5,0x29} },
{ .cmdlen = 2, .cmddata = {0xC6,0x1E} },
{ .cmdlen = 2, .cmddata = {0xC7,0x1F} },
{ .cmdlen = 2, .cmddata = {0xC8,0x8B} },
{ .cmdlen = 2, .cmddata = {0xC9,0x1D} },
{ .cmdlen = 2, .cmddata = {0xCA,0x2A} },
{ .cmdlen = 2, .cmddata = {0xCB,0x7B} },
{ .cmdlen = 2, .cmddata = {0xCC,0x1A} },
{ .cmdlen = 2, .cmddata = {0xCD,0x19} },
{ .cmdlen = 2, .cmddata = {0xCE,0x4E} },
{ .cmdlen = 2, .cmddata = {0xCF,0x24} },
{ .cmdlen = 2, .cmddata = {0xD0,0x29} },
{ .cmdlen = 2, .cmddata = {0xD1,0x4D} },
{ .cmdlen = 2, .cmddata = {0xD2,0x5C} },
{ .cmdlen = 2, .cmddata = {0xD3,0x3E} },
{ .cmdlen = 4, .cmddata = {0xFF,0x98,0x81,0x00} },
{ .cmdlen = 2, .cmddata = {0x11,0x00} },
{ .cmdlen = 2, .cmddata = {0x29,0x00} },
{ .cmdlen = 2, .cmddata = {0x35,0x00} },
{ .cmdlen = 2, .cmddata = {0x00,0x00} },
};


struct mj070bi30ia2_desc {
	const struct mingjun_panel_cmd *on_cmds;
	unsigned int on_cmds_num;
	const struct display_timing *timing;
};

struct mj070bi30ia2_panel_priv {
	struct udevice *backlight;
	struct gpio_desc reset;
	const struct mj070bi30ia2_desc *desc;
};

static const struct display_timing mj070bi30ia2_timing = {
	.pixelclock.typ		= 75750000,
	.hactive.typ		= 800,
	.hfront_porch.typ	= 48,
	.hback_porch.typ	= 80,
	.hsync_len.typ		= 32,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 3,
	.vback_porch.typ	= 24,
	.vsync_len.typ		= 10,
	.flags			= DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW,
};

static int mingjun_send_mipi_cmds(struct udevice *panel,
					struct mingjun_panel_cmd *cmds)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(panel);
	int ret;
	int i;

	for (i = 0; i < priv->desc->on_cmds_num; i++) {
		ret = mipi_dsi_dcs_write_buffer(dsi,
				&(cmds[i].cmddata[0]), cmds[i].cmdlen);
		if (ret < 0)
			return ret;
	}
	dev_info(dsi->dev, "%s: send initial instruction\n", __func__);

	return 0;
}

static int mj070bi30ia2_panel_setup(struct udevice *panel)
{
	int ret;
	u8 power_mode;
	const struct mj070bi30ia2_instr *instr;
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(panel);
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	/* sanity test for connection */
	ret = mipi_dsi_dcs_get_power_mode(dsi, &power_mode);
	if (ret) {
		dev_warn(dsi->dev, "%s: failed to get power mode: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int mj070bi30ia2_panel_prepare(struct udevice *panel)
{
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(panel);
	int ret;

	/* reset panel */
	ret = dm_gpio_set_value(&priv->reset, true);
	if (ret)
		return ret;
	mdelay(1);

	ret = dm_gpio_set_value(&priv->reset, false);
	if (ret)
		return ret;
	mdelay(10);

	return 0;
}

static int mj070bi30ia2_panel_enable(struct udevice *panel)
{
	int ret;
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(panel);
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mingjun_send_mipi_cmds(panel, priv->desc->on_cmds);
	if (ret < 0) {
		dev_err(panel->dev, "failed to send DCS Init Code: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret)
		return ret;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret)
		return ret;

	mdelay(10);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret)
		return ret;

#if 0
	ret = backlight_enable(priv->backlight);
	if (ret)
		return ret;
#endif

	return 0;
}

static int mj070bi30ia2_panel_enable_backlight(struct udevice *dev)
{
	int ret;
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(dev);
	struct mipi_dsi_device *device = plat->device;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	ret = mj070bi30ia2_panel_setup(dev);
	if (ret)
		return ret;

	ret = mj070bi30ia2_panel_enable(dev);
	if (ret)
		return ret;

	return 0;
}

static int mj070bi30ia2_panel_get_display_timing(struct udevice *dev,
					     struct display_timing *timings)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(dev);
	struct mipi_dsi_device *device = plat->device;
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(dev);

	memcpy(timings, priv->desc->timing, sizeof(*timings));

	device->lanes	= 4;
	device->format	= MIPI_DSI_FMT_RGB888;
	device->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST;

	return 0;
}

static int mj070bi30ia2_panel_ofdata_to_platdata(struct udevice *dev)
{
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
			&priv->reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get reset GPIO\n");
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		dev_err(dev, "Cannot get backlight: ret=%d\n", ret);
	}

	/* TODO: get lanes, format and mode from dtb */

	return 0;
}

static int mj070bi30ia2_panel_probe(struct udevice *panel)
{
	int ret;
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(panel);

	priv->desc = (const struct mj070bi30ia2_desc *)dev_get_driver_data(panel);

	/* prepare_panel */
	ret = mj070bi30ia2_panel_prepare(panel);
	if (ret) {
		dev_err(panel, "failed to prepare panel : %d\n", ret);
		return ret;
	}

	return 0;
}

static int mj070bi30ia2_panel_remove(struct udevice *panel)
{
	struct mj070bi30ia2_panel_priv *priv = dev_get_priv(panel);

	return dm_gpio_set_value(&priv->reset, true);
}

static const struct mj070bi30ia2_desc mj070bi30ia2_desc = {
	.on_cmds = mingjun_on_cmds,
	.on_cmds_num = ARRAY_SIZE(mingjun_on_cmds),
        .timing = &mj070bi30ia2_timing,
};

static const struct panel_ops mj070bi30ia2_panel_ops = {
	.enable_backlight	= mj070bi30ia2_panel_enable_backlight,
	.get_display_timing	= mj070bi30ia2_panel_get_display_timing,
};

static const struct udevice_id mj070bi30ia2_panel_ids[] = {
	{ .compatible = "mingjun,mj070bi30ia2", .data = (ulong)&mj070bi30ia2_desc },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mj070bi30ia2_panel) = {
	.name 			  = "mj070bi30ia2_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = mj070bi30ia2_panel_ids,
	.ops			  = &mj070bi30ia2_panel_ops,
	.ofdata_to_platdata	  = mj070bi30ia2_panel_ofdata_to_platdata,
	.probe			  = mj070bi30ia2_panel_probe,
	.remove			  = mj070bi30ia2_panel_remove,
	.platdata_auto_alloc_size = sizeof(struct mipi_dsi_panel_plat),
	.priv_auto_alloc_size	  = sizeof(struct mj070bi30ia2_panel_priv),
};
