#ifndef MMIO_BUS_H
#define MMIO_BUS_H

#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include "qemu/bitops.h"
#include "qemu/thread.h"

#include "hw/misc/vpipc.h"

#define TYPE_MMIO_BUS "mmio_bus"
#define MMIO_SIZE 0x6000000
#define MMIO_ADDR 0x6000000


#define MMIO_BUS(obj) \
    OBJECT_CHECK(MMIOBusDevice, obj, TYPE_MMIO_BUS)

#define TYPE_MMIO_BUS_DEVICE "mmio_bus-device"
OBJECT_DECLARE_TYPE(MMIOBusDevice, MMIOBusDeviceClass,
                    MMIO_BUS_DEVICE)

#define INT_ENABLED            BIT(0)
#define INT_BUFFER_DEQ         BIT(1)

struct MMIOBusDevice {
    SysBusDevice parent_obj; // is a system Bus device (MMIO)
    MemoryRegion iomem;
    qemu_irq irq;
    QemuThread thread;
    bool thread_running;
    QemuCond cond;
    QemuMutex mutex;

    struct vp_ipc_module vpm;
};

struct MMIOBusDeviceClass {
    /*< private >*/
    SysBusDeviceClass parent_class;
};

#endif