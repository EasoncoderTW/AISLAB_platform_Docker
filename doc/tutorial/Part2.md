
# QEMU Simulator

## 2. QOM (QEMU Object Model)

### 2.1 Introduction to QOM
QEMU Object Model (QOM) 是 QEMU 中的核心架構，提供了一個面向對象的框架，用於構建和管理 QEMU 的各種設備和元件。QOM 允許開發人員創建靈活且可擴展的虛擬設備，並在模擬中進行有效的管理和配置。

### 2.2 Understanding the QEMU Object Model
QOM 的核心概念包括：
- **Object**: 基本的對象單位，每個對象都有類型和屬性。
- **Class**: 定義對象的類型，包含類別方法和屬性。
- **Property**: 對象的屬性，可以是基本數據類型或複合類型。
- **Realize**: 將對象實例化，使其在模擬中變為活躍。

### 2.3 Creating and Using Simple QOM Objects
這裡我們創建一個簡單的 QOM 對象，展示如何定義類別和實例化對象。

#### 步驟 1：建立新對象類別
首先，創建一個新的 QOM 類別文件，例如 `simple-device.c`：

```c
#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_SIMPLE_DEVICE "simple-device"
#define SIMPLE_DEVICE(obj) \
    OBJECT_CHECK(SimpleDeviceState, (obj), TYPE_SIMPLE_DEVICE)

typedef struct SimpleDeviceState {
    SysBusDevice parent_obj;
    uint32_t some_property;
} SimpleDeviceState;

static void simple_device_realize(DeviceState *dev, Error **errp)
{
    SimpleDeviceState *s = SIMPLE_DEVICE(dev);
    // Initialize the device
    s->some_property = 0;
}

static void simple_device_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = simple_device_realize;
}

static const TypeInfo simple_device_info = {
    .name          = TYPE_SIMPLE_DEVICE,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SimpleDeviceState),
    .class_init    = simple_device_class_init,
};

static void simple_device_register_types(void)
{
    type_register_static(&simple_device_info);
}

type_init(simple_device_register_types);
```

#### 步驟 2：編譯並添加到 QEMU
將新文件添加到 QEMU 的構建系統中，修改 `Makefile` 或相關的構建腳本以包含 `simple-device.c`。例如，在 `hw/Makefile.objs` 中添加：

```makefile
hw-obj-y += simple-device.o
```

#### 步驟 3：實例化並使用新對象
在 QEMU 中實例化並使用這個新設備。可以在 QEMU 的命令行中添加新設備：

```sh
$ qemu-system-x86_64 -device simple-device
```

### 2.4 Advanced QOM Object Usage
進一步擴展 QOM 對象，包括添加更多屬性和方法，實現複雜的設備邏輯。

#### 步驟 1：擴展對象屬性
修改 `simple-device.c` 文件，添加新的屬性：

```c
static Property simple_device_properties[] = {
    DEFINE_PROP_UINT32("some-property", SimpleDeviceState, some_property, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void simple_device_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = simple_device_realize;
    dc->props = simple_device_properties;
}
```

#### 步驟 2：處理屬性變更
在實現中處理屬性變更，確保屬性能夠正確地影響設備的行為：

```c
static void simple_device_set_property(Object *obj, Visitor *v, const char *name, void *opaque, Error **errp)
{
    SimpleDeviceState *s = SIMPLE_DEVICE(obj);
    if (strcmp(name, "some-property") == 0) {
        visit_type_uint32(v, name, &s->some_property, errp);
    }
}

static void simple_device_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = simple_device_realize;
    dc->props = simple_device_properties;
    object_class_set_property(klass, "some-property", simple_device_set_property, simple_device_get_property, 0);
}
```

### 2.5 Debugging and Testing QOM Objects
對 QOM 對象進行調試和測試，以確保其正確性和穩定性。

#### 調試 QOM 對象
使用 QEMU 的調試功能，例如 `-d` 選項來啟用詳細的調試信息：

```sh
$ qemu-system-x86_64 -device simple-device -d guest_errors
```

#### 單元測試
創建單元測試來驗證 QOM 對象的行為，確保其在各種情況下正常工作。

```c
void test_simple_device_property(void)
{
    SimpleDeviceState *dev = SIMPLE_DEVICE(object_new(TYPE_SIMPLE_DEVICE));
    object_property_set_int(OBJECT(dev), "some-property", 42, NULL);
    g_assert(dev->some_property == 42);
}
```

通過這些步驟，你可以創建、使用和調試 QEMU 中的 QOM 對象，並構建複雜的虛擬設備。
