#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

#include "led_operation.h"

static volatile unsigned  int *PMUCRU_CLKGATE_CON1;
static volatile unsigned  int *PMUGRF_GPIO0B_IOMUX;
static volatile unsigned  int *GPIO_SWPORTA_DR;
static volatile unsigned  int *GPIO_SWPORTA_DDR;
// static volatile unsigned  int *GPIO_EXT_PORTA;

static int board_demo_init(int which) {
	printk("%s %s %d led %d\r\n", __FILE__, __FUNCTION__, __LINE__, which);

	if(!PMUCRU_CLKGATE_CON1)
		PMUCRU_CLKGATE_CON1 = ioremap(0xFF760000 + 0x0104, 4);
	if(!PMUGRF_GPIO0B_IOMUX)
		PMUGRF_GPIO0B_IOMUX = ioremap(0xFF320000 + 0x0004, 4);
	if(!GPIO_SWPORTA_DR)
		GPIO_SWPORTA_DR = ioremap(0XFF720000 + 0x0000, 4);	
	if(!GPIO_SWPORTA_DDR)
		GPIO_SWPORTA_DDR = ioremap(0XFF720000 + 0x0004, 4);
	
	/* operations register */
	*PMUCRU_CLKGATE_CON1 = (1 << (3 + 16)) & (0 << 3);
	*PMUGRF_GPIO0B_IOMUX = (3 << (10 + 16)) & (0 << 10);
	*GPIO_SWPORTA_DDR = (*GPIO_SWPORTA_DDR) | (1 << 13);

	return 0;
}

static int board_demo_ctl(int witch, char status) {
	printk("%s %s %d led %d %s\r\n", __FILE__, __FUNCTION__, __LINE__, witch, status ? "on" : "off");

	if(witch == 0) {
		if(status)
			*GPIO_SWPORTA_DR = (*GPIO_SWPORTA_DR) | (1 << 13);
		else
			*GPIO_SWPORTA_DR = (*GPIO_SWPORTA_DR) & (~(1 << 13));
	}

	return 0;
}

static struct led_operations board_demo_led_opr = {
	.num = 1,
	.init = board_demo_init,
	.ctl = board_demo_ctl,
};

struct led_operations *get_board_led_opr(void) {

	return &board_demo_led_opr;
}
