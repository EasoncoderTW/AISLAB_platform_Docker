# IPC (Inter-Process Communication) Tutorial

## 4. IPC (Inter-Process Communication)

### 4.1 Introduction to IPC
Inter-Process Communication (IPC) 是一種允許多個進程在操作系統中相互通信和同步的技術。IPC 機制包括信號、管道、FIFO、消息隊列、共享內存、信號量和互斥鎖。

### 4.2 Types of IPC Mechanisms
IPC 機制主要包括：
- **Pipes and FIFOs**: 用於在相關或無關進程之間傳遞數據的單向通信通道。
- **Message Queues**: 用於進程間傳遞消息的隊列，支持異步通信。
- **Shared Memory**: 允許多個進程訪問相同的內存區域，實現高速數據共享。
- **Semaphores**: 用於進程間同步的計數器機制。
- **Mutexes**: 用於保護共享資源，確保同一時間只有一個進程能夠訪問該資源。

### 4.3 Using Pipes and FIFOs for IPC
#### 步驟 1：創建和使用管道
創建一個名為 `pipe_example.c` 的文件：
```c
#include <stdio.h>
#include <unistd.h>

int main() {
    int fd[2];
    pid_t pid;
    char buffer[20];

    // 創建管道
    pipe(fd);

    pid = fork();

    if (pid > 0) { // 父進程
        close(fd[0]); // 關閉讀端
        write(fd[1], "Hello, IPC", 10);
        close(fd[1]); // 關閉寫端
    } else { // 子進程
        close(fd[1]); // 關閉寫端
        read(fd[0], buffer, 10);
        printf("Received: %s\n", buffer);
        close(fd[0]); // 關閉讀端
    }

    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
gcc -o pipe_example pipe_example.c
./pipe_example
```

#### 步驟 3：創建和使用 FIFO
創建一個名為 `fifo_example.c` 的文件：
```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    const char *fifo_path = "/tmp/my_fifo";
    char buffer[20];

    // 創建 FIFO
    mkfifo(fifo_path, 0666);

    if (fork() > 0) { // 父進程
        int fd = open(fifo_path, O_WRONLY);
        write(fd, "Hello, FIFO", 11);
        close(fd);
    } else { // 子進程
        int fd = open(fifo_path, O_RDONLY);
        read(fd, buffer, 11);
        printf("Received: %s\n", buffer);
        close(fd);
    }

    // 刪除 FIFO
    unlink(fifo_path);
    return 0;
}
```

#### 步驟 4：編譯和運行
使用以下命令編譯並運行程序：
```sh
gcc -o fifo_example fifo_example.c
./fifo_example
```

### 4.4 Message Queues and Shared Memory
#### 步驟 1：使用消息隊列
創建一個名為 `msg_queue_example.c` 的文件：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct message {
    long msg_type;
    char msg_text[100];
};

int main() {
    key_t key;
    int msgid;
    struct message msg;

    // 創建消息隊列
    key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    if (fork() > 0) { // 父進程
        msg.msg_type = 1;
        strcpy(msg.msg_text, "Hello, Message Queue");
        msgsnd(msgid, &msg, sizeof(msg), 0);
    } else { // 子進程
        msgrcv(msgid, &msg, sizeof(msg), 1, 0);
        printf("Received: %s\n", msg.msg_text);
    }

    // 刪除消息隊列
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
gcc -o msg_queue_example msg_queue_example.c
./msg_queue_example
```

#### 步驟 3：使用共享內存
創建一個名為 `shm_example.c` 的文件：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    key_t key;
    int shmid;
    char *data;

    // 創建共享內存
    key = ftok("shmfile", 65);
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    data = (char*) shmat(shmid, (void*)0, 0);

    if (fork() > 0) { // 父進程
        strcpy(data, "Hello, Shared Memory");
        shmdt(data);
    } else { // 子進程
        sleep(1); // 確保父進程已寫入數據
        printf("Received: %s\n", data);
        shmdt(data);
    }

    // 刪除共享內存
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
```

#### 步驟 4：編譯和運行
使用以下命令編譯並運行程序：
```sh
gcc -o shm_example shm_example.c
./shm_example
```

### 4.5 Semaphores and Mutexes in IPC
#### 步驟 1：使用信號量
創建一個名為 `semaphore_example.c` 的文件：
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

void sem_lock(int semid) {
    struct sembuf p = {0, -1, SEM_UNDO};
    semop(semid, &p, 1);
}

void sem_unlock(int semid) {
    struct sembuf v = {0, 1, SEM_UNDO};
    semop(semid, &v, 1);
}

int main() {
    key_t key;
    int semid;

    // 創建信號量
    key = ftok("semfile", 65);
    semid = semget(key, 1, 0666 | IPC_CREAT);
    semctl(semid, 0, SETVAL, 1);

    if (fork() > 0) { // 父進程
        sem_lock(semid);
        printf("Parent entering critical section\n");
        sleep(2);
        printf("Parent leaving critical section\n");
        sem_unlock(semid);
    } else { // 子進程
        sleep(1); // 確保父進程先獲得信號量
        sem_lock(semid);
        printf("Child entering critical section\n");
        sleep(2);
        printf("Child leaving critical section\n");
        sem_unlock(semid);
    }

    // 刪除信號量
    semctl(semid, 0, IPC_RMID);
    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
gcc -o semaphore_example semaphore_example.c
./semaphore_example
```

### 4.6 Practical Examples of IPC in QEMU
在 QEMU 中實現 IPC，可以模擬不同虛擬設備之間的通信。

#### 步驟 1：修改 QEMU 源碼以使用 IPC
例如，創建一個新設備使用共享內存進行通信。可以參考 QEMU 開發文檔，了解如何創建和編譯新的虛擬設備。

#### 步驟 2：創建一個 QEMU 設備，使用 FIFO 與主機進行通信
這需要修改 QEMU 源碼，並在合適的位置添加 IPC 機制，例如：
```c
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define TYPE_IPC_DEVICE "ipc-device"
#define IPC_DEVICE(obj) \
    OBJECT_CHECK(IPCDeviceState, (obj), TYPE_IPC_DEVICE)

typedef struct IPCDeviceState {
    SysBusDevice parent

_obj;
    char *fifo_path;
    int fd;
} IPCDeviceState;

static void ipc_device_realize(DeviceState *dev, Error **errp) {
    IPCDeviceState *s = IPC_DEVICE(dev);
    s->fifo_path = g_strdup("/tmp/qemu_fifo");
    mkfifo(s->fifo_path, 0666);
    s->fd = open(s->fifo_path, O_WRONLY);
}

static void ipc_device_unrealize(DeviceState *dev) {
    IPCDeviceState *s = IPC_DEVICE(dev);
    close(s->fd);
    unlink(s->fifo_path);
    g_free(s->fifo_path);
}

static void ipc_device_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = ipc_device_realize;
    dc->unrealize = ipc_device_unrealize;
}

static const TypeInfo ipc_device_info = {
    .name = TYPE_IPC_DEVICE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(IPCDeviceState),
    .class_init = ipc_device_class_init,
};

static void ipc_device_register_types(void) {
    type_register_static(&ipc_device_info);
}

type_init(ipc_device_register_types);
```

這段程式碼展示了如何創建一個 QEMU 設備，該設備在實現時創建並打開一個 FIFO，並在清理時關閉並刪除該 FIFO。

這些步驟和示例程式碼應該能夠幫助你理解和實現 IPC 機制，並應用於 QEMU 模擬環境中。如果需要更詳細的說明或有其他問題，請隨時告訴我！
