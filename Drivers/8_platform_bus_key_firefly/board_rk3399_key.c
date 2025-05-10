#include "key_resource.h"

static struct resource key_resource[] = {
	{
		.start = GPIO_PIN(0, 12),
		.flags = IORESOURCE_IRQ,
		.name = "key_diy",
	},
	{
		.start = GPIO_PIN(4, 29),
		.flags = IORESOURCE_IRQ,
		.name = "key_work",
	},	
};

static struct platform_device board_rk3399_key_dev = {
	.name = "rk3399_key",
	.num_resources = ARRAY_SIZE(key_resource),
	.resource = key_resource,
};

static int key_dev_init(void) {
	int err;
	
	printk("register platform device\r\n");
	err = platform_device_register(&board_rk3399_key_dev);
	
	return 0;
}

static void key_dev_exit(void) {

	printk("unregister platform device\r\n");
	platform_device_unregister(&board_rk3399_key_dev);
}

module_init(key_dev_init);
module_exit(key_dev_exit);
MODULE_LICENSE("GPL");