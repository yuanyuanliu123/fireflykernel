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

static volatile unsigned  int *PMUCRU_CLKGATE_CON31;
static volatile unsigned  int *PMUGRF_GPIO2D_IOMUX;
static volatile unsigned  int *GPIO2_SWPORTA_DR;
static volatile unsigned  int *GPIO2_SWPORTA_DDR;

// static volatile unsigned  int *GPIO_EXT_PORTA;

static int board_demo_init(void) {
	printk("%s %s %d\r\n", __FILE__, __FUNCTION__, __LINE__);

	/* GPIO0_B5 */
	if(!PMUCRU_CLKGATE_CON1)
		PMUCRU_CLKGATE_CON1 = ioremap(0xFF760000 + 0x0104, 4);
	else
		printk(KERN_ERR "Failed to map PMUCRU_CLKGATE_CON1\r\n");
	if(!PMUGRF_GPIO0B_IOMUX)
		PMUGRF_GPIO0B_IOMUX = ioremap(0xFF320000 + 0x0004, 4);
	else
		printk(KERN_ERR "Failed to map PMUGRF_GPIO0B_IOMUX\r\n");
	if(!GPIO_SWPORTA_DR)
		GPIO_SWPORTA_DR = ioremap(0XFF720000 + 0x0000, 4);	
	else
		printk(KERN_ERR "Failed to map GPIO_SWPORTA_DR\r\n");
	if(!GPIO_SWPORTA_DDR)
		GPIO_SWPORTA_DDR = ioremap(0XFF720000 + 0x0004, 4);
	else
		printk(KERN_ERR "Failed to map GPIO_SWPORTA_DDR\r\n");
	
	/* GPIO0_B5 operations register */
	*PMUCRU_CLKGATE_CON1 = (1 << (3 + 16)) | (0 << 3);
	*PMUGRF_GPIO0B_IOMUX = (3 << (10 + 16)) | (0 << 10);
	*GPIO_SWPORTA_DDR = (*GPIO_SWPORTA_DDR) | (1 << 13);

	
	/* GPIO2_D3 */
	if(!PMUCRU_CLKGATE_CON31)
		PMUCRU_CLKGATE_CON31 = ioremap(0xFF760000 + 0x037C, 4);
	else
		printk(KERN_ERR "Failed to map PMUCRU_CLKGATE_CON31\r\n");
	if(!PMUGRF_GPIO2D_IOMUX)
		PMUGRF_GPIO2D_IOMUX = ioremap(0xFF770000 + 0x0e00c, 4);
	else
		printk(KERN_ERR "Failed to map PMUGRF_GPIO2D_IOMUX\r\n");
	if(!GPIO2_SWPORTA_DR)
		GPIO2_SWPORTA_DR = ioremap(0XFF780000 + 0x0000, 4);
	else
		printk(KERN_ERR "Failed to map GPIO2_SWPORTA_DR\r\n");
	if(!GPIO2_SWPORTA_DDR)
		GPIO2_SWPORTA_DDR = ioremap(0XFF780000 + 0x0004, 4);
	else
		printk(KERN_ERR "Failed to map GPIO2_SWPORTA_DDR\r\n");

	/* GPIO2_D3 operations register */
	*PMUCRU_CLKGATE_CON31 = (1 << (3 + 16)) | (0 << 3);
	*PMUGRF_GPIO2D_IOMUX = (3 << (6 + 16)) | (0 << 6);
	*GPIO2_SWPORTA_DDR = (*GPIO2_SWPORTA_DDR) | (1 << 27);

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
	else if(witch == 1) {
		if(status)
			*GPIO2_SWPORTA_DR = (*GPIO2_SWPORTA_DR) | (1 << 27);
		else
			*GPIO2_SWPORTA_DR = (*GPIO2_SWPORTA_DR) & (~(1 << 27));
	}
	return 0;
}

static int board_demo_exit(void) {
	iounmap(PMUCRU_CLKGATE_CON1);
	iounmap(PMUGRF_GPIO0B_IOMUX);
	iounmap(GPIO_SWPORTA_DR);
	iounmap(GPIO_SWPORTA_DDR);

	iounmap(PMUCRU_CLKGATE_CON31);
	iounmap(PMUGRF_GPIO2D_IOMUX);
	iounmap(GPIO2_SWPORTA_DR);
	iounmap(GPIO2_SWPORTA_DDR);
	
	printk("led io unmap end\\r\n");

	return 0;
}

static struct led_operations board_demo_led_opr = {
	.num = 2,
	.init = board_demo_init,
	.ctl = board_demo_ctl,
	.exit = board_demo_exit,
};

struct led_operations *get_board_led_opr(void) {

	return &board_demo_led_opr;
}
