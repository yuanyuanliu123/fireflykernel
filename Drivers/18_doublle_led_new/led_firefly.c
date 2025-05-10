#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include "led_operation.h"

#define LED_NAME "diy_led"
// #define LED_NUM 2

static int major = 0;
static struct class *led_class;
struct led_operations *p_ledopr;

static int led_open(struct inode *inode, struct file *file) {
	printk("%s  %s  %d led device open\r\n", __FILE__, __FUNCTION__, __LINE__);
	
	return 0;
}

static ssize_t led_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {
	printk("%s  %s  %d led device read\r\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	int err;
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("%s  %s  %d led device write\r\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);

	/*Control LED by status and sub-device number.*/
	p_ledopr->ctl(minor, status);
	
	return 1;
}

static int led_release(struct inode *inode, struct file *file) { 
	printk("%s  %s  %d led device release\r\n", __FILE__, __FUNCTION__, __LINE__);

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
	int i = 0;
	printk("led init.\r\n");

	major = register_chrdev(0, LED_NAME, &led_fops);

	led_class = class_create(THIS_MODULE, "LED_CLASS");

	p_ledopr = get_board_led_opr();

	for(i=0; i<p_ledopr->num; i++) {
		device_create(led_class, NULL, MKDEV(major, i), NULL, "diy_led_%d", i);
	}

	/*Init LED device by sub-device number */
	p_ledopr->init();

	return 0;
}

static void __exit led_exit(void) 
{
	int i = 0;
	printk("led exit!!!\r\n");

	for(i=0; i<p_ledopr->num; i++) {
		device_destroy(led_class, MKDEV(major, i));
		
	}

	class_destroy(led_class); 
	
	unregister_chrdev(major, LED_NAME);

	
	p_ledopr->exit();
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liu");
MODULE_INFO(intree, "Y");
