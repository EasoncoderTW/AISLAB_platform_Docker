# SystemC Tutorial

## 3. SystemC

### 3.1 Introduction to SystemC
SystemC 是一個用於系統級設計的 C++ 標準庫，提供了硬體設計和驗證的抽象。它允許設計者在高層次上進行建模、模擬和測試系統，包括硬體和軟體的共同設計。

### 3.2 Installing and Setting Up SystemC
#### 步驟 1：下載 SystemC
從官方網站下載 SystemC 源碼：
```sh
wget https://accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
tar -xvf systemc-2.3.3.tar.gz
```

#### 步驟 2：編譯和安裝 SystemC
進入解壓的目錄並進行編譯和安裝：
```sh
cd systemc-2.3.3
mkdir objdir
cd objdir
../configure --prefix=/usr/local/systemc
make
sudo make install
```

#### 步驟 3：配置環境變數
在 `~/.bashrc` 或 `~/.zshrc` 中添加以下行：
```sh
export SYSTEMC_HOME=/usr/local/systemc
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$SYSTEMC_HOME/include:$CPLUS_INCLUDE_PATH
```
然後重新加載配置：
```sh
source ~/.bashrc
```

### 3.3 Basic SystemC Modules and Processes
#### 步驟 1：創建一個基本的 SystemC 模組
創建一個名為 `basic_module.cpp` 的文件：
```cpp
#include <systemc.h>

SC_MODULE(BasicModule) {
    SC_CTOR(BasicModule) {
        SC_THREAD(main_process);
    }

    void main_process() {
        while (true) {
            std::cout << "Hello, SystemC!" << std::endl;
            wait(1, SC_SEC); // Wait for 1 second
        }
    }
};

int sc_main(int argc, char* argv[]) {
    BasicModule module("basic_module");
    sc_start();
    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -o basic_module basic_module.cpp -lsystemc -lm
./basic_module
```

### 3.4 Communication in SystemC
#### 步驟 1：使用信號進行模組間通信
創建一個名為 `communication.cpp` 的文件：
```cpp
#include <systemc.h>

SC_MODULE(Producer) {
    sc_out<int> out_signal;
    SC_CTOR(Producer) {
        SC_THREAD(produce);
    }

    void produce() {
        int value = 0;
        while (true) {
            out_signal.write(value++);
            wait(1, SC_SEC);
        }
    }
};

SC_MODULE(Consumer) {
    sc_in<int> in_signal;
    SC_CTOR(Consumer) {
        SC_THREAD(consume);
    }

    void consume() {
        while (true) {
            int value = in_signal.read();
            std::cout << "Received value: " << value << std::endl;
            wait(1, SC_SEC);
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sc_signal<int> signal;
    Producer producer("producer");
    Consumer consumer("consumer");

    producer.out_signal(signal);
    consumer.in_signal(signal);

    sc_start();
    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -o communication communication.cpp -lsystemc -lm
./communication
```

### 3.5 Timing and Synchronization in SystemC
#### 步驟 1：使用事件進行同步
創建一個名為 `timing_sync.cpp` 的文件：
```cpp
#include <systemc.h>

SC_MODULE(TimedModule) {
    sc_event event;
    SC_CTOR(TimedModule) {
        SC_THREAD(trigger_event);
        SC_THREAD(wait_for_event);
    }

    void trigger_event() {
        while (true) {
            wait(2, SC_SEC);
            event.notify();
        }
    }

    void wait_for_event() {
        while (true) {
            wait(event);
            std::cout << "Event received at " << sc_time_stamp() << std::endl;
        }
    }
};

int sc_main(int argc, char* argv[]) {
    TimedModule module("timed_module");
    sc_start();
    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -o timing_sync timing_sync.cpp -lsystemc -lm
./timing_sync
```

### 3.6 Example Projects with SystemC
#### 步驟 1：創建一個簡單的 ALU 模型
創建一個名為 `alu.cpp` 的文件：
```cpp
#include <systemc.h>

SC_MODULE(ALU) {
    sc_in<int> a, b;
    sc_in<char> op;
    sc_out<int> result;

    SC_CTOR(ALU) {
        SC_METHOD(process);
        sensitive << a << b << op;
    }

    void process() {
        switch(op.read()) {
            case '+': result.write(a.read() + b.read()); break;
            case '-': result.write(a.read() - b.read()); break;
            case '*': result.write(a.read() * b.read()); break;
            case '/': 
                if (b.read() != 0)
                    result.write(a.read() / b.read()); 
                else
                    result.write(0); // Handle divide by zero
                break;
            default: result.write(0); break;
        }
    }
};

int sc_main(int argc, char* argv[]) {
    sc_signal<int> a, b, result;
    sc_signal<char> op;

    ALU alu("alu");
    alu.a(a);
    alu.b(b);
    alu.op(op);
    alu.result(result);

    a.write(10);
    b.write(5);
    op.write('+');
    sc_start(1, SC_NS);
    std::cout << "10 + 5 = " << result.read() << std::endl;

    op.write('*');
    sc_start(1, SC_NS);
    std::cout << "10 * 5 = " << result.read() << std::endl;

    return 0;
}
```

#### 步驟 2：編譯和運行
使用以下命令編譯並運行程序：
```sh
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -o alu alu.cpp -lsystemc -lm
./alu
```

通過這些步驟，你可以開始使用 SystemC 進行基本和進階的硬體設計建模。這些示例應該能幫助你理解 SystemC 的基本概念和用法。如果需要更詳細的說明或有任何其他問題，請隨時告訴我！
