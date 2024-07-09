#!/bin/bash

mountdir=/workspace  # substituted during docker build
mountdir=${mountdir:-'/workspace'}

state=$1
arg1=$2
arg2=$3

name=$(id -un)
if [[ -e .first ]]; then
    sudo chown -R $name:$name *
    touch .first
fi

cd ${mountdir}

usage_message(){
cat <<EOF

    Welcome to use the AISLAB-platform working environment

    This docker environment is built for the AISLAB-platform development
    All projects are in the /workspace directory
    Here is a list of startup commands that you can run in the docker

    startup [help]                      : show the welcome and usage message
    ----------------    Get    --------------------------------------------------------------
    startup get-qemu [version]          : download qemu source code (default version: 9.0.1)
    startup get-riscv-gcc               : download riscv-gnu-toolchain Cross Compiler
    startup get-opensbi                 : download opensbi source code
    startup get-linux                   : download linux kernel source code
    startup get-busybox                 : download busybox source code
    startup get-freertos                : download FreeRTOS source code
    ----------------   Build   --------------------------------------------------------------
    startup build-qemu                  : build qemu
    startup build-systemc [version]     : build SystemC library (default version: 2.3.1)
    startup build-riscv-gcc             : build riscv-gnu-toolchain Cross Compiler
    startup build-opensbi               : build opensbi source code
    startup build-linux                 : build linux kernel source code
    startup build-busybox               : build busybox source code
    ----------------   Task   --------------------------------------------------------------
    startup init                        : download and build all the above tools with default versions
    startup rebuild-qemu                : rebuild qemu from source code
EOF
}

rebuild_qemu(){
    echo ---------------- rebuild qemu ----------------
    rm -rf $mountdir/qemu/build || exit 1
    mkdir -p $mountdir/qemu/build || exit 1
    cd $mountdir/qemu/build || exit 1
    ../configure --enable-debug-info --target-list=riscv64-softmmu,riscv32-softmmu --enable-virtfs || exit 1
    make -j $(nproc) || exit 1
}

build_qemu(){
    [[ -f $mountdir/qemu/build/qemu-system-riscv64 ]]  && {
        echo $(which qemu-system-riscv64) exists
    } || {
        mkdir -p $mountdir/qemu/build || exit 1
        cd $mountdir/qemu/build || exit 1
        ../configure --enable-debug-info --target-list=riscv64-softmmu, --enable-virtfs || exit 1
        make -j $(nproc) || exit 1
    } 
}

build_riscv_gcc(){
    [[ -f $mountdir/rv64-linux/bin/riscv64-unknown-linux-gnu-gcc ]]  && { 
        echo $(which riscv64-unknown-linux-gnu-gcc) exists
    } || {
        cd $mountdir/riscv-gnu-toolchain || exit 1
        ./configure --prefix=$mountdir/rv64-linux --with-arch=rv64gc --with-abi=lp64d --disable-gdb || exit 1
        make -j $(nproc) linux || exit 1
    } 
}

build_opensbi(){
    [[ -f $mountdir/opensbi/build/platform/generic/firmware/fw_jump.bin ]]  && { 
        echo opensbi firmware exists
    } || {
        cd $mountdir/opensbi || exit 1
        make PLATFORM=generic CROSS_COMPILE=riscv64-unknown-linux-gnu- O=build -j$(nproc) || exit 1
        make -j $(nproc) linux || exit 1
    } 
}

build_linux(){
    [[ -f $mountdir/linux/arch/riscv/boot/Image ]]  && { 
        echo linux kernel Image exists
    } || {
        cd $mountdir/linux || exit 1
        cp $mountdir/config/linux.config .config || exit 1
        make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -j $(nproc) || exit 1
        make -j $(nproc) linux || exit 1
    } 
}

build_busybox(){
    [[ -f $mountdir/busybox/root.ext2 ]]  && { 
        echo busybox/root.ext2 exists
    } || {
        cd $mountdir/busybox || exit 1
        cp $mountdir/config/busybox.config $mountdir/busybox/.config || exit 1
        ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- make -j $(nproc) || exit 1
        ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- make install || exit 1
        echo ===================== Make Root File System ===================== || exit 1
        dd if=/dev/zero of=root.ext2 bs=1M count=1024 || exit 1 # 1G
        sudo mkfs.ext2 -F root.ext2 || exit 1
        mkdir -p /tmp/root || exit 1
        sudo mount root.ext2 /tmp/root || exit 1
        sudo rsync -avr _install/* /tmp/root || exit 1
        cd /tmp/root && sudo mkdir -p proc sys dev etc etc/init.d || exit 1
        cd /tmp/root && sudo cp $mountdir/config/rcS etc/init.d/rcS || exit 1 # active script
        cd /tmp/root && sudo cp $mountdir/config/bashrc ./.bashrc || exit 1 # bashrc
        sudo chmod +x /tmp/root/etc/init.d/rcS || exit 1
        cd $mountdir/busybox  || exit 1
        sudo umount /tmp/root || exit 1
    }
}


######################################################################

get_qemu(){
    top=$mountdir
    dir=$top/qemu
    ver=${1:-9.0.1}
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        wget https://download.qemu.org/qemu-$ver.tar.xz -O - | tar xJC $top
        mv $dir-$ver $dir
    }
}

get_riscv_gcc(){
    top=$mountdir
    dir=$top/riscv-gnu-toolchain
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git $dir
    }
}

get_opensbi(){
    top=$mountdir
    dir=$top/opensbi
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        git clone https://github.com/riscv/opensbi.git $dir
    }
}

get_linux(){
    top=$mountdir
    dir=$top/linux
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        git clone https://github.com/torvalds/linux.git $dir
    }
}

get_busybox(){
    top=$mountdir
    dir=$top/busybox
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        git clone git://git.busybox.net/busybox $dir || exit 1
        cd $mountdir/busybox || exit 1
        git checkout -b 1_36_stable origin/1_36_stable || exit 1
    }
}

get_freertos(){
    top=$mountdir
    dir=$top/FreeRTOS
    ver=202212.01
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        git clone --recurse-submodules https://github.com/FreeRTOS/FreeRTOS.git $dir
    }
}

case $state in
    get-qemu)
        version=$arg1
        get_qemu $version
        ;;
    get-freertos)
        get_freertos
        ;;
    get-riscv-gcc)
        get_riscv_gcc
        ;;
    get-opensbi)
        get_opensbi
        ;;
    get-linux)
        get_linux
        ;;
    get-busybox)
        get_busybox
        ;;
    build-qemu)
        build_qemu
        ;;
    build-riscv-gcc)
        build_riscv_gcc
        ;;
    build-opensbi)
        build_opensbi
        ;;
    build-linux)
        build_linux
        ;;
    build-busybox)
        build_busybox
        ;;
    build-systemc)
        version=$arg1
        bash $mountdir/Docker/modules/systemc/systemc.sh $version
        ;;
    rebuild-qemu)
        rebuild_qemu
        ;;
    check-machine | check | cm)
        qemu-system-riscv64 -machine help
        ;;
    init)
        get_qemu
        build_qemu
        get_riscv_gcc
        build_riscv_gcc
        get_opensbi
        build_opensbi
        get_linux
        build_linux
        get_busybox
        build_busybox
        qemu-system-riscv64 -machine help
        ;;
    help)
        usage_message
        ;;
    *)
        echo unknown command: $state
        usage_message
        ;;
esac
