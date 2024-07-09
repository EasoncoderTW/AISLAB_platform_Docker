# Software Porting (OpenSBI + Linux)

## 1. Software Porting (OpenSBI + Linux)

### 1.1 Introduction to Software Porting

軟體移植涉及將軟體適配到不同的硬體或軟體環境，使其能夠在原本設計之外的平台上運行。這個過程對於增強軟體的兼容性、提高其可用性和覆蓋範圍至關重要。

### 1.2 Overview of OpenSBI and its Role in Bootstrapping

OpenSBI（開源監督者二進制接口）是一個提供 RISC-V 監督者二進制接口 (SBI) 參考實現的項目。它用於啟動 RISC-V 系統，初始化硬體並為操作系統（如 Linux）提供服務。OpenSBI 在啟動過程中起著關鍵作用，通過設置執行環境並將控制權交給操作系統內核。

### 1.3 Downloading and Compiling OpenSBI  

要開始使用 OpenSBI，請按照以下步驟操作：

#### 步驟 1：安裝前置需求

確保您的系統上安裝了必要的工具：

```sh
# riscv-gnu-toolchain
git clone https://github.com/riscv-collab/riscv-gnu-toolchain
cd riscv-gnu-toolchain
./configure --prefix=$RISCV_DEST --with-arch=rv64gc --with-abi=lp64d --disable-gdb
make -j$(nproc) linux
```

#### 步驟 2：Clone OpenSBI 存儲庫

從 GitHub clone OpenSBI 存儲庫：

```sh
git clone https://github.com/riscv/opensbi.git
cd opensbi
```

#### 步驟 3：編譯 OpenSBI

使用以下命令編譯 OpenSBI：

```sh
make PLATFORM=generic CROSS_COMPILE=riscv64-unknown-linux-gnu- O=build -j$(nproc) [-FW_PAYLOAD_PATH=$YOUR_PAYLOAD.bin]
```
- device-tree based: 通用
- 這將在 `build/platform/generic/firmware` 目錄中生成 OpenSBI 固件二進制文件。

- 檢查 ELF file
```
riscv64-unknown-linux-gnu-readelf build/platform/fw_payload.bin -h
```

### 1.4 Preparing the Linux Kernel for QEMU
- [Running 64- and 32-bit RISC-V Linux on QEMU](https://risc-v-getting-started-guide.readthedocs.io/en/latest/linux-qemu.html)
- [在QEMU上執行64 bit RISC-V Linux](https://medium.com/swark/%E5%9C%A8qemu%E4%B8%8A%E5%9F%B7%E8%A1%8C64-bit-risc-v-linux-2a527a078819)
要為 QEMU 準備 Linux 內核，請按照以下步驟操作：

#### 步驟 1：安裝 QEMU

- 安裝 QEMU，它是模擬 RISC-V 硬體所需的工具
```sh
# wget and extract
wget https://download.qemu.org/qemu-9.0.1.tar.xz -O - | tar xJC $WHERE_TO_EXTRACT
# rename
mv qemu-9.0.1 qemu
# build
mkdir -p $WHERE_TO_EXTRACT/qemu/build
cd $WHERE_TO_EXTRACT/qemu/build
../configure --enable-debug-info --target-list=riscv64-softmmu,riscv32-softmmu --enable-virtfs
make -j $(nproc)
```

#### 步驟 2：下載 Linux kernel source code

下載 Linux kernel source code：

```sh
git clone https://github.com/torvalds/linux
cd linux
```

#### 步驟 3：配置 Linux kernel

Config (defconfig: Default, menuconfig:可以自行設定) :

```sh
make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- defconfig
```

#### 步驟 4：編譯 Linux kernel

編譯 Linux 內核：

```sh
make ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- -j $(nproc)
```

這將生成 `/linux/arch/riscv/boot/Image` kernel image。

### 1.5 Booting Linux with OpenSBI on QEMU (Using Busybox root-fs)
- 參考 [使用 Busybox 建立 RISC-V 的迷你系統](https://coldnew.github.io/6cc46ece/)
要使用 OpenSBI 在 QEMU 上啟動 Linux，請按照以下步驟操作：

#### 步驟 1: 下載 Busybox
下載 Busybox source code：
```sh
git clone git://git.busybox.net/busybox
cd busybox
```
切換至穩定版本 :
```sh
git branch -a # 查詢版本
git checkout -b 1_36_stable origin/1_36_stable
```
#### 步驟 2：安裝 Busybox
設定 configuration
```sh
ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- make menuconfig
```
- 注意以下設定
  ```sh
  Busybox Settings  --->
          Build Options  --->
                [*] Build BusyBox as a static binary (no shared libs)

  Init Utilities  --->
          [*] init

  Networking Utilities  --->
          [ ] inetd

  Shells  --->
          [*] ash
  ```
Build and Install :
```sh
ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- make
ARCH=riscv CROSS_COMPILE=riscv64-unknown-linux-gnu- make install
```
完成後可以在`/busybox/_install/` 中看到 file system

#### 步驟 3：創建磁碟映像

創建一個磁碟映像以用作根文件系統：
```sh
# 操作目錄:　./busybox
cd $PATH_TO_BUSYBOX
# 建立 root-fs 空間 (1024 MB)
dd if=/dev/zero of=root.ext2 bs=1M count=1024
# 格式化成 ext2
mkfs.ext2 rootfs.ext2
```
掛載磁碟映像並將必要的文件（例如基本的 Linux 文件系統）複製到其中。
```sh
# 操作目錄:　./busybox

# 掛載 root-fs (mount)
mkdir -p /tmp/root
sudo mount root.ext2 /tmp/root
# 移植 busybox root-fs
sudo rsync -avr _install/* /tmp/root
```
建立缺少的資料夾
```sh
cd /tmp/root && sudo mkdir -p proc sys dev etc/init.d
```
建立 etc/init.d/rcS 作為啟動腳本
```sh
sudo vim /tmp/root/etc/init.d/rcS
```
```sh
#!/bin/sh
# etc/init.d/rcS
mount -t proc none /proc
mount -t sysfs none /sys
/sbin/mdev -s

# set shared dir ( ignore if no need )
mkdir -p /mnt/shared
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt/shared

# set hostname
hostname aislabvp
```
增加權限
```sh
sudo chmod +x /tmp/root/etc/init.d/rcS
```
卸載 root.exts
```sh
sudo umount /tmp/root
```

#### 步驟 4：使用 QEMU 啟動
使用 QEMU 啟動系統並加載 OpenSBI 和 Linux kernal：
```sh
mkdir -p ./shared
qemu-system-riscv64 \
    -M virt \
    -m 256M \
    -nographic \
    -bios $PATH_TO_OPENSBI/build/platform/generic/firmware/fw_jump.bin \
    -kernel $PATH_TO_LINUX/arch/riscv/boot/Image \
    -drive file=$PATH_TO_BUSYBOX/root.ext2,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    -append "root=/dev/vda rw console=ttyS0" \
    -virtfs local,path=./shared,mount_tag=host0,security_model=mapped,id=host0
```
將 `$PATH_TO_OPENSBI`、`$PATH_TO_LINUX`、`$PATH_TO_BUSYBOX` 換成對應的安裝路徑

1. 使用 `fw_jump.bin` 作為 bios
2. 使用 `Image` 為 kernel
3. `./shared` 為與 qemu 內部共享的儲存空間

通過這些步驟，應該能夠成功地移植 OpenSBI 和 Linux，並在 QEMU 模擬的 RISC-V 環境中運行它們。

