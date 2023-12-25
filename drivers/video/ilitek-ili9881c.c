// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <asm/gpio.h>

enum ili9881c_op {
	ILI9881C_SWITCH_PAGE,
	ILI9881C_COMMAND,
};

struct ili9881c_instr {
	enum ili9881c_op        op;

	union arg {
		struct cmd {
			u8      cmd;
			u8      data;
		} cmd;
		u8      page;
	} arg;
};

struct ili9881c_desc {
	const struct ili9881c_instr *init;
	const size_t init_length;
	const struct display_timing *timing;
};

struct ili9881c_panel_priv {
	struct udevice *backlight;
	struct gpio_desc reset;
	struct gpio_desc lcd_en;
	struct gpio_desc lcd_bias_en;
	const struct ili9881c_desc *desc;
};

#define ILI9881C_SWITCH_PAGE_INSTR(_page)       \
        {                                       \
                .op = ILI9881C_SWITCH_PAGE,     \
                .arg = {                        \
                        .page = (_page),        \
                },                              \
        }

#define ILI9881C_COMMAND_INSTR(_cmd, _data)             \
        {                                               \
                .op = ILI9881C_COMMAND,         \
                .arg = {                                \
                        .cmd = {                        \
                                .cmd = (_cmd),          \
                                .data = (_data),        \
                        },                              \
                },                                      \
        }

static const struct display_timing txd_dy800qwxpab_timing = {
	.pixelclock.typ		= 74250000,
	.hactive.typ		= 800,
	.hfront_porch.typ	= 60,
	.hback_porch.typ	= 60,
	.hsync_len.typ		= 40,
	.vactive.typ		= 1280,
	.vfront_porch.typ	= 16,
	.vback_porch.typ	= 16,
	.vsync_len.typ		= 8,
	.flags			= DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW,
};

static const struct ili9881c_instr txd_dy800qwxpab_init[] = {
	ILI9881C_SWITCH_PAGE_INSTR(3),
	ILI9881C_COMMAND_INSTR(0x01, 0x00),
	ILI9881C_COMMAND_INSTR(0x02, 0x00),
	ILI9881C_COMMAND_INSTR(0x03, 0x53),
	ILI9881C_COMMAND_INSTR(0x04, 0x53),
	ILI9881C_COMMAND_INSTR(0x05, 0x13),
	ILI9881C_COMMAND_INSTR(0x06, 0x04),
	ILI9881C_COMMAND_INSTR(0x07, 0x02),
	ILI9881C_COMMAND_INSTR(0x08, 0x02),
	ILI9881C_COMMAND_INSTR(0x09, 0x00),
	ILI9881C_COMMAND_INSTR(0x0A, 0x00),
	ILI9881C_COMMAND_INSTR(0x0B, 0x00),
	ILI9881C_COMMAND_INSTR(0x0C, 0x00),
	ILI9881C_COMMAND_INSTR(0x0D, 0x00),
	ILI9881C_COMMAND_INSTR(0x0E, 0x00),
	ILI9881C_COMMAND_INSTR(0x0F, 0x00),
	ILI9881C_COMMAND_INSTR(0x10, 0x00),
	ILI9881C_COMMAND_INSTR(0x11, 0x00),
	ILI9881C_COMMAND_INSTR(0x12, 0x00),
	ILI9881C_COMMAND_INSTR(0x13, 0x00),
	ILI9881C_COMMAND_INSTR(0x14, 0x00),
	ILI9881C_COMMAND_INSTR(0x15, 0x00),
	ILI9881C_COMMAND_INSTR(0x16, 0x00),
	ILI9881C_COMMAND_INSTR(0x17, 0x00),
	ILI9881C_COMMAND_INSTR(0x18, 0x00),
	ILI9881C_COMMAND_INSTR(0x19, 0x00),
	ILI9881C_COMMAND_INSTR(0x1A, 0x00),
	ILI9881C_COMMAND_INSTR(0x1B, 0x00),
	ILI9881C_COMMAND_INSTR(0x1C, 0x00),
	ILI9881C_COMMAND_INSTR(0x1D, 0x00),
	ILI9881C_COMMAND_INSTR(0x1E, 0xC0),
	ILI9881C_COMMAND_INSTR(0x1F, 0x80),
	ILI9881C_COMMAND_INSTR(0x20, 0x02),
	ILI9881C_COMMAND_INSTR(0x21, 0x09),
	ILI9881C_COMMAND_INSTR(0x22, 0x00),
	ILI9881C_COMMAND_INSTR(0x23, 0x00),
	ILI9881C_COMMAND_INSTR(0x24, 0x00),
	ILI9881C_COMMAND_INSTR(0x25, 0x00),
	ILI9881C_COMMAND_INSTR(0x26, 0x00),
	ILI9881C_COMMAND_INSTR(0x27, 0x00),
	ILI9881C_COMMAND_INSTR(0x28, 0x55),
	ILI9881C_COMMAND_INSTR(0x29, 0x03),
	ILI9881C_COMMAND_INSTR(0x2A, 0x00),
	ILI9881C_COMMAND_INSTR(0x2B, 0x00),
	ILI9881C_COMMAND_INSTR(0x2C, 0x00),
	ILI9881C_COMMAND_INSTR(0x2D, 0x00),
	ILI9881C_COMMAND_INSTR(0x2E, 0x00),
	ILI9881C_COMMAND_INSTR(0x2F, 0x00),
	ILI9881C_COMMAND_INSTR(0x30, 0x00),
	ILI9881C_COMMAND_INSTR(0x31, 0x00),
	ILI9881C_COMMAND_INSTR(0x32, 0x00),
	ILI9881C_COMMAND_INSTR(0x33, 0x00),
	ILI9881C_COMMAND_INSTR(0x34, 0x00), /* GPWR1/2 non overlap time 2.62us */
	ILI9881C_COMMAND_INSTR(0x35, 0x00),
	ILI9881C_COMMAND_INSTR(0x36, 0x00),
	ILI9881C_COMMAND_INSTR(0x37, 0x00),
	ILI9881C_COMMAND_INSTR(0x38, 0x3C),
	ILI9881C_COMMAND_INSTR(0x39, 0x00),
	ILI9881C_COMMAND_INSTR(0x3A, 0x00),
	ILI9881C_COMMAND_INSTR(0x3B, 0x00),
	ILI9881C_COMMAND_INSTR(0x3C, 0x00),
	ILI9881C_COMMAND_INSTR(0x3D, 0x00),
	ILI9881C_COMMAND_INSTR(0x3E, 0x00),
	ILI9881C_COMMAND_INSTR(0x3F, 0x00),
	ILI9881C_COMMAND_INSTR(0x40, 0x00),
	ILI9881C_COMMAND_INSTR(0x41, 0x00),
	ILI9881C_COMMAND_INSTR(0x42, 0x00),
	ILI9881C_COMMAND_INSTR(0x43, 0x00),
	ILI9881C_COMMAND_INSTR(0x44, 0x00),
	ILI9881C_COMMAND_INSTR(0x50, 0x01),
	ILI9881C_COMMAND_INSTR(0x51, 0x23),
	ILI9881C_COMMAND_INSTR(0x52, 0x45),
	ILI9881C_COMMAND_INSTR(0x53, 0x67),
	ILI9881C_COMMAND_INSTR(0x54, 0x89),
	ILI9881C_COMMAND_INSTR(0x55, 0xAB),
	ILI9881C_COMMAND_INSTR(0x56, 0x01),
	ILI9881C_COMMAND_INSTR(0x57, 0x23),
	ILI9881C_COMMAND_INSTR(0x58, 0x45),
	ILI9881C_COMMAND_INSTR(0x59, 0x67),
	ILI9881C_COMMAND_INSTR(0x5A, 0x89),
	ILI9881C_COMMAND_INSTR(0x5B, 0xAB),
	ILI9881C_COMMAND_INSTR(0x5C, 0xCD),
	ILI9881C_COMMAND_INSTR(0x5D, 0xEF),
	ILI9881C_COMMAND_INSTR(0x5E, 0x01),
	ILI9881C_COMMAND_INSTR(0x5F, 0x08),
	ILI9881C_COMMAND_INSTR(0x60, 0x02),
	ILI9881C_COMMAND_INSTR(0x61, 0x02),
	ILI9881C_COMMAND_INSTR(0x62, 0x0A),
	ILI9881C_COMMAND_INSTR(0x63, 0x15),
	ILI9881C_COMMAND_INSTR(0x64, 0x14),
	ILI9881C_COMMAND_INSTR(0x65, 0x02),
	ILI9881C_COMMAND_INSTR(0x66, 0x11),
	ILI9881C_COMMAND_INSTR(0x67, 0x10),
	ILI9881C_COMMAND_INSTR(0x68, 0x02),
	ILI9881C_COMMAND_INSTR(0x69, 0x0F),
	ILI9881C_COMMAND_INSTR(0x6A, 0x0E),
	ILI9881C_COMMAND_INSTR(0x6B, 0x02),
	ILI9881C_COMMAND_INSTR(0x6C, 0x0D),
	ILI9881C_COMMAND_INSTR(0x6D, 0x0C),
	ILI9881C_COMMAND_INSTR(0x6E, 0x06),
	ILI9881C_COMMAND_INSTR(0x6F, 0x02),
	ILI9881C_COMMAND_INSTR(0x70, 0x02),
	ILI9881C_COMMAND_INSTR(0x71, 0x02),
	ILI9881C_COMMAND_INSTR(0x72, 0x02),
	ILI9881C_COMMAND_INSTR(0x73, 0x02),
	ILI9881C_COMMAND_INSTR(0x74, 0x02),
	ILI9881C_COMMAND_INSTR(0x75, 0x06),
	ILI9881C_COMMAND_INSTR(0x76, 0x02),
	ILI9881C_COMMAND_INSTR(0x77, 0x02),
	ILI9881C_COMMAND_INSTR(0x78, 0x0A),
	ILI9881C_COMMAND_INSTR(0x79, 0x15),
	ILI9881C_COMMAND_INSTR(0x7A, 0x14),
	ILI9881C_COMMAND_INSTR(0x7B, 0x02),
	ILI9881C_COMMAND_INSTR(0x7C, 0x10),
	ILI9881C_COMMAND_INSTR(0x7D, 0x11),
	ILI9881C_COMMAND_INSTR(0x7E, 0x02),
	ILI9881C_COMMAND_INSTR(0x7F, 0x0C),
	ILI9881C_COMMAND_INSTR(0x80, 0x0D),
	ILI9881C_COMMAND_INSTR(0x81, 0x02),
	ILI9881C_COMMAND_INSTR(0x82, 0x0E),
	ILI9881C_COMMAND_INSTR(0x83, 0x0F),
	ILI9881C_COMMAND_INSTR(0x84, 0x08),
	ILI9881C_COMMAND_INSTR(0x85, 0x02),
	ILI9881C_COMMAND_INSTR(0x86, 0x02),
	ILI9881C_COMMAND_INSTR(0x87, 0x02),
	ILI9881C_COMMAND_INSTR(0x88, 0x02),
	ILI9881C_COMMAND_INSTR(0x89, 0x02),
	ILI9881C_COMMAND_INSTR(0x8A, 0x02),
	ILI9881C_SWITCH_PAGE_INSTR(4),
	ILI9881C_COMMAND_INSTR(0x6C, 0x15), /* Set VCORE voltage = 1.5V */
	ILI9881C_COMMAND_INSTR(0x6E, 0x30), /* di_pwr_reg=0 for power mode 2A, VGH clamp 18V */
	ILI9881C_COMMAND_INSTR(0x6F, 0x47), /* pumping ratio VGH=5x VGL=-3x */
	ILI9881C_COMMAND_INSTR(0x8D, 0x1F), /* VGL clamp -10V */
	ILI9881C_COMMAND_INSTR(0x87, 0xBA), /* ESD */
	ILI9881C_COMMAND_INSTR(0x26, 0x76),
	ILI9881C_COMMAND_INSTR(0xB2, 0xD1),
	ILI9881C_COMMAND_INSTR(0x35, 0x1F),
	ILI9881C_COMMAND_INSTR(0x31, 0x75),
	ILI9881C_COMMAND_INSTR(0x33, 0x14),
	ILI9881C_COMMAND_INSTR(0x3A, 0xA9),
	ILI9881C_COMMAND_INSTR(0x3B, 0x98),
	ILI9881C_COMMAND_INSTR(0x38, 0x01),
	ILI9881C_COMMAND_INSTR(0x39, 0x00),
	ILI9881C_SWITCH_PAGE_INSTR(1),
	ILI9881C_COMMAND_INSTR(0x22, 0x0A), /* BGR, SS */
	ILI9881C_COMMAND_INSTR(0x31, 0x00), /* Zigzag type3 inversion */
	ILI9881C_COMMAND_INSTR(0x50, 0xCF),
	ILI9881C_COMMAND_INSTR(0x51, 0xCA),
	ILI9881C_COMMAND_INSTR(0x53, 0x3C),
	ILI9881C_COMMAND_INSTR(0x55, 0x3C),
	ILI9881C_COMMAND_INSTR(0x60, 0x28),
	ILI9881C_COMMAND_INSTR(0x2E, 0xC8),
	ILI9881C_COMMAND_INSTR(0x34, 0x01),
	ILI9881C_COMMAND_INSTR(0xA0, 0x00),
	ILI9881C_COMMAND_INSTR(0xA1, 0x08), /* VP251 */
	ILI9881C_COMMAND_INSTR(0xA2, 0x18), /* VP247 */
	ILI9881C_COMMAND_INSTR(0xA3, 0x15), /* VP243 */
	ILI9881C_COMMAND_INSTR(0xA4, 0x19), /* VP239 */
	ILI9881C_COMMAND_INSTR(0xA5, 0x2D), /* VP231 */
	ILI9881C_COMMAND_INSTR(0xA6, 0x21), /* VP219 */
	ILI9881C_COMMAND_INSTR(0xA7, 0x23), /* VP203 */
	ILI9881C_COMMAND_INSTR(0xA8, 0x74), /* VP175 */
	ILI9881C_COMMAND_INSTR(0xA9, 0x1C), /* VP144 */
	ILI9881C_COMMAND_INSTR(0xAA, 0x26), /* VP111 */
	ILI9881C_COMMAND_INSTR(0xAB, 0x64), /* VP80 */
	ILI9881C_COMMAND_INSTR(0xAC, 0x1A), /* VP52 */
	ILI9881C_COMMAND_INSTR(0xAD, 0x1C), /* VP36 */
	ILI9881C_COMMAND_INSTR(0xAE, 0x50), /* VP24 */
	ILI9881C_COMMAND_INSTR(0xAF, 0x25), /* VP16 */
	ILI9881C_COMMAND_INSTR(0xB0, 0x2A), /* VP12 */
	ILI9881C_COMMAND_INSTR(0xB1, 0x4C), /* VP8 */
	ILI9881C_COMMAND_INSTR(0xB2, 0x5D), /* VP4 */
	ILI9881C_COMMAND_INSTR(0xB3, 0x23), /* VP0 */
	ILI9881C_COMMAND_INSTR(0xC0, 0x00), /* VN255 GAMMA N */
	ILI9881C_COMMAND_INSTR(0xC1, 0x1B), /* VN251 */
	ILI9881C_COMMAND_INSTR(0xC2, 0x27), /* VN247 */
	ILI9881C_COMMAND_INSTR(0xC3, 0x13), /* VN243 */
	ILI9881C_COMMAND_INSTR(0xC4, 0x16), /* VN239 */
	ILI9881C_COMMAND_INSTR(0xC5, 0x28), /* VN231 */
	ILI9881C_COMMAND_INSTR(0xC6, 0x1E), /* VN219 */
	ILI9881C_COMMAND_INSTR(0xC7, 0x20), /* VN203 */
	ILI9881C_COMMAND_INSTR(0xC8, 0x7D), /* VN175 */
	ILI9881C_COMMAND_INSTR(0xC9, 0x19), /* VN144 */
	ILI9881C_COMMAND_INSTR(0xCA, 0x27), /* VN111 */
	ILI9881C_COMMAND_INSTR(0xCB, 0x65), /* VN80 */
	ILI9881C_COMMAND_INSTR(0xCC, 0x1D), /* VN52 */
	ILI9881C_COMMAND_INSTR(0xCD, 0x1D), /* VN36 */
	ILI9881C_COMMAND_INSTR(0xCE, 0x52), /* VN24 */
	ILI9881C_COMMAND_INSTR(0xCF, 0x26), /* VN16 */
	ILI9881C_COMMAND_INSTR(0xD0, 0x2C), /* VN12 */
	ILI9881C_COMMAND_INSTR(0xD1, 0x4E), /* VN8 */
	ILI9881C_COMMAND_INSTR(0xD2, 0x59), /* VN4 */
	ILI9881C_COMMAND_INSTR(0xD3, 0x23), /* VN0 */
};

/*
 * The panel seems to accept some private DCS commands that map
 * directly to registers.
 *
 * It is organised by page, with each page having its own set of
 * registers, and the first page looks like it's holding the standard
 * DCS commands.
 *
 * So before any attempt at sending a command or data, we have to be
 * sure if we're in the right page or not.
 */
static int ili9881c_switch_page(struct udevice *panel, u8 page)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;
	u8 buf[4] = { 0xff, 0x98, 0x81, page };
	int ret;

	ret = mipi_dsi_dcs_write_buffer(dsi, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return 0;
}

static int ili9881c_send_cmd_data(struct udevice *panel, u8 cmd, u8 data)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;
	u8 buf[2] = { cmd, data };
	int ret;

	ret = mipi_dsi_dcs_write_buffer(dsi, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return 0;
}

static int ili9881c_panel_setup(struct udevice *panel)
{
	int ret;
	u8 power_mode;
	unsigned int i;
	const struct ili9881c_instr *instr;
	struct ili9881c_panel_priv *priv = dev_get_priv(panel);
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	/* sanity test for connection */
	ret = mipi_dsi_dcs_get_power_mode(dsi, &power_mode);
	if (ret) {
		dev_warn(dsi->dev, "%s: failed to get power mode: %d\n", __func__, ret);
		return ret;
	}

	for (i = 0; i < priv->desc->init_length; i++) {
		instr = &priv->desc->init[i];

		switch(instr->op) {
		case ILI9881C_SWITCH_PAGE:
			ret = ili9881c_switch_page(panel, instr->arg.page);
			if (ret)
				return ret;
			break;
		case ILI9881C_COMMAND:
			ret = ili9881c_send_cmd_data(panel,
					instr->arg.cmd.cmd,
					instr->arg.cmd.data);
			if (ret)
				return ret;
			break;
		default:
			return -EINVAL;
		}
	}

	/* TODO: set pixel format */

	return 0;
}

static int ili9881c_panel_prepare(struct udevice *panel)
{
	struct ili9881c_panel_priv *priv = dev_get_priv(panel);
	int ret;

	/* power on */
	ret = dm_gpio_set_value(&priv->lcd_en, true);
	if (ret)
		return ret;

	ret = dm_gpio_set_value(&priv->lcd_bias_en, true);
	if (ret)
		return ret;

	/* reset panel */
	ret = dm_gpio_set_value(&priv->reset, true);
	if (ret)
		return ret;
	mdelay(500);

	ret = dm_gpio_set_value(&priv->reset, false);
	if (ret)
		return ret;
	mdelay(100);

	return 0;
}

static int ili9881c_panel_enable(struct udevice *panel)
{
	int ret;
#if 0
	struct ili9881c_panel_priv *priv = dev_get_priv(panel);
#endif
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(panel);
	struct mipi_dsi_device *dsi = plat->device;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = ili9881c_switch_page(panel, 0);
	if (ret)
		return ret;

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

static int ili9881c_panel_enable_backlight(struct udevice *dev)
{
	int ret;
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(dev);
	struct mipi_dsi_device *device = plat->device;

	ret = mipi_dsi_attach(device);
	if (ret < 0)
		return ret;

	ret = ili9881c_panel_setup(dev);
	if (ret)
		return ret;

	ret = ili9881c_panel_enable(dev);
	if (ret)
		return ret;

	return 0;
}

static int ili9881c_panel_get_display_timing(struct udevice *dev,
					     struct display_timing *timings)
{
	struct mipi_dsi_panel_plat *plat = dev_get_platdata(dev);
	struct mipi_dsi_device *device = plat->device;
	struct ili9881c_panel_priv *priv = dev_get_priv(dev);

	memcpy(timings, priv->desc->timing, sizeof(*timings));

	device->lanes	= 4;
	device->format	= MIPI_DSI_FMT_RGB888;
	device->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST;

	return 0;
}

static int ili9881c_panel_ofdata_to_platdata(struct udevice *dev)
{
	struct ili9881c_panel_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
			&priv->reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get reset GPIO\n");
		if (ret != -ENOENT)
			return ret;
	} else {
		/* not a bug, but uboot's regulator is buggy,
		I haven't more time to fix it, so put it here
		*/
		/* reset panel */
		ret = dm_gpio_set_value(&priv->reset, false);
		if (ret)
			return ret;
		mdelay(100);

		ret = dm_gpio_set_value(&priv->reset, true);
		if (ret)
			return ret;
		mdelay(100);
	}

	/* power gpios */
	ret = gpio_request_by_name(dev, "lcd-en-gpios", 0,
			&priv->lcd_en, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get lcd-en GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

	ret = gpio_request_by_name(dev, "lcd-bias-en-gpios", 0,
			&priv->lcd_bias_en, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "Warning: cannot get lcd-bias-en GPIO\n");
		if (ret != -ENOENT)
			return ret;
	}

#if 0
	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		dev_err(dev, "Cannot get backlight: ret=%d\n", ret);
		return ret;
	}
#endif

	/* TODO: get lanes, format and mode from dtb */

	return 0;
}

static int ili9881c_panel_probe(struct udevice *panel)
{
	int ret;
	struct ili9881c_panel_priv *priv = dev_get_priv(panel);

	priv->desc = (const struct ili9881c_desc *)dev_get_driver_data(panel);

	/* prepare_panel */
	ret = ili9881c_panel_prepare(panel);
	if (ret) {
		dev_err(panel, "failed to prepare panel : %d\n", ret);
		return ret;
	}

	return 0;
}

static int ili9881c_panel_remove(struct udevice *panel)
{
	struct ili9881c_panel_priv *priv = dev_get_priv(panel);

	return dm_gpio_set_value(&priv->reset, true);
}

static const struct ili9881c_desc txd_dy800qwxpab_desc = {
        .init = txd_dy800qwxpab_init,
        .init_length = ARRAY_SIZE(txd_dy800qwxpab_init),
        .timing = &txd_dy800qwxpab_timing,
};

static const struct panel_ops ili9881c_panel_ops = {
	.enable_backlight	= ili9881c_panel_enable_backlight,
	.get_display_timing	= ili9881c_panel_get_display_timing,
};

static const struct udevice_id ili9881c_panel_ids[] = {
	{ .compatible = "ilitek,ili9881c", .data = (ulong)&txd_dy800qwxpab_desc },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ili9881c_panel) = {
	.name 			  = "ili9881c_panel",
	.id			  = UCLASS_PANEL,
	.of_match		  = ili9881c_panel_ids,
	.ops			  = &ili9881c_panel_ops,
	.ofdata_to_platdata	  = ili9881c_panel_ofdata_to_platdata,
	.probe			  = ili9881c_panel_probe,
	.remove			  = ili9881c_panel_remove,
	.platdata_auto_alloc_size = sizeof(struct mipi_dsi_panel_plat),
	.priv_auto_alloc_size	  = sizeof(struct ili9881c_panel_priv),
};
