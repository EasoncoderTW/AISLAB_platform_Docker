echo "[copying Customized SoC into qemu]" || exit 1
cp ./qemu/riscv64_default.mak /workspace/qemu/configs/devices/riscv64-softmmu/default.mak || exit 1
cp ./qemu/riscv32_default.mak /workspace/qemu/configs/devices/riscv32-softmmu/default.mak || exit 1
cp ./qemu/aislab.c  /workspace/qemu/hw/riscv/aislab.c || exit 1
cp ./qemu/Kconfig  /workspace/qemu/hw/riscv/Kconfig || exit 1
cp ./qemu/meson.build  /workspace/qemu/hw/riscv/meson.build || exit 1
cp ./qemu/aislab.h  /workspace/qemu/include/hw/riscv/aislab.h || exit 1

## adder ##
cp ./adder/* /workspace/qemu/hw/misc || exit 1