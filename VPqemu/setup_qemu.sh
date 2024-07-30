#!/bin/bash

echo "[setup VP qemu]"
cp -f /workspace/qemu/build/qemu-system-riscv64 ./ || exit 1
cp -f /workspace/opensbi/build/platform/generic/firmware/fw_jump.bin ./ || exit 1
cp -f /workspace/linux/arch/riscv/boot/Image ./ || exit 1
cp -f /workspace/busybox/root.ext2 ./ || exit 1
mkdir -p ./shared || exit 1

echo "[setup linux root FS]"
sudo mount root.ext2 /tmp/root || exit 1
cd /tmp/root && sudo mkdir -p proc sys dev etc etc/init.d || exit 1
cd /tmp/root && sudo cp /workspace/config/rcS etc/init.d/rcS || exit 1 # active script
cd /tmp/root && sudo cp /workspace/config/bashrc ./.bashrc || exit 1 # bashrc
sudo chmod +x /tmp/root/etc/init.d/rcS || exit 1
cd /tmp/root && sudo cp -r /workspace/rv64-linux/sysroot/lib ./ || exit 1
cd /tmp/root && sudo cp -r /workspace/rv64-linux/sysroot/etc ./ || exit 1
cd /tmp/root && sudo cp -r /workspace/rv64-linux/sysroot/sbin ./ || exit 1
cd /tmp/root && sudo cp -r /workspace/rv64-linux/sysroot/usr ./ || exit 1
cd /tmp/root && sudo cp -r /workspace/rv64-linux/sysroot/var ./ || exit 1
cd /workspace/VPqemu  || exit 1
sudo umount /tmp/root || exit 1