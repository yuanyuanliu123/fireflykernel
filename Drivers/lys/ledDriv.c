#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>

/*************************************
 *FileName:ledDriv.c
 *Fuc:     RK3399,GPIO0B_5 driver
 *
 *Author:  PineLiu
 ************************************/

#define CHAR_CNT     1          //设备数量
#define LED_NAME     "gumpled"	//设备名称

#define LEDON        1		//点亮
#define LEDOFF	     0  	//熄灭

//寄存器物理地址
#define PMUCRU_CLKGATE_CON1	0XFF750104
#define PMUGRF_GPIO0B_IOMUX	0XFF320004
#define GPIO_SWPORTA_DDR	0XFF720004
#define GPIO_SWPORTA_DR		0XFF720000


//重映射后的寄存器虚拟地址
static void __iomem *GPIO0_DR;	  //数据寄存器
static void __iomem *GPIO0_DDR;	  //方向寄存器
static void __iomem *GPIO0_IOMUX; //复用寄存器
static void __iomem *GPIO0_CRU;	  //时钟寄存器


//声明设备结构体(该设备包含的属性)
struct newexternal_dev{
	dev_t           devid;	 //设备号
	struct cdev     cdev;	 //字符设备
	struct class   *class;	 //设备节点类 主动创建设备节点文件用
	struct device  *device;  //设备节点 主动创建设备节点文件用
	int             major;	 //主设备号
	int             minor;	 //次设备号
};


//声明一个外部设备，本文这里是字符设备
struct newexternal_dev charled;


//LED打开/关闭，传入1点亮，传入0关闭
void led_switch(u8 sta)
{
	u32 val = 0;
	if(sta == LEDON){
		val = readl(GPIO0_DR);
		val |= (1<<13);
		writel(val, GPIO0_DR);
	}else if(sta == LEDOFF){
		val = readl(GPIO0_DR);
		val &= ~(1<<13);
		writel(val,GPIO0_DR);
	}
}


//===========================================================================
//以下实现设备的具体操作函数：open函数、read函数、write函数和release函数
//===========================================================================

//打开设备
static int led_open(struct inode *inode, struct file *filp)
{
	printk("Enter led_open function\r\n");
	filp -> private_data = &charled;	//设置私有数据
	return 0;
}

//从设备读取数据
static ssize_t led_read(struct file *filp,char __user *buf,
			size_t cnt,loff_t *offt)
{
	return 0;
}

//向设备写数据
//filp:设备文件，表示打开的文件描述
//buf :保存着要向设备写入的数据
//cnt :要写入的数据长度
//offt:相对于文件首地址的偏移
//
static ssize_t led_write(struct file *filp,const char __user *buf,
			size_t cnt,loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1];
	unsigned char ledstat;


	retvalue = copy_from_user(databuf,buf,cnt);
	if(retvalue < 0){
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}

	printk("Enter led_write %d function\r\n", databuf[0]);

	ledstat = databuf[0];	//获取状态值
	
	if(ledstat == LEDON){
		led_switch(LEDON);
	}else if(ledstat == LEDOFF){
		led_switch(LEDOFF);
	}

	return 0;
	
}

//关闭，释放设备
//filp: 要关闭的设备文件描述
//
static int led_release(struct inode *inode,struct file *filp)
{
	printk("close\r\n");

	return 0;                                                                                                                                                
}


//==========================================================================
//以下实现的设备具体函数与内核的对应函数的映射
//==========================================================================
//映射设备操作函数
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.read  = led_read,
	.write = led_write,
	.release = led_release,
};


//==========================================================================
//内核模块相关
//==========================================================================

//内核模块加载函数或称为驱动入口函数
static int __init led_init(void)
{
	u32 val = 0;
	//=========初始化LED
	//寄存器地址映射
	GPIO0_DR    = ioremap(GPIO_SWPORTA_DR,  4);
	GPIO0_DDR   = ioremap(GPIO_SWPORTA_DDR, 4);
	GPIO0_IOMUX = ioremap(PMUGRF_GPIO0B_IOMUX, 4);
	GPIO0_CRU   = ioremap(PMUCRU_CLKGATE_CON1, 4);

	//使能时钟
//	GPIO0_CRU |= (1 << (3 + 16));
//      GPIO0_CRU &= ~(1 << 3);
	val = readl(GPIO0_CRU);
	val |= (1 << (3+16));
	val &= ~(1 << 3);
	writel(val,GPIO0_CRU);

	//设置GPIO0B_5引脚复用功能为GPIO
//	GPIO0_IOMUX |= (3 << (10+16));
//	GPIO0_IOMUX &= ~(3 << 10);
	val = readl(GPIO0_IOMUX);
	val |= (3 << (10+16));
	val &= ~(3 << 10);
	writel(val,GPIO0_IOMUX);

	//配置GPIO0B_5引脚为输出模式
//	GPIO0_DDR |= (1 << 13);
	val = readl(GPIO0_DDR);
	val |= (1 << 13);
	writel(val,GPIO0_DDR);
	
	//配置GPIO0B_5引脚默认关闭LED
//	GPIO0_DR &= ~(1 << 13);
	val = readl(GPIO0_DR);
	val &= ~(1 << 13);
	writel(val,GPIO0_DR);


	//===========注册设备	
	//创建设备号
	if(charled.major){ //定义了设备号
		charled.devid = MKDEV(charled.major,0);
		register_chrdev_region(charled.devid,CHAR_CNT,LED_NAME);
	}else{ //没有定义设备号，要向系统申请设备号
		alloc_chrdev_region(&(charled.devid),0,CHAR_CNT,LED_NAME);
		charled.major = MAJOR(charled.devid);
		charled.minor = MINOR(charled.devid);
	}
	printk("charled major = %d,minor = %d\r\n",charled.major,charled.minor);

	//初始化cdev
	charled.cdev.owner = THIS_MODULE;
	cdev_init(&charled.cdev,&led_fops);
	//向系统注册设备
	cdev_add(&charled.cdev,charled.devid,CHAR_CNT);
	//创建类
	charled.class = class_create(THIS_MODULE,LED_NAME);
	if(IS_ERR(charled.class)){
		return PTR_ERR(charled.class);
	}
	//创建设备节点
	charled.device = device_create(charled.class,NULL,charled.devid,NULL,LED_NAME);
	if(IS_ERR(charled.device)){
		return PTR_ERR(charled.device);
	}

	return 0;

}


//内核模块卸载函数
static void __exit led_exit(void)
{
	//取消映射
	iounmap(GPIO0_CRU);
	iounmap(GPIO0_IOMUX);
	iounmap(GPIO0_DDR);
	iounmap(GPIO0_DR);

	//注销设备
	cdev_del(&charled.cdev);
	unregister_chrdev_region(charled.devid,CHAR_CNT);

	device_destroy(charled.class,charled.devid);
	class_destroy(charled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gump");
