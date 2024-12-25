KERNELDIR := /home/liu/rockchip/kernel/kernel-develop-4.4_back
CURRENT_PATH := $(shell pwd)
leds_firefly-y := led_firefly.o board_firefly.o
obj-m := leds_firefly.o

build:kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
