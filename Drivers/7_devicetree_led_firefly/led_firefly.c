#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include "led_firefly.h"

#define LED_NAME "diy_led"

static int major = 0;
static struct class *led_class;
struct led_operations *p_led_opr;

void led_class_create_device(int minor, const char *name_led)
{
	device_create(led_class, NULL, MKDEV(major, minor), NULL, "%s", name_led); 

	printk("crete subdevice name %s\r\n", name_led);
}

void led_class_destroy_device(int minor)
{
	device_destroy(led_class, MKDEV(major, minor));

	printk("subdevice %d destroyed\r\n", minor);
}

void register_led_operations(struct led_operations *opr)
{
	p_led_opr = opr;
}

EXPORT_SYMBOL(led_class_create_device);
EXPORT_SYMBOL(led_class_destroy_device);
EXPORT_SYMBOL(register_led_operations);

static int led_open(struct inode *inode, struct file *file) {
	int minor = iminor(inode);
	
	printk("led subdevice %d open\r\n", minor);
	
	p_led_opr->init(minor);
	
	return 0;
}

static ssize_t led_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {

	return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	int err;
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("led subdevice %d write\r\n", minor);
	
	err = copy_from_user(&status, buf, 1);

	/*Control LED by status and sub-device number.*/
	p_led_opr->ctl(minor, status);
	
	return 1;
}

static int led_release(struct inode *inode, struct file *file) { 
	int minor = iminor(inode);
	
	printk("led device release\r\n");
	
	p_led_opr->exit(minor);
	
	return 0;
}

const struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_release,
	.read = led_read,
	.write = led_write,
};

static int __init led_init(void)
{
	major = register_chrdev(0, LED_NAME, &led_fops);

	printk("led major number is %d.\r\n", major);

	led_class = class_create(THIS_MODULE, "LED_CLASS");

	return 0;
}

static void __exit led_exit(void) 
{
	printk("led exit!!!\r\n");

	class_destroy(led_class); 
	
	unregister_chrdev(major, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liu");
MODULE_INFO(intree, "Y");
