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
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <linux/slab.h>

static int major;
static struct class *mmap_class;
static char *kernel_buf;
static int bufsize = 1024 * 8; 

static int mmap_open (struct inode *inode, struct file *filp) {
	printk("Enter Open function\r\n");
	
	return 0;
}

static ssize_t mmap_read (struct file *filp, char __user *buf, size_t size, loff_t *offset) {
	int err;
	
	printk("Enter Read function\r\n");

	err = copy_to_user(buf, kernel_buf, size);

	return err;
}

static ssize_t mmap_write (struct file *filp, const char __user *buf, size_t size, loff_t *offset) {
	char app_info[64];
	int err;
	
	printk("Enter Write function\r\n");

	err = copy_from_user(app_info, buf, size);
	printk("It's send string %s \r\n", app_info);
	
	return err;
}

static int mmap_release (struct inode *inode, struct file *filp) {
	printk("Enter Release function\r\n");

	return 0;
}

static int mmap_mmap (struct file *filp, struct vm_area_struct *vma) {
	unsigned long phy;

	/* 获得物理地址 */
	phy = virt_to_phys(kernel_buf);
	/* 设置属性 使用cache 0 buffer 1 */
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	// vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	/* 映射 */
	remap_pfn_range(vma, vma->vm_start, phy >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);

	printk("mmap %s\r\n", kernel_buf);
	
	return 0;
}

static struct file_operations fops_mmap = {
	.owner = THIS_MODULE,
	.open = mmap_open,
	.read = mmap_read,
	.write = mmap_write,
	.release = mmap_release,
	.mmap = mmap_mmap,
};

static int __init mmap_drv_init(void) {
	int err = 0;
	printk("mmap init function\r\n");
	kernel_buf = kmalloc(bufsize, GFP_KERNEL);
	// kernel_buf = (char *)__get_free_pages(GFP_KERNEL, get_order(bufsize));
	major = register_chrdev(0, "mmap_diy", &fops_mmap);
	mmap_class = class_create(THIS_MODULE, "mmap_class");
	err = PTR_ERR(mmap_class);
	if(IS_ERR(mmap_class)) {
		printk("class_create error\r\n");
		unregister_chrdev(major, "mmap_diy");
		return -1;
	}
	device_create(mmap_class, NULL, MKDEV(major, 0), NULL, "mmap");

	return 0;
}

static void __exit mmap_drv_exit(void) {
	device_destroy(mmap_class, MKDEV(major, 0));
	class_destroy(mmap_class);
	unregister_chrdev(major, "mmap_diy");
	kfree(kernel_buf);
	// free_pages((unsigned long)kernel_buf, get_order(bufsize));
}

module_init(mmap_drv_init);
module_exit(mmap_drv_exit);
MODULE_LICENSE("GPL");