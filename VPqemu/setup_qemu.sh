#!/bin/bash

echo "[setup VP qemu]"
cp -f /workspace/qemu/build/qemu-system-riscv64 ./ || exit 1
cp -f /workspace/opensbi/build/platform/generic/firmware/fw_jump.bin ./ || exit 1
cp -f /workspace/linux/arch/riscv/boot/Image ./ || exit 1
cp -f /workspace/buildroot/output/images/rootfs.ext2 ./ || exit 1
mkdir -p ./shared || exit 1

echo "[setup linux root FS]"
mkdir -p /tmp/root || exit 1
sudo mount rootfs.ext2 /tmp/root || exit 1

cd /tmp/root && sudo cp /workspace/config/rcS etc/init.d/rcS || exit 1 # active script
cd /tmp/root && sudo cp /workspace/config/bashrc ./root/.bashrc || exit 1 # bashrc
cd /tmp/root && sudo cp /workspace/config/profile ./root/.profile || exit 1 # bashrc
sudo chmod +x /tmp/root/etc/init.d/rcS || exit 1
sudo chmod +x /tmp/root/root/.bashrc || exit 1
sudo chmod +x /tmp/root/root/.profile || exit 1

cd /workspace/VPqemu  || exit 1
sudo umount /tmp/root || exit 1