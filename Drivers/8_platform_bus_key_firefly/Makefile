KERNELDIR := /home/liu/rockchip/kernel/kernel-develop-4.4_back
CURRENT_PATH := $(shell pwd)
obj-m := key_firefly.o chip_firefly.o board_rk3399_key.o

build:kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
