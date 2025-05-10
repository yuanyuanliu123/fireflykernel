#!/bin/bash

make dtbs ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
cp arch/arm64/boot/dts/rockchip/rk3399-firefly-linux.dtb boot/rk3399.dtb
cp arch/arm64/boot/Image boot/
genext2fs -b 32768 -B $((64*1024*1024/32768)) -d boot/ -i 8192 -U boot.img
