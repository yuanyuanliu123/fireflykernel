#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include <linux/platform_device.h>

#include "chip_firefly.h"
#include "key_firefly.h"
#include "key_resource.h"

static volatile unsigned  int *PMUCRU_CLKGATE_CON[100];
static volatile unsigned  int *PMUGRF_GPIO_IOMUX[100];
static volatile unsigned  int *GPIO_SWPORTA_DR[100];
static volatile unsigned  int *GPIO_SWPORTA_DDR[100];
static volatile unsigned  int *GPIO_EXT_PORTA[100];

static int keycnt = 0;
static int keypins[64] = {0};

static int board_rk3399_key_init(int witch)
{
	int keypin = keypins[witch];
	unsigned int cru_addr = 0;
	unsigned int gpio_dr_addr = 0;
	unsigned int gpio_ddr_addr = 0;
	unsigned int gpio_porta_addr = 0;
	unsigned int grf_addr = 0;
	int pin = 0;
	
	printk("init gpio: group %d, pin %d\n", GROUP(keypins[witch]), PIN(keypins[witch]));
	pin = PIN(keypin);
	switch(GROUP(keypin)) {
		case 0: {
			cru_addr = 0XFF760000 + 0x0104;
			grf_addr = 0xFF320000 + (pin / 8) * 4;
			gpio_dr_addr = 0XFF720000;
			gpio_ddr_addr = 0XFF720000 + 4;
			gpio_porta_addr = 0XFF720000 + 0x50;
			break;
		}
		case 1: {
			cru_addr = 0XFF760000 + 0x0104;
			grf_addr = 0xFF320000 + 0x00010 + (pin / 8) * 4;
			gpio_dr_addr = 0XFF730000;
			gpio_ddr_addr = 0XFF730000 + 4;
			gpio_porta_addr = 0XFF730000 + 0x50;
			break;
		}	
		case 2: {
			cru_addr = 0XFF760000 + 0x037C;
			grf_addr = 0xFF770000 + (pin / 8) * 4;
			gpio_dr_addr = 0xFF780000;
			gpio_ddr_addr = 0xFF780000 + 4;
			gpio_porta_addr = 0xFF780000 + 0x50;
			break;
		}	
		case 3: {
			cru_addr = 0XFF760000 + 0x037C;
			grf_addr = 0xFF770000 + 0x0e010 + (pin / 8) * 4;
			gpio_dr_addr = 0xFF788000;
			gpio_ddr_addr = 0xFF788000 + 4;
			gpio_porta_addr = 0xFF788000 + 0x50;
			break;
		}	
		case 4: {
			cru_addr = 0XFF760000 + 0x037C;
			grf_addr = 0xFF770000 + 0x0e020 + (pin / 8) * 4;
			gpio_dr_addr = 0xFF790000;
			gpio_ddr_addr = 0xFF790000 + 4;
			gpio_porta_addr = 0xFF790000 + 0x50;
			break;
		}
	}

	PMUCRU_CLKGATE_CON[witch] = ioremap(cru_addr, 4);
	PMUGRF_GPIO_IOMUX[witch] = ioremap(grf_addr, 4);
	GPIO_SWPORTA_DR[witch] = ioremap(gpio_dr_addr, 4);
	GPIO_SWPORTA_DDR[witch]  = ioremap(gpio_ddr_addr, 4);
	GPIO_EXT_PORTA[witch]  = ioremap(gpio_porta_addr, 4);

	printk("cru_addr = %#x\r\n", cru_addr);
	printk("grf_addr = %#x\r\n", grf_addr);
	printk("gpio_dr_addr = %#x\r\n", gpio_dr_addr);
	printk("gpio_ddr_addr = %#x\r\n", gpio_ddr_addr);
	printk("gpio_porta_addr = %#x\r\n", gpio_porta_addr);
	
	if(GROUP(keypin) == 0 || GROUP(keypin) == 2)
		*PMUCRU_CLKGATE_CON[witch]  = (1 << (3 + 16)) | (0 << 3);
	else if(GROUP(keypin) == 1 || GROUP(keypin) == 3) 
		*PMUCRU_CLKGATE_CON[witch]  = (1 << (4 + 16)) | (0 << 4);
	else
		*PMUCRU_CLKGATE_CON[witch]  = (1 << (5 + 16)) | (0 << 5);
	printk("*PMUCRU_CLKGATE_CON is %x\r\n", *PMUCRU_CLKGATE_CON[witch] );
	
	*PMUGRF_GPIO_IOMUX[witch]  = (3 << ((pin % 8) * 2 + 16)) | (0 << (pin % 8) * 2);
	printk("*PMUGRF_GPIO_IOMUX is %x\r\n", *PMUGRF_GPIO_IOMUX[witch] );
	
	*GPIO_SWPORTA_DDR[witch] = (*GPIO_SWPORTA_DDR[witch]) & (~(1 << pin));
	printk("*GPIO_SWPORTA_DDR is %x\r\n", *GPIO_SWPORTA_DDR[witch]);

	printk("*GPIO_EXT_PORTA is %x\r\n", *GPIO_EXT_PORTA[witch]);
	
	printk("key init achievment\r\n");
	
	return 0;
}

static int board_rk3399_key_ctl(int witch) 
{
	int pin = PIN(keypins[witch]);
	int level_key = 0;

	level_key = (*GPIO_EXT_PORTA[witch]) & (1 << pin) ? 1 : 0;

	printk("key ctl achievment pin %d level %d GPIO_EXT_PORTA %d\r\n", pin, level_key, *GPIO_EXT_PORTA[witch]);

	return level_key;
}

static void board_rk3399_key_exit(int witch) 
{
	iounmap(PMUCRU_CLKGATE_CON[witch]);
	iounmap(PMUGRF_GPIO_IOMUX[witch]);
	iounmap(GPIO_SWPORTA_DR[witch]);
	iounmap(GPIO_SWPORTA_DDR[witch]);

	printk("key IO unmap\r\n");
}

static struct key_operations board_rk3399_key_opr = {
	.init = board_rk3399_key_init,
	.ctl = board_rk3399_key_ctl,
	.exit = board_rk3399_key_exit,
};

static int chip_rk3399_gpio_probe(struct platform_device *pdev)
{
	struct resource *ret;
	int i = 0;

	while(1) {
		ret = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		if(!ret) {
			break;
		}

		keypins[keycnt] = ret->start;
		key_class_create_device(keycnt, ret->name);

		printk("subdevice %s cteate succesful\r\n", ret->name);
		
		keycnt++;
		i++;
	}

	return 0;
}

static int chip_rk3399_gpio_remove(struct platform_device *pdev)
{
	struct resource *ret;
	int i = 0;

	while(1) {
		ret = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		if(!ret) {
			break;
		}
		
		key_class_destroy_device(i);

		keycnt--;
		i++;
	}

	return 0;
}

struct platform_driver board_rk3399_key_dri = {
	.probe = chip_rk3399_gpio_probe,
	.remove = chip_rk3399_gpio_remove,
	.driver = {
		.name = "rk3399_key",
	},
};

static int key_dri_init(void) {
	int err;

	err = platform_driver_register(&board_rk3399_key_dri);
	register_key_operations(&board_rk3399_key_opr);
	
	return 0;
}

static void key_dri_exit(void) {
	platform_driver_unregister(&board_rk3399_key_dri);
}

module_init(key_dri_init);
module_exit(key_dri_exit);

MODULE_LICENSE("GPL");
