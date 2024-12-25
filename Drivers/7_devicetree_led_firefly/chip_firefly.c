#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include <linux/platform_device.h>
#include <linux/of.h>
#include "chip_firefly.h"
#include "led_firefly.h"

static volatile unsigned  int *PMUCRU_CLKGATE_CON[100];
static volatile unsigned  int *PMUGRF_GPIO_IOMUX[100];
static volatile unsigned  int *GPIO_SWPORTA_DR[100];
static volatile unsigned  int *GPIO_SWPORTA_DDR[100];

static int ledcnt = 0;
static int ledpins[64] = {0};

static int board_rk3399_led_init(int witch)
{
	int ledpin = ledpins[witch];
	unsigned int cru_addr = 0;
	unsigned int gpio_dr_addr = 0;
	unsigned int gpio_ddr_addr = 0;
	unsigned int grf_addr = 0;
	int pin = 0;
	
	printk("init gpio: group %d, pin %d\n", GROUP(ledpins[witch]), PIN(ledpins[witch]));
	pin = PIN(ledpin);
	switch(GROUP(ledpin)) {
		case 0: {
			cru_addr = 0XFF760000 + 0x0104;
			grf_addr = 0xFF320000 + (pin / 8) * 4;
			gpio_dr_addr = 0XFF720000;
			gpio_ddr_addr = 0XFF720000 + 4;
			break;
		}
		case 1: {
			cru_addr = 0XFF760000 + 0x0104;
			grf_addr = 0xFF320000 + 0x00010 + (pin / 8) * 4;
			gpio_dr_addr = 0XFF730000;
			gpio_ddr_addr = 0XFF730000 + 4;
			break;
		}	
		case 2: {
			cru_addr = 0XFF760000 + 0x037C;
			grf_addr = 0xFF770000 + (pin / 8) * 4;
			gpio_dr_addr = 0xFF780000;
			gpio_ddr_addr = 0xFF780000 + 4;
			break;
		}	
		case 3: {
			cru_addr = 0XFF760000 + 0x037C;
			grf_addr = 0xFF770000 + 0x0e010 + (pin / 8) * 4;
			gpio_dr_addr = 0xFF788000;
			gpio_ddr_addr = 0xFF788000 + 4;
			break;
		}	
		case 4: {
			cru_addr = 0XFF760000 + 0x037C;
			grf_addr = 0xFF770000 + 0x0e020 + (pin / 8) * 4;
			gpio_dr_addr = 0xFF790000;
			gpio_ddr_addr = 0xFF790000 + 4;
			break;
		}
	}

	PMUCRU_CLKGATE_CON[witch] = ioremap(cru_addr, 4);
	PMUGRF_GPIO_IOMUX[witch] = ioremap(grf_addr, 4);
	GPIO_SWPORTA_DR[witch] = ioremap(gpio_dr_addr, 4);
	GPIO_SWPORTA_DDR[witch]  = ioremap(gpio_ddr_addr, 4);

	printk("cru_addr = %#x\r\n", cru_addr);
	printk("grf_addr = %#x\r\n", grf_addr);
	printk("gpio_dr_addr = %#x\r\n", gpio_dr_addr);
	printk("gpio_ddr_addr = %#x\r\n", gpio_ddr_addr);
	
	if(GROUP(ledpin) == 0 || GROUP(ledpin) == 2)
		*PMUCRU_CLKGATE_CON[witch]  = (1 << (3 + 16)) | (0 << 3);
	else if(GROUP(ledpin) == 1 || GROUP(ledpin) == 3) 
		*PMUCRU_CLKGATE_CON[witch]  = (1 << (4 + 16)) | (0 << 4);
	else
		*PMUCRU_CLKGATE_CON[witch]  = (1 << (5 + 16)) | (0 << 5);
	printk("*PMUCRU_CLKGATE_CON is %x\r\n", *PMUCRU_CLKGATE_CON[witch] );
	
	*PMUGRF_GPIO_IOMUX[witch]  = (3 << ((pin % 8) * 2 + 16)) | (0 << (pin % 8) * 2);
	printk("*PMUGRF_GPIO_IOMUX is %x\r\n", *PMUGRF_GPIO_IOMUX[witch] );
	
	*GPIO_SWPORTA_DDR[witch] = (*GPIO_SWPORTA_DDR[witch]) | (1 << pin);
	printk("*GPIO_SWPORTA_DDR is %x\r\n", *GPIO_SWPORTA_DDR[witch]);

	printk("led init achievment\r\n");
	
	return 0;
}

static int board_rk3399_led_ctl(int witch, char status) 
{
	int pin = PIN(ledpins[witch]);

	if(status)
		*GPIO_SWPORTA_DR[witch] = (*GPIO_SWPORTA_DR[witch]) | (1 << pin);
	else
		*GPIO_SWPORTA_DR[witch] = (*GPIO_SWPORTA_DR[witch]) & (~(1 << pin));


	printk("led ctl achievment\r\n");

	return 0;
}

static void board_rk3399_led_exit(int witch) 
{
	iounmap(PMUCRU_CLKGATE_CON[witch]);
	iounmap(PMUGRF_GPIO_IOMUX[witch]);
	iounmap(GPIO_SWPORTA_DR[witch]);
	iounmap(GPIO_SWPORTA_DDR[witch]);

	printk("led IO unmap\r\n");
}

static struct led_operations board_rk3399_led_opr = {
	.init = board_rk3399_led_init,
	.ctl = board_rk3399_led_ctl,
	.exit = board_rk3399_led_exit,
};

static int chip_rk3399_gpio_probe(struct platform_device *pdev)
{
	struct  device_node *np;
	int err = 0;
	int led_pin;
	const char *name_led;

	np = pdev->dev.of_node;

	err = of_property_read_u32 (np, "PIN", &led_pin);
	ledpins[ledcnt] = led_pin;
	printk("init gpio: group %d, pin %d\n", GROUP(ledpins[ledcnt]), PIN(ledpins[ledcnt]));
  	err = of_property_read_string(np, "name_led", &name_led);
	
	led_class_create_device(ledcnt, name_led);
	printk("subdevice %s cteate succesful\r\n", name_led);
	ledcnt++;

	return 0;
}

static int chip_rk3399_gpio_remove(struct platform_device *pdev)
{
	struct  device_node *np;
	int err = 0;
	int led_pin;
	int i = 0;

	np = pdev->dev.of_node;

	err = of_property_read_u32 (np, "PIN", &led_pin);

	for(i=0; i<ledcnt; i++) {
		if(led_pin == ledpins[ledcnt])
			led_class_destroy_device(i);
		 ledpins[ledcnt] = -1;
	}

	for (i = 0; i < ledcnt; i++)
       {
	    if (ledpins[i] != -1)
	        break;
	 }

	 if (i == ledcnt)
	    ledcnt = 0;

	return 0;
}

static const struct of_device_id leds_of_match[] = {
	{ .compatible = "rk3399, ledsdrv" },
	{},
};

struct platform_driver board_rk3399_led_dri = {
	.probe = chip_rk3399_gpio_probe,
	.remove = chip_rk3399_gpio_remove,
	.driver = {
		.name = "rk3399_led",
		.of_match_table = leds_of_match,
	},
};

static int led_dri_init(void) {
	int err;

	err = platform_driver_register(&board_rk3399_led_dri);
	register_led_operations(&board_rk3399_led_opr);
	
	return 0;
}

static void led_dri_exit(void) {
	platform_driver_unregister(&board_rk3399_led_dri);
}

module_init(led_dri_init);
module_exit(led_dri_exit);

MODULE_LICENSE("GPL");
