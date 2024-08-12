./qemu-system-riscv64 -M aislab -m 256M -nographic \
    -bios ./fw_jump.bin \
    -kernel ./Image \
    -drive file=./rootfs.ext2,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    -append "root=/dev/vda rw console=ttyS0" \
    -virtfs local,path=./shared,mount_tag=host0,security_model=mapped,id=host0