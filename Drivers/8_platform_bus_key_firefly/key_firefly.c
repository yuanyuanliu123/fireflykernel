#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include "key_firefly.h"

#define KEY_NAME "diy_key"

static int major = 0;
static struct class *key_class;
struct key_operations *p_key_opr;

void key_class_create_device(int minor, const char *name_key)
{
	device_create(key_class, NULL, MKDEV(major, minor), NULL, "%s", name_key); 

	printk("crete subdevice name %s\r\n", name_key);
}

void key_class_destroy_device(int minor)
{
	device_destroy(key_class, MKDEV(major, minor));

	printk("subdevice %d destroyed\r\n", minor);
}

void register_key_operations(struct key_operations *opr)
{
	p_key_opr = opr;
}

EXPORT_SYMBOL(key_class_create_device);
EXPORT_SYMBOL(key_class_destroy_device);
EXPORT_SYMBOL(register_key_operations);

static int key_open(struct inode *inode, struct file *file) {
	int minor = iminor(inode);
	
	printk("key subdevice %d open\r\n", minor);
	
	p_key_opr->init(minor);
	
	return 0;
}

static ssize_t key_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {
	
	char status;
	int err;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("key subdevice %d write\r\n", minor);
	
	status = p_key_opr->ctl(minor);

	printk("current pin level is %d\n\r", status);
	
	err = copy_to_user(buf, &status, 1);
	
	return 0;
}

static ssize_t key_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	// int err;
	/*
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);
	
	printk("key subdevice %d write\r\n", minor);
	
	err = copy_from_user(&status, buf, 1);
	
	p_key_opr->ctl(minor, status);
	*/
	return 1;
}

static int key_release(struct inode *inode, struct file *file) { 
	int minor = iminor(inode);
	
	printk("key device release\r\n");
	
	p_key_opr->exit(minor);
	
	return 0;
}

const struct file_operations key_fops = {
	.owner = THIS_MODULE,
	.open = key_open,
	.release = key_release,
	.read = key_read,
	.write = key_write,
};

static int __init key_diy_init(void)
{
	major = register_chrdev(0, KEY_NAME, &key_fops);

	printk("key major number is %d.\r\n", major);

	key_class = class_create(THIS_MODULE, "KEY_CLASS");

	return 0;
}

static void __exit key_diy_exit(void) 
{
	printk("key exit!!!\r\n");

	class_destroy(key_class); 
	
	unregister_chrdev(major, KEY_NAME);
}

module_init(key_diy_init);
module_exit(key_diy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liu");
MODULE_INFO(intree, "Y");
