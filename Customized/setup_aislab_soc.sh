echo "[copying Customized SoC into qemu]" || exit 1
## riscv aislab virt machine ##
cp ./QEMU/riscv/riscv64_default.mak /workspace/qemu/configs/devices/riscv64-softmmu/default.mak || exit 1
cp ./QEMU/riscv/riscv32_default.mak /workspace/qemu/configs/devices/riscv32-softmmu/default.mak || exit 1
cp ./QEMU/riscv/aislab.c  /workspace/qemu/hw/riscv/aislab.c || exit 1
cp ./QEMU/riscv/Kconfig  /workspace/qemu/hw/riscv/Kconfig || exit 1
cp ./QEMU/riscv/meson.build  /workspace/qemu/hw/riscv/meson.build || exit 1
cp ./QEMU/riscv/aislab.h  /workspace/qemu/include/hw/riscv/aislab.h || exit 1

## mmio bus ##
cp ./QEMU/mmio_bus/mmio_bus.c /workspace/qemu/hw/misc || exit 1
cp ./QEMU/mmio_bus/meson.build /workspace/qemu/hw/misc || exit 1
cp ./QEMU/mmio_bus/Kconfig /workspace/qemu/hw/misc || exit 1
cp ./QEMU/mmio_bus/mmio_bus.h /workspace/qemu/include/hw/misc || exit 1

## virtual platform IPC ##
cp ./QEMU/vpipc/vpipc.c /workspace/qemu/hw/misc || exit 1
cp ./QEMU/vpipc/vpipc.h /workspace/qemu/include/hw/misc || exit 1

## adder ##
# cp ./adder/adder.c /workspace/qemu/hw/misc || exit 1
# cp ./adder/meson.build /workspace/qemu/hw/misc || exit 1
# cp ./adder/Kconfig /workspace/qemu/hw/misc || exit 1
# cp ./adder/adder.h /workspace/qemu/include/hw/misc || exit 1