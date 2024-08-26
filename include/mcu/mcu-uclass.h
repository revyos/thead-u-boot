#ifndef __MCU_H
#define __MCU_H

struct mcu_ops {
	int (*shutdown)(struct udevice *dev);
	int (*poweron)(struct udevice *dev);
};

/**
 * mcu_shutdown() - power off supplies
 *
 * @return 0 on success or negative value of errno.
 */
int mcu_shutdown(void);

/**
 * mcu_poweron() - power on supplies
 *
 * @return 0 on success or negative value of errno.
 */
int mcu_poweron(void);

#endif
