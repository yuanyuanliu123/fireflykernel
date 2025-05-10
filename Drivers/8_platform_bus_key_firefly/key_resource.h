#ifndef __KEY_RES__
#define __KEY_RES__

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#define GPIO_PIN(x, y) 	 ((x << 16) | (y))
#define GROUP(x) 		(x >> 16)
#define PIN(x)			 (x & 0x0000ffff)

#endif
