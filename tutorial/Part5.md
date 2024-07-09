# QEMU-SystemC Co-Design Tutorial

## 5. QEMU-SystemC Co-Design

### 5.1 Introduction to Co-Design Concepts
QEMU 和 SystemC 的聯合設計（Co-Design）允許模擬和驗證硬體和軟體的整合系統。這種方法可以在虛擬環境中進行高效的硬體/軟體協同設計和調試，從而減少實體硬體開發的風險和成本。

### 5.2 Setting Up a QEMU-SystemC Co-Design Environment
#### 步驟 1：安裝 QEMU
首先，安裝 QEMU。如果尚未安裝，可以使用以下命令：
```sh
sudo apt-get install qemu
```

#### 步驟 2：安裝 SystemC
參考之前的 SystemC 安裝步驟，確保已正確安裝 SystemC 並設置環境變數。

#### 步驟 3：安裝 TLM-2.0
下載並安裝 TLM-2.0（Transaction-Level Modeling）的標準庫：
```sh
wget http://www.accellera.org/images/downloads/standards/tlm/tlm-2_0_1.tgz
tar -xvf tlm-2_0_1.tgz
```

#### 步驟 4：設置 SystemC 和 TLM-2.0
在 `~/.bashrc` 或 `~/.zshrc` 中添加以下行：
```sh
export SYSTEMC_HOME=/usr/local/systemc
export TLM_HOME=/path/to/tlm-2_0_1
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$SYSTEMC_HOME/include:$TLM_HOME/include:$CPLUS_INCLUDE_PATH
```
然後重新加載配置：
```sh
source ~/.bashrc
```

### 5.3 Creating a Simple QEMU-SystemC Co-Design Project
#### 步驟 1：創建 SystemC 模塊
創建一個名為 `simple_systemc.cpp` 的文件：
```cpp
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

struct SimpleModule : sc_module {
    tlm_utils::simple_target_socket<SimpleModule> socket;

    SC_CTOR(SimpleModule) : socket("socket") {
        socket.register_b_transport(this, &SimpleModule::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        std::cout << "Received transaction at " << sc_time_stamp() << std::endl;
    }
};

int sc_main(int argc, char* argv[]) {
    SimpleModule module("module");
    sc_start();
    return 0;
}
```

#### 步驟 2：集成到 QEMU
在 QEMU 中創建一個簡單設備，與 SystemC 模塊通信。修改 QEMU 源碼，創建新設備並設置與 SystemC 通信的接口。

### 5.4 Synchronizing QEMU and SystemC
#### 步驟 1：創建同步機制
使用事件機制或 FIFO/管道進行同步。在 QEMU 設備的實現中，創建與 SystemC 進行通信的接口，並在適當的時間點進行同步。

#### 步驟 2：實現同步
在 QEMU 設備中實現同步邏輯，例如：
```c
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_SIMPLE_DEVICE "simple-device"
#define SIMPLE_DEVICE(obj) \
    OBJECT_CHECK(SimpleDeviceState, (obj), TYPE_SIMPLE_DEVICE)

typedef struct SimpleDeviceState {
    SysBusDevice parent_obj;
    char *fifo_path;
    int fd;
} SimpleDeviceState;

static void simple_device_realize(DeviceState *dev, Error **errp) {
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    s->fifo_path = g_strdup("/tmp/qemu_fifo");
    mkfifo(s->fifo_path, 0666);
    s->fd = open(s->fifo_path, O_WRONLY);
    // 與 SystemC 模塊同步
}

static void simple_device_unrealize(DeviceState *dev) {
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    close(s->fd);
    unlink(s->fifo_path);
    g_free(s->fifo_path);
}

static void simple_device_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = simple_device_realize;
    dc->unrealize = simple_device_unrealize;
}

static const TypeInfo simple_device_info = {
    .name = TYPE_SIMPLE_DEVICE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SimpleDeviceState),
    .class_init = simple_device_class_init,
};

static void simple_device_register_types(void) {
    type_register_static(&simple_device_info);
}

type_init(simple_device_register_types);
```

### 5.5 Debugging and Profiling in a Co-Design Environment
#### 步驟 1：使用 SystemC 和 QEMU 的調試工具
使用 GDB 和 SystemC 提供的調試工具，在聯合設計環境中進行調試。

#### 步驟 2：設置斷點和查看變量
在 QEMU 和 SystemC 中設置斷點，並查看重要變量的值。確保 QEMU 和 SystemC 之間的通信正確無誤。

### 5.6 Case Studies and Real-World Examples
#### 案例研究 1：簡單計算器
創建一個簡單計算器，模擬硬體和軟體的協同工作。使用 SystemC 模擬計算器的硬體部分，使用 QEMU 模擬運行計算器軟體的虛擬機。

#### 案例研究 2：嵌入式系統模擬
模擬一個完整的嵌入式系統，包括處理器、內存和外設。使用 QEMU 模擬處理器和內存，使用 SystemC 模擬外設。

```cpp
// 示例代碼
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

struct Initiator : sc_module {
    tlm_utils::simple_initiator_socket<Initiator> socket;

    SC_CTOR(Initiator) : socket("socket") {
        SC_THREAD(process);
    }

    void process() {
        tlm::tlm_generic_payload trans;
        sc_time delay = SC_ZERO_TIME;
        trans.set_command(tlm::TLM_WRITE_COMMAND);
        trans.set_address(0);
        trans.set_data_length(4);
        unsigned int data = 42;
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
        socket->b_transport(trans, delay);
    }
};

struct Target : sc_module {
    tlm_utils::simple_target_socket<Target> socket;

    SC_CTOR(Target) : socket("socket") {
        socket.register_b_transport(this, &Target::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
        unsigned int* data = reinterpret_cast<unsigned int*>(trans.get_data_ptr());
        std::cout << "Received data: " << *data << " at " << sc_time_stamp() << std::endl;
    }
};

int sc_main(int argc, char* argv[]) {
    Initiator initiator("initiator");
    Target target("target");

    initiator.socket.bind(target.socket);

    sc_start();
    return 0;
}
```

這些步驟和範例應能幫助你開始使用 QEMU 和 SystemC 進行聯合設計。如果需要更詳細的說明或有任何其他問題，請隨時告訴我！

---
---
---
---
---

### 5.4 Synchronizing QEMU and SystemC

#### 使用 IPC 同步 QEMU 和 SystemC

在這個例子中，我們將使用命名管道 (FIFO) 作為 QEMU 和 SystemC 之間的通信和同步機制。

#### 步驟 1：創建 SystemC 模塊並與 QEMU 通信

1. 創建一個名為 `simple_systemc.cpp` 的文件：

```cpp
#include <systemc.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

SC_MODULE(SimpleModule) {
    SC_CTOR(SimpleModule) {
        SC_THREAD(process);
    }

    void process() {
        int fd;
        char buffer[100];

        // 打開 FIFO
        fd = open("/tmp/qemu_fifo", O_RDONLY);
        if (fd == -1) {
            perror("Error opening FIFO");
            return;
        }

        while (true) {
            // 從 FIFO 讀取數據
            int n = read(fd, buffer, sizeof(buffer));
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "SystemC received: " << buffer << " at " << sc_time_stamp() << std::endl;
            }
            wait(10, SC_NS); // 模擬延遲
        }

        close(fd);
    }
};

int sc_main(int argc, char* argv[]) {
    SimpleModule module("module");
    sc_start();
    return 0;
}
```

2. 編譯並運行 SystemC 模塊：

```sh
g++ -I. -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -o simple_systemc simple_systemc.cpp -lsystemc -lm
./simple_systemc
```

#### 步驟 2：修改 QEMU 設備實現與 SystemC 通信

1. 在 QEMU 源碼中創建一個簡單設備，與 SystemC 進行通信。創建一個新文件 `hw/misc/simple_device.c`：

```c
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define TYPE_SIMPLE_DEVICE "simple-device"
#define SIMPLE_DEVICE(obj) \
    OBJECT_CHECK(SimpleDeviceState, (obj), TYPE_SIMPLE_DEVICE)

typedef struct SimpleDeviceState {
    SysBusDevice parent_obj;
    char *fifo_path;
    int fd;
} SimpleDeviceState;

static void simple_device_realize(DeviceState *dev, Error **errp) {
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    s->fifo_path = g_strdup("/tmp/qemu_fifo");
    mkfifo(s->fifo_path, 0666);
    s->fd = open(s->fifo_path, O_WRONLY);
    if (s->fd == -1) {
        perror("Error opening FIFO");
        return;
    }

    // 向 SystemC 發送數據
    const char *message = "Hello from QEMU";
    write(s->fd, message, strlen(message));
}

static void simple_device_unrealize(DeviceState *dev) {
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    close(s->fd);
    unlink(s->fifo_path);
    g_free(s->fifo_path);
}

static void simple_device_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = simple_device_realize;
    dc->unrealize = simple_device_unrealize;
}

static const TypeInfo simple_device_info = {
    .name = TYPE_SIMPLE_DEVICE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SimpleDeviceState),
    .class_init = simple_device_class_init,
};

static void simple_device_register_types(void) {
    type_register_static(&simple_device_info);
}

type_init(simple_device_register_types);
```

2. 編譯並運行 QEMU，確保新設備被加載：

```sh
./configure --target-list=x86_64-softmmu
make
```

運行 QEMU，並加載新創建的設備（這取決於你的 QEMU 配置和使用的設備）：

```sh
qemu-system-x86_64 -device simple-device
```

### 5.4 完整步驟總結

1. **設置 FIFO**：在 SystemC 模塊中設置 FIFO，並創建一個 SystemC 模塊來讀取 FIFO 中的數據。
2. **修改 QEMU 設備**：在 QEMU 中創建或修改一個設備，該設備將打開 FIFO，並向 SystemC 模塊發送數據。
3. **同步機制**：使用 FIFO 作為同步機制，保證 QEMU 和 SystemC 之間的數據通信。


---
---
---
---
---

### 5.4 Synchronizing QEMU and SystemC Using Shared Memory

在这个例子中，我们将使用共享内存作为 QEMU 和 SystemC 之间的通信和同步机制。

#### 步骤 1：创建 SystemC 模块并与 QEMU 通信

1. 创建一个名为 `simple_systemc.cpp` 的文件：

```cpp
#include <systemc.h>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

SC_MODULE(SimpleModule) {
    SC_CTOR(SimpleModule) {
        SC_THREAD(process);
    }

    void process() {
        int shm_fd;
        void *shm_ptr;
        const char *shm_name = "/qemu_systemc_shm";
        const size_t shm_size = 4096;

        // 打开共享内存对象
        shm_fd = shm_open(shm_name, O_RDONLY, 0666);
        if (shm_fd == -1) {
            perror("Error opening shared memory");
            return;
        }

        // 映射共享内存
        shm_ptr = mmap(0, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            perror("Error mapping shared memory");
            close(shm_fd);
            return;
        }

        while (true) {
            // 从共享内存读取数据
            std::cout << "SystemC received: " << static_cast<char*>(shm_ptr) << " at " << sc_time_stamp() << std::endl;
            wait(10, SC_NS); // 模拟延迟
        }

        // 取消映射共享内存
        munmap(shm_ptr, shm_size);
        close(shm_fd);
    }
};

int sc_main(int argc, char* argv[]) {
    SimpleModule module("module");
    sc_start();
    return 0;
}
```

2. 编译并运行 SystemC 模块：

```sh
g++ -I. -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -o simple_systemc simple_systemc.cpp -lsystemc -lm
./simple_systemc
```

#### 步骤 2：修改 QEMU 设备实现与 SystemC 通信

1. 在 QEMU 源码中创建一个简单设备，与 SystemC 进行通信。创建一个新文件 `hw/misc/simple_device.c`：

```c
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define TYPE_SIMPLE_DEVICE "simple-device"
#define SIMPLE_DEVICE(obj) \
    OBJECT_CHECK(SimpleDeviceState, (obj), TYPE_SIMPLE_DEVICE)

typedef struct SimpleDeviceState {
    SysBusDevice parent_obj;
    char *shm_name;
    int shm_fd;
    void *shm_ptr;
} SimpleDeviceState;

static void simple_device_realize(DeviceState *dev, Error **errp) {
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    s->shm_name = g_strdup("/qemu_systemc_shm");
    const size_t shm_size = 4096;

    // 创建共享内存对象
    s->shm_fd = shm_open(s->shm_name, O_CREAT | O_RDWR, 0666);
    if (s->shm_fd == -1) {
        perror("Error creating shared memory");
        return;
    }

    // 设置共享内存大小
    if (ftruncate(s->shm_fd, shm_size) == -1) {
        perror("Error setting shared memory size");
        return;
    }

    // 映射共享内存
    s->shm_ptr = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, s->shm_fd, 0);
    if (s->shm_ptr == MAP_FAILED) {
        perror("Error mapping shared memory");
        return;
    }

    // 向共享内存写入数据
    const char *message = "Hello from QEMU";
    strncpy((char*)s->shm_ptr, message, shm_size);
}

static void simple_device_unrealize(DeviceState *dev) {
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    const size_t shm_size = 4096;

    // 取消映射共享内存
    munmap(s->shm_ptr, shm_size);
    close(s->shm_fd);
    shm_unlink(s->shm_name);
    g_free(s->shm_name);
}

static void simple_device_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = simple_device_realize;
    dc->unrealize = simple_device_unrealize;
}

static const TypeInfo simple_device_info = {
    .name = TYPE_SIMPLE_DEVICE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SimpleDeviceState),
    .class_init = simple_device_class_init,
};

static void simple_device_register_types(void) {
    type_register_static(&simple_device_info);
}

type_init(simple_device_register_types);
```

2. 编译并运行 QEMU，确保新设备被加载：

```sh
./configure --target-list=x86_64-softmmu
make
```

运行 QEMU，并加载新创建的设备（这取决于你的 QEMU 配置和使用的设备）：

```sh
qemu-system-x86_64 -device simple-device
```

### 5.4 完整步骤总结

1. **设置共享内存**：在 SystemC 模块中设置共享内存，并创建一个 SystemC 模块来读取共享内存中的数据。
2. **修改 QEMU 设备**：在 QEMU 中创建或修改一个设备，该设备将创建和映射共享内存，并向 SystemC 模块写入数据。
3. **同步机制**：使用共享内存作为同步机制，保证 QEMU 和 SystemC 之间的数据通信。

