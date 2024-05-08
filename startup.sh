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
    startup build-qemu                  : build qemu
    startup build-systemc [version]     : build SystemC library (default version: 2.3.1)
    startup get-qemu [version]          : download qemu source code (default version: 8.0.0)
    startup get-riscv-gcc               : download riscv-gnu-toolchain Cross Compiler
    startup get-freertos                : download FreeRTOS source code
    startup init                        : download and build all the above tools with default versions

EOF
}

build_qemu(){
    [[ $(qemu-system-riscv64 --version > /dev/null) -eq 0 ]] && exit 0
    mkdir -p $mountdir/qemu/build || exit 1
    cd $mountdir/qemu/build || exit 1
    ../configure --enable-debug-info --target-list=riscv64-softmmu --enable-virtfs || exit 1
    make -j $(nproc) || exit 1
}

get_qemu(){
    top=$mountdir
    dir=$top/qemu
    ver=${1:-8.0.0}
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        wget https://download.qemu.org/qemu-$ver.tar.xz -O - | tar xJC $top
        mv $dir-$ver $dir
    }
}

get_riscv_gcc(){
    top=$mountdir
    dir=$top/riscv
    file=riscv64-elf-ubuntu-20.04-gcc-nightly-2024.04.12-nightly.tar.gz
    [[ -d $dir ]] && {
        echo $dir exists
    } || {
        wget https://github.com/riscv-collab/riscv-gnu-toolchain/releases/download/2024.04.12/$file -O - | tar zxC $top
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
        # wget https://github.com/FreeRTOS/FreeRTOS/archive/refs/tags/$ver.tar.gz -O - | tar zxC $top
        # mv $dir-$ver $dir
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
    build-qemu)
        build_qemu
        ;;
    build-systemc)
        version=$arg1
        bash $mountdir/Docker/modules/systemc/systemc.sh $version
        ;;
    check-machine | check | cm)
        qemu-system-riscv64 -machine help
        ;;
    init)
        get_qemu
        get_freertos
        get_riscv_gcc
        build_qemu
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
