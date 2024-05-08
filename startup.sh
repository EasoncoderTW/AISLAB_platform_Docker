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

    startup                             : show the welcome and usage message
    startup build-qemu                  : build qemu
    startup build-systemc [version]     : build SystemC library (default version: 2.3.1)
    startup get-riscv-gcc               : download riscv-gnu-toolchain Cross Compiler

EOF
}

build_qemu(){
    echo 'build '$state' in '$mountdir
    cd qemu
    mkdir -p build || exit 1
    cd build || exit 1
    ../configure --enable-debug-info --target-list=riscv64-softmmu --enable-virtfs || exit 1
    make -j $(nproc) || exit 1
}

get_riscv_gcc(){
    dir=$mountdir
    file=riscv64-elf-ubuntu-20.04-gcc-nightly-2024.04.12-nightly.tar.gz
    [[ -f $file ]] && {
        echo $file exists in $dir
    } || {
        wget https://github.com/riscv-collab/riscv-gnu-toolchain/releases/download/2024.04.12/$file -O - | tar xvC $dir
    }
}

if [ $# -eq 0 ]; then
    usage_message
fi

if [[ $state == 'build-qemu' ]]
then
    cd $mountdir
    build_qemu
fi

if [[ $state == 'build-systemc' ]]
then
    version=$arg1
    bash $mountdir/Docker/modules/systemc/systemc.sh $version
fi

if [[ $state == 'get-riscv-gcc' ]]
then
    cd $mountdir
    get_riscv_gcc
fi