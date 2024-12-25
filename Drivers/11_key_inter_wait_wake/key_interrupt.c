#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

struct gpio_key {
	int gpio;
	int irq;
	enum of_gpio_flags flags;
};
static int major = 0;
static struct class *led_class;
static unsigned int key_signal = 0;
static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);
struct gpio_key *gpios_key;


static int key_open(struct inode *inode, struct file *file) {
	printk("%s  %s  %d led device open\r\n", __FILE__, __FUNCTION__, __LINE__);
	
	return 0;
}

static ssize_t key_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {
	int err;
	
	printk("%s  %s  %d led device read\r\n", __FILE__, __FUNCTION__, __LINE__);
	
	wait_event_interruptible(gpio_key_wait, key_signal);
	printk("led read : key statu is %d\r\n", key_signal);

	err = copy_to_user(buf, &key_signal, 4);
	key_signal = 0;
	
	return 0;
}

static ssize_t key_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	printk("%s  %s  %d led device write\r\n", __FILE__, __FUNCTION__, __LINE__);
	
	return 0;
}

static int key_release(struct inode *inode, struct file *file) {
	printk("%s  %s  %d led device release\r\n", __FILE__, __FUNCTION__, __LINE__);
	
	return 0;
}

const struct file_operations key_fops = {
	.owner = THIS_MODULE,
	.open = key_open,	
	.release = key_release,	
	.read = key_read,	
	.write = key_write,
};

static irqreturn_t gpio_key_rk3399(int irq, void *dev_id)
{
	struct gpio_key *gpios_key1 = dev_id;

	key_signal = ((gpio_get_value(gpios_key1->gpio)) << 8) |(gpios_key1->gpio);
	printk("key %d val %d\r\n", irq, gpio_get_value(gpios_key1->gpio));
	wake_up_interruptible(&gpio_key_wait);

	return IRQ_HANDLED;
}

static int rk3399_key_probe(struct platform_device *pdev) {
	int count = 0;
	int i = 0;
	int err = 0;

	printk("enter interrupt\r\n");
	count = of_gpio_count(pdev->dev.of_node);
	printk("count is %d\r\n", count);
	gpios_key = kzalloc(count * sizeof(struct gpio_key), GFP_KERNEL);
	for(i=0; i<count; i++) {
		gpios_key[i].gpio = of_get_gpio_flags(pdev->dev.of_node, i, &(gpios_key[i].flags));
		gpios_key[i].irq = gpio_to_irq(gpios_key[i].gpio);
		err = request_irq(gpios_key[i].irq, gpio_key_rk3399, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "rk3399_key", &gpios_key[i]);
	}

	major = register_chrdev(0, "KEY_L", &key_fops);
	led_class = class_create(THIS_MODULE, "KEY_CLASS");
	device_create(led_class, NULL, MKDEV(major, 0), NULL, "diy_key_double");

	return err;
}

static int rk3399_key_remove(struct platform_device *pdev) {
	int count = 0;
	int i = 0;
	
	count = of_gpio_count(pdev->dev.of_node);
	for(i=0; i<count; i++) {
		free_irq(gpios_key[i].irq, &gpios_key[i]);
	}
	
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class); 
	unregister_chrdev(major, "KEY_L");
	
	return 0;
}


static const struct of_device_id firefly_rk3399_key[] = {
	{ .compatible = "rk3399, keydrv" },
	{},
};

struct platform_driver key_interrupt_drv = {
	.probe = rk3399_key_probe,
	.remove = rk3399_key_remove,
	.driver = {
		.name = "rk3399",
		.of_match_table = firefly_rk3399_key,
	},
};

static int gpio_interrupt_init(void) {
	return platform_driver_register(&key_interrupt_drv);
}

static void gpio_interrupt_exit(void) {
	platform_driver_unregister(&key_interrupt_drv);
}

module_init(gpio_interrupt_init);
module_exit(gpio_interrupt_exit);
MODULE_LICENSE("GPL");