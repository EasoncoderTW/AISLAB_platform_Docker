# AISLAB QEMU-systemC co-design Simulator

- Auther - EasonYeh, Yuting
- Version - v0.1
- Date - 2024.07.09
---
- Hardware
    - QEMU - riscv ( virt machine based )
    - systemC - ???
- Software
    - BIOS - opensbi
    - OS - Linux
    - RootFS - busybox
---
## Quick Start
Clone 
```sh
$ git clone https://github.com/EasoncoderTW/AISLAB_platform_Docker.git aisvp
$ cd aisvp
```
Clone base project or your own project
```sh
# clone base project
$ git clone https://github.com/EasoncoderTW/AISLAB_platform_project.git project
```
Run Docker
```sh
$ ./run-docker.sh
```

## Startup usage
In shell, just type `startup` and you can see the help information.
```sh
# In docker env
$ startup
unknown command:

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
```
You can download package and build it separately.
```sh
# In docker env
$ startup get-$PACKAGE_NAME
$ startup build-$PACKAGE_NAME
```
Or you can make it all in once:
```sh
# In docker env
$ startup init
```

## Hardware
Setup Customized QEMU SOC
```sh
# In docker env
$ cd /workspace/Customized
$ ./setup_aislab_soc.sh
$ startup rebuild-qemu
```

Setup Customized Device
- TBD

## Driver
TBD

## Software
TBD