#include "led_resource.h"

static struct resource led_resource[] = {
	{
		.start = GPIO_PIN(0, 13),
		.flags = IORESOURCE_IRQ,
		.name = "led_diy",
	},
	{
		.start = GPIO_PIN(2, 27),
		.flags = IORESOURCE_IRQ,
		.name = "led_work",
	},	
};

static struct platform_device board_rk3399_led_dev = {
	.name = "rk3399_led",
	.num_resources = ARRAY_SIZE(led_resource),
	.resource = led_resource,
};

static int led_dev_init(void) {
	int err;
	
	printk("register platform device\r\n");
	err = platform_device_register(&board_rk3399_led_dev);
	
	return 0;
}

static void led_dev_exit(void) {

	printk("unregister platform device\r\n");
	platform_device_unregister(&board_rk3399_led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");