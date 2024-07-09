echo "[copying Customized SoC into qemu]"
cp /workspace/Customized/qemu/riscv64_default.mak /workspace/qemu/configs/devices/riscv64-softmmu/default.mak 
cp /workspace/Customized/qemu/riscv32_default.mak /workspace/qemu/configs/devices/riscv32-softmmu/default.mak 
cp /workspace/Customized/qemu/aislab.c  /workspace/qemu/hw/riscv/aislab.c
cp /workspace/Customized/qemu/Kconfig  /workspace/qemu/hw/riscv/Kconfig
cp /workspace/Customized/qemu/meson.build  /workspace/qemu/hw/riscv/meson.build
cp /workspace/Customized/qemu/aislab.h  /workspace/qemu/include/hw/riscv/aislab.h
