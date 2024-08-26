/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mcu/mcu-uclass.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

int _mcu_shutdown(struct udevice *dev)
{
	struct mcu_ops *ops = dev_get_driver_ops(dev);

	if (!ops->shutdown)
		return -ENOSYS;

	return ops->shutdown(dev);
}

int _mcu_poweron(struct udevice *dev)
{
	struct mcu_ops *ops = dev_get_driver_ops(dev);

	if (!ops->poweron)
		return -ENOSYS;

	return ops->poweron(dev);
}

int mcu_poweron(void)
{
	struct udevice *mcu;
	int ret;

	ret = uclass_get_device(UCLASS_MCU, 0, &mcu);
	if (ret) {
		printf("Get UCLASS_MCU failed, ret=%d\n", ret);
		return ret;
	}

	return _mcu_poweron(mcu);
}

int mcu_shutdown(void)
{
	struct udevice *mcu;
	int ret;

	ret = uclass_get_device(UCLASS_MCU, 0, &mcu);
	if (ret) {
		printf("Get charge display failed, ret=%d\n", ret);
		return ret;
	}

	return _mcu_shutdown(mcu);
}

UCLASS_DRIVER(mcu) = {
	.id		= UCLASS_MCU,
	.name		= "mcu",
};
