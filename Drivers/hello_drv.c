#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>

static int major = 0;
static char kernel_buf[1024];
static struct class *hello_class;

#define MIN(a, b) (a > b ? a : b)

static int hello_drv_open (struct inode *node, struct file *file)
{	
	printk("%s %s line %s", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static ssize_t  hello_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %s", __FILE__, __FUNCTION__, __LINE__);

	copy_to_user(buf, kernel_buf, MIN(size, 1024));
}

static ssize_t  hello_drv_write (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %s", __FILE__, __FUNCTION__, __LINE__);

	copy_from_user(kernel_buf, buf, MIN(size, 1024));
}

static int  hello_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %s", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static struct file_operations hello_drv {
	.owner = THIS_MODULE,
	.open = hello_drv_open,
	.read = hello_drv_read,
	.write = hello_drv_write,
	.release = hello_drv_close,
};

// 

static int __init hello_init(void)
{
	major = register_chrdev(0, "hello", &hello_drv);

	// create device
	hello_class = class_create(THIS_MODULE, "hello");
	err = PTR_ERR(hello_class);
	if (IS_ERR(hello_class)){
		unregister_chrdev(major, "hello");
		return -1;
	}
	device_create(hello_class, NULL, MKDEV(major, 0), NULL, "hello");
	
	return 0;
}

static void __exit hello_exit(void)
{
	device_destroy(hello_class, MKDEV(major, 0));
	class_destroy(hello_class);
	unregister_chrdev(major, "hello");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");








