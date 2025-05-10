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

struct gpio_desc *gpio_leds[2];
int major = 0;
static struct class *key_dev_class;
static char *led_name[2] = {"diy_led", "work_led"};
static int gpio_num[2];
#define DEV_NAME "key_diy"
#define MINOR_NUM 2
#define GROUP(x) 		(x / 32)
#define PIN(x)			(x % 32)

int key_diy_open(struct inode * inode, struct file *file) {
	printk("Open device %s\r\n", led_name[iminor(inode)]);

	return 0;
}
ssize_t key_diy_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {
	char value = 0;
	int err = 0;

	value = gpio_get_value(gpio_num[iminor(file_inode(file))]);
	err = copy_to_user(buf, &value, sizeof(value));

	printk("Read %s value is %d\r\n", led_name[iminor(file_inode(file))], value);

	return 0;
}

ssize_t key_diy_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	char value = 0;
	int err = 0;

	err = copy_from_user(&value, buf, sizeof(value));
	gpio_set_value(gpio_num[iminor(file_inode(file))], value);

	printk("Write %s value is %d\r\n", led_name[iminor(file_inode(file))], value);

	return 0;
}

int key_diy_release(struct inode *inode, struct file *file) {
	printk("Release device %s\r\n", led_name[iminor(inode)]);

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
	struct device_node *np = pdev->dev.of_node;
	struct device_node *diyled_np, *workled_np;
	int i = 0;

	major = register_chrdev(0, DEV_NAME, &key_fops);
	key_dev_class = class_create(THIS_MODULE, "key-dev");
	for(i=0; i<MINOR_NUM; i++) {	
		device_create(key_dev_class, NULL, MKDEV(major, i), NULL, "%s", led_name[i]);
		printk("device_create %s\r\n", led_name[i]);
	}

	diyled_np = of_get_child_by_name(np, "diyled");
	workled_np = of_get_child_by_name(np, "workled");

	gpio_num[0] = of_get_named_gpio(diyled_np, "key-gpios", 0);
	gpio_num[1] = of_get_named_gpio(workled_np, "key-gpios", 0);

	printk("diyled_pin is %d-GPIO%d_%c%d\r\n", gpio_num[0], GROUP(gpio_num[0]), 'A' + PIN(gpio_num[0]) / 8, PIN(gpio_num[0]) % 8);
	printk("workled_pin is %d-GPIO%d_%c%d\r\n", gpio_num[1], GROUP(gpio_num[1]), 'A' + PIN(gpio_num[1]) / 8, PIN(gpio_num[1]) % 8);

	gpio_request(gpio_num[0], "diyled");
	gpio_request(gpio_num[1], "workled");

	gpio_direction_output(gpio_num[0], 0);
	gpio_direction_output(gpio_num[1], 0);

	gpio_set_value(gpio_num[0], 1);
	gpio_set_value(gpio_num[1], 1);

	return 0;
}

static int rk3399_key_remove(struct platform_device * pdev) {
	int i = 0;

	gpio_set_value(gpio_num[0], 0);
	gpio_set_value(gpio_num[1], 0);
	
    gpio_free(gpio_num[0]);
    gpio_free(gpio_num[1]);

	for(i=0; i<MINOR_NUM; i++) {
		device_destroy(key_dev_class, MKDEV(major, i));
	}
	class_destroy(key_dev_class);
	unregister_chrdev(major, DEV_NAME);

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
		/* 匹配设备树 */
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