version DIY_LIU V1.0
# 编译打包主目录
cd /home/liu/rockchip/kernel/code/fireflykernel/kernel-develop-4.4_back
# 编译
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j4  Complie driver and device-tree
make dtbs ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-  Complie device-tree
# 打包
cp arch/arm64/boot/dts/rockchip/rk3399-firefly-linux.dtb boot/rk3399.dtb
cp arch/arm64/boot/Image boot/
genext2fs -b 32768 -B 2048 -d boot/ -i 8192 -U boot.img
# git 目录
cd /home/liu/rockchip/kernel/code/fireflykernel/
