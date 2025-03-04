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
#define DEV_NAME "key_diy"
#define MINOR_NUM 2
#define GROUP(x) 		(x / 32)
#define PIN(x)			(x % 32)

int key_diy_open(struct inode * inode, struct file *file) {
	/* iminor 通过inode获取次设备号 */
	printk("Open device %s\r\n", led_name[iminor(inode)]);

	return 0;
}
ssize_t key_diy_read(struct file *file, char __user *buf, size_t cnt, loff_t *offt) {
	char value = 0;
	int err = 0;

	/* 获取当前GPIO电平值 file_inode 通过文件信息结构体获取 inode */
	value = gpiod_get_value(gpio_leds[iminor(file_inode(file))]);
	err = copy_to_user(buf, &value, sizeof(value));

	printk("Read %s value is %d\r\n", led_name[iminor(file_inode(file))], value);

	return 0;
}

ssize_t key_diy_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offt) {
	char value = 0;
	int err = 0;

	err = copy_from_user(&value, buf, sizeof(value));
	/* 设置当前GPIO电平值 */
	gpiod_set_value(gpio_leds[iminor(file_inode(file))], value);

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

/* 匹配完成之后会调用probe函数（匹配到几个调用几次probe函数）：
    struct platform_device {
        struct device	dev;		//父类
    };
    struct device {
        struct device_node	*of_node;
    };
*/
static int rk3399_key_probe(struct platform_device * pdev) {
	int i = 0;

	/* 注册字符设备驱动，创建主设备号 */
	major = register_chrdev(0, DEV_NAME, &key_fops);
	/* 创建字符设备类 */
	key_dev_class = class_create(THIS_MODULE, "key-dev");
	for(i=0; i<MINOR_NUM; i++) {	
		/* 创建设备子节点 : diy and work */
		device_create(key_dev_class, NULL, MKDEV(major, i), NULL, "%s", led_name[i]);
		printk("device_create %s\r\n", led_name[i]);
	}

	// 使用 gpiod_get() 获取 GPIO 描述符
	gpio_leds[0] = gpiod_get(&pdev->dev, "key", 0);
    if (IS_ERR(gpio_leds[0])) {
        printk("Failed to get GPIO descriptor for diyled\n");
        return PTR_ERR(gpio_leds[0]);
    }
	gpio_leds[1] = gpiod_get(&pdev->dev, "key", 1);
    if (IS_ERR(gpio_leds[1])) {
        printk("Failed to get GPIO descriptor for workled\n");
        gpiod_put(gpio_leds[0]);  // 释放已获取的 GPIO 描述符
        return PTR_ERR(gpio_leds[1]);
    }

	// 设置 GPIO 引脚为输出
	gpiod_direction_output(gpio_leds[0], 0);
	gpiod_direction_output(gpio_leds[1], 0);

	// 初始化 GPIO 为高电平
	gpiod_set_value(gpio_leds[0], 1);
	gpiod_set_value(gpio_leds[1], 1);

	return 0;
}

static int rk3399_key_remove(struct platform_device * pdev) {
	int i = 0;
    /* 和probe调用顺序相反 */
	/* GPIO置零 */
	gpiod_set_value(gpio_leds[0], 0);
	gpiod_set_value(gpio_leds[1], 0);
	
	/* 释放GPIO */
	// 释放 GPIO 描述符
	gpiod_put(gpio_leds[0]);
	gpiod_put(gpio_leds[1]);
    /* 销毁设备子节点 */
	for(i=0; i<MINOR_NUM; i++) {
		device_destroy(key_dev_class, MKDEV(major, i));
	}
	/* 销毁字符设备类 */
	class_destroy(key_dev_class);
	/* 注销字符设备驱动 */
	unregister_chrdev(major, DEV_NAME);

	return 0;
}

static const struct of_device_id firefly_rk3399_key[] = {
	{ .compatible = "rk3399, keydrv" },
	{ },
};

/* 对于被注册的 platform driver 当发生匹配时，将以struct platform_device作为参数调用XXXX_probe函数 */
struct platform_driver rk3399_key_drv = {
	.probe = rk3399_key_probe,
	.remove = rk3399_key_remove,
	.driver = {
		.name = "rk3399",
		/* 设备树匹配节点 */
		.of_match_table = firefly_rk3399_key,
	},
};

static int __init key_drv_init(void)
{
	/* 注册platform driver 将platform_driver中的driver.of_match_table.compatible 与设备树节点下的compatible匹配 */
	return platform_driver_register(&rk3399_key_drv);
}

static void __exit key_drv_exit(void)
{
	platform_driver_unregister(&rk3399_key_drv);
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");