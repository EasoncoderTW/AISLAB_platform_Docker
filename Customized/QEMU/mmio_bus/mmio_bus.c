#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include "qemu/bitops.h"
#include "hw/misc/mmio_bus.h"
#include "qemu/thread.h"

#include "hw/misc/vpipc.h"

/* QEMU Thread */
static void *tcp_thread_func(void *opaque) {
    printf("[DEBUG] tcp_thread_func\n");
    MMIOBusDevice *dev = (MMIOBusDevice *)opaque;
    
    while (dev->thread_running) {
        // Sleep for 10 ms
        qemu_mutex_lock(&dev->mutex);
        qemu_cond_timedwait(&dev->cond, &dev->mutex, 10);
        qemu_mutex_unlock(&dev->mutex);
    }
    return NULL;
}

/* Design functionality */
static void mmio_bus_set_irq(MMIOBusDevice *s, int irq)
{
    qemu_set_irq(s->irq, 1);
}

static void mmio_bus_clr_irq(MMIOBusDevice *s)
{
    qemu_set_irq(s->irq, 0);
}

static uint64_t mmio_bus_read(void *opaque, hwaddr offset, unsigned size)
{
    MMIOBusDevice *s = (MMIOBusDevice *)opaque;
    struct vp_transfer_data vpt_send, vpt_recv;

    if (!client_is_connect(s->vpm)) {
        fprintf(stderr, "client is not connect\n");
        return 0;
    }

    /* setting vp transfer data*/
    vpt_send.type = VP_READ;
    vpt_send.length = size;
    vpt_send.addr = offset;
    vpt_send.data = 0; // ignore
    
    /* blocking transfer */
    vpt_recv = vp_b_transfer(&s->vpm, vpt_send);
    
    /* check recv type and status */
    if(vpt_recv.type != VP_READ_RESP || vpt_recv.status != VP_OK){
        fprintf(stderr, "mmio_bus read error through the vpipc\n");
    }

    return vpt_recv.data;
}

static void mmio_bus_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    MMIOBusDevice *s = (MMIOBusDevice *)opaque;
    struct vp_transfer_data vpt_send, vpt_recv;

    if (!client_is_connect(s->vpm)) {
        fprintf(stderr, "client is not connect\n");
        return;
    }

    /* setting vp transfer data*/
    vpt_send.type = VP_WRITE;
    vpt_send.length = size;
    vpt_send.addr = offset;
    vpt_send.data = value;
    /* blocking transfer */
    vpt_recv = vp_b_transfer(&s->vpm, vpt_send);
    
    /* check recv type and status */
    if(vpt_recv.type != VP_WRITE_RESP || vpt_recv.status != VP_OK){
        fprintf(stderr, "mmio_bus write error through the vpipc\n");
    }
}

static const MemoryRegionOps mmio_bus_ops = {
    .read = mmio_bus_read,
    .write = mmio_bus_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

/* How to unrealize the device */
static void mmio_bus_instance_finalize(Object *obj)
{
    printf("[DEBUG] mmio_bus_instance_finalize\n");  // Debug statement
}

/* How to unrealize the device */
static void mmio_bus_instance_init(Object *obj)
{
    printf("[DEBUG] mmio_bus_instance_init\n");
}


/* How to realize the device */
static void mmio_bus_realize(DeviceState *dev, Error **errp)
{
    printf("[DEBUG] mmio_bus_realize\n");
    MMIOBusDevice *s = MMIO_BUS(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    memory_region_init_io(&s->iomem, OBJECT(s), &mmio_bus_ops, s,
                          TYPE_MMIO_BUS, MMIO_SIZE);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    // vpipc
    char symbal[] = "-\\|/";
    int i = 0;
    s->vpm = create_vp_module(MODULE_TYPE_SERVER);
    struct vp_transfer vpt[3];
    do
    {
        printf("[AISLAB VP] Waiting systemC to connect ");
        printf("%c \r", symbal[(i/250)]); i = (i+1)%1000;
        vp_wait(&s->vpm, vpt, 2);
    }
    while (!client_is_connect(s->vpm));

    printf("\n[AISLAB VP] SystemC socket Connectted\n");

    // thread
    qemu_mutex_init(&s->mutex);
    qemu_cond_init(&s->cond);
    s->thread_running = true;
    qemu_thread_create(&s->thread, "tcp_thread", tcp_thread_func, s, QEMU_THREAD_DETACHED);
    printf("[DEBUG] qemu_thread_create\n");
}

static void mmio_bus_reset(DeviceState *dev)
{
    printf("[DEBUG] mmio_bus_reset\n");
    MMIOBusDevice *s = MMIO_BUS(dev);
    // Initialize the device

}


/* Enclosure the device into class */
static void mmio_bus_class_init(ObjectClass *klass, void *data)
{
    printf("[DEBUG] mmio_bus_class_init\n");
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = mmio_bus_realize;
    dc->reset = mmio_bus_reset;
}

/* setting device info and class link */
static const TypeInfo mmio_bus_info = {
    .name          = TYPE_MMIO_BUS,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .class_init    = mmio_bus_class_init,
    .instance_size = sizeof(MMIOBusDevice),
    .class_size    = sizeof(MMIOBusDeviceClass),
    .instance_init = mmio_bus_instance_init,
    .instance_finalize = mmio_bus_instance_finalize,
};

/* register device */
static void mmio_bus_register_types(void)
{
    type_register_static(&mmio_bus_info);
}

type_init(mmio_bus_register_types);