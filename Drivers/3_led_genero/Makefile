KERNELDIR := /home/liu/rockchip/kernel/kernel-develop-4.4_back
CURRENT_PATH := $(shell pwd)
leds-y := led.o board_demo.o
obj-m := leds.o

build:kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
