#ifndef ADDER_H
#define ADDER_H

#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include "qemu/bitops.h"
#include "qemu/thread.h"

#include "hw/misc/vpipc_pipe.h"

#define TYPE_ADDER "adder"

#define ADDER(obj) \
    OBJECT_CHECK(AdderDevice, obj, TYPE_ADDER)

#define TYPE_ADDER_DEVICE "adder-device"
OBJECT_DECLARE_TYPE(AdderDevice, AdderDeviceClass,
                    ADDER_DEVICE)

/* Register map */
#define REG_ID                 0x0
#define CHIP_ID                0xf001

#define REG_INIT               0x4
#define CHIP_EN                BIT(1)

#define REG_A                  0x8
#define REG_B                  0xc
#define REG_O                  0x10

#define REG_INT_STATUS         0x14
#define INT_ENABLED            BIT(0)
#define INT_BUFFER_DEQ         BIT(1)

struct AdderDevice {
    SysBusDevice parent_obj; // is a system Bus device (MMIO)
    MemoryRegion iomem;
    qemu_irq irq;
    QemuThread thread;
    bool thread_running;
    QemuCond cond;
    QemuMutex mutex;

    struct vp_ipc_module vpm;

    uint32_t input_A;
    uint32_t input_B;
    uint32_t output;
    uint32_t id;
    uint32_t init;
    uint32_t status;
};

struct AdderDeviceClass {
    /*< private >*/
    SysBusDeviceClass parent_class;
};

#endif