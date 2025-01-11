#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>

struct gpio_desc *gpio_key;
int major = 0;
static struct class *key_dev_class;
#define DEV_NAME "key_diy"

int key_diy_open(struct inode * inode, struct file *file) {
	gpiod_direction_output(gpio_key, 1);

	printk("Set gpio mode output\r\n");
	
	return 0;
}
ssize_t key_diy_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {
	char statu;
	int err;

	statu = gpiod_get_value(gpio_key);
	err = copy_to_user(buf, &statu, 1);

	printk("Get gpio level statu is %d\r\n", statu);

	return 0;
}

ssize_t key_diy_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	char statu;
	int err;

	err = copy_from_user(&statu, buf, 1);
	gpiod_set_value(gpio_key, statu);

	printk("Set gpio level statu is %d\r\n", statu);
	
	return 0;
}

int key_diy_release(struct inode *inode, struct file *file) {

	printk("Release\r\n");

	return 0;
}

const struct file_operations key_fops = {
	.owner = THIS_MODULE,
	.open = key_diy_open,
	.read = key_diy_read,
	.write = key_diy_write,
	.release = key_diy_release,
};

static int rk3399_key_probe(struct platform_device * pdev) {
	major = register_chrdev(0, DEV_NAME, &key_fops);
	key_dev_class = class_create(THIS_MODULE, "key-dev");
	device_create(key_dev_class, NULL, MKDEV(major, 0), NULL, "key_diy0");

	gpio_key = gpiod_get(&pdev->dev, "key", 1);

	return 0;
}

static int rk3399_key_remove(struct platform_device * pdev) {
	device_destroy(key_dev_class, MKDEV(major, 0));
	class_destroy(key_dev_class);
	unregister_chrdev(major, DEV_NAME);

	gpiod_put(gpio_key);

	return 0;
}

static const struct of_device_id firefly_rk3399_key[] = {
	{ .compatible = "rk3399, keydrv" },
	{ },
};

struct platform_driver rk3399_key_drv = {
	.probe = rk3399_key_probe,
	.remove = rk3399_key_remove,
	.driver = {
		.name = "rk3399",
		.of_match_table = firefly_rk3399_key,
	},
};

static int __init key_drv_init(void)
{
	return platform_driver_register(&rk3399_key_drv);
}

static void __exit key_drv_exit(void)
{
	platform_driver_unregister(&rk3399_key_drv);
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");