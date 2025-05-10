#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include "led_operation.h"

static int board_demo_init(int which) {
	printk("%s %s %d led %d\r\n", __FILE__, __FUNCTION__, __LINE__, which);

	return 0;
}

static int board_demo_ctl(int which, char status) {
	printk("%s %s %d led %d %s\r\n", __FILE__, __FUNCTION__, __LINE__, which, status ? "on" : "off");

	return 0;
}

static struct led_operations board_demo_led_opr = {
	.init = board_demo_init,
	.ctl = board_demo_ctl,
};

struct led_operations *get_board_led_opr(void) {

	return &board_demo_led_opr;
}