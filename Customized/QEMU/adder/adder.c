#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qom/object.h"
#include "qemu/bitops.h"
#include "hw/misc/adder.h"
#include "qemu/thread.h"

#include "hw/misc/vpipc_pipe.h"

/* QEMU Thread */
static void *tcp_thread_func(void *opaque) {
    printf("[DEBUG] tcp_thread_func\n");
    AdderDevice *dev = (AdderDevice *)opaque;
    // vpipc
    dev->vpm = create_vp_module(MODULE_TYPE_SERVER);
    struct vp_transfer vpt[3];
    do
    {
        vp_wait(&dev->vpm, vpt, 1);
        // Sleep for 10 ms
        qemu_mutex_lock(&dev->mutex);
        qemu_cond_timedwait(&dev->cond, &dev->mutex, 10);
        qemu_mutex_unlock(&dev->mutex);
        //printf("[DEBUG] Waiting...\r");
    }
    while (!client_is_connect(dev->vpm));

    printf("[AISLAB VP] SystemC socket Connectted\n");
    while (dev->thread_running) {
        // Sleep for 10 ms
        qemu_mutex_lock(&dev->mutex);
        qemu_cond_timedwait(&dev->cond, &dev->mutex, 10);
        qemu_mutex_unlock(&dev->mutex);
    }
    return NULL;
}

/* Design functionality */
static void adder_set_irq(AdderDevice *s, int irq)
{
    s->status = irq;
    qemu_set_irq(s->irq, 1);
}

static void adder_clr_irq(AdderDevice *s)
{
    qemu_set_irq(s->irq, 0);
}

static uint64_t adder_read(void *opaque, hwaddr offset, unsigned size)
{
    AdderDevice *s = (AdderDevice *)opaque;
    bool is_enabled = s->init & CHIP_EN;
    struct vp_transfer_data vpt_send, vpt_recv;

    if (!is_enabled) {
        fprintf(stderr, "Device is disabled\\n");
        return 0;
    }

    if (!client_is_connect(s->vpm)) {
        fprintf(stderr, "client is not connect\n");
        return 0;
    }

    switch (offset) {
    case REG_ID:
        return s->id;
    case REG_INIT:
        return s->init;
    case REG_O:
        //return s->output;
    case REG_A:
        //return s->input_A;
    case REG_B:
        //return s->input_B;
        vpt_send.type = VP_READ;
        vpt_send.status = VP_OK;
        vpt_send.addr = offset;
        vpt_send.data = 0;
        vpt_recv = vp_b_transfer(&s->vpm, vpt_send);
        return vpt_recv.data;
    case REG_INT_STATUS:
        adder_clr_irq(s);
        return s->status;
    default:
        break;
    }

    return 0;
}

static void adder_write(void *opaque, hwaddr offset, uint64_t value,
                          unsigned size)
{
    AdderDevice *s = (AdderDevice *)opaque;
    struct vp_transfer_data vpt_send;

    if (!client_is_connect(s->vpm)) {
        fprintf(stderr, "client is not connect\n");
        return;
    }

    switch (offset) {
    case REG_INIT:
        s->init = (int)value;

        if (value)
            adder_set_irq(s, INT_ENABLED);

        break;
    case REG_A:
        //s->input_A = (int)value;
        //s->output = s->input_A + s-> input_B;
        //adder_set_irq(s, INT_BUFFER_DEQ);
        //break;
    case REG_B:
        //s->input_B = (int)value;
        //s->output = s->input_A + s-> input_B;
        vpt_send.type = VP_WRITE;
        vpt_send.status = VP_OK;
        vpt_send.addr = offset;
        vpt_send.data = value;
        vp_b_transfer(&s->vpm, vpt_send);
        adder_set_irq(s, INT_BUFFER_DEQ);
        break;
    default:
        break;
    }
}

static const MemoryRegionOps adder_ops = {
    .read = adder_read,
    .write = adder_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

/* How to unrealize the device */
static void adder_instance_finalize(Object *obj)
{
    printf("[DEBUG] adder_instance_finalize\n");  // Debug statement
}

/* How to unrealize the device */
static void adder_instance_init(Object *obj)
{
    printf("[DEBUG] adder_instance_init\n");
}


/* How to realize the device */
static void adder_realize(DeviceState *dev, Error **errp)
{
    printf("[DEBUG] adder_realize\n");
    AdderDevice *s = ADDER(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    memory_region_init_io(&s->iomem, OBJECT(s), &adder_ops, s,
                          TYPE_ADDER, 0x200);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    // thread
    qemu_mutex_init(&s->mutex);
    qemu_cond_init(&s->cond);
    s->thread_running = true;
    qemu_thread_create(&s->thread, "tcp_thread", tcp_thread_func, s, QEMU_THREAD_DETACHED);
    printf("[DEBUG] qemu_thread_create\n");
}

static void adder_reset(DeviceState *dev)
{
    printf("[DEBUG] adder_reset\n");
    AdderDevice *s = ADDER(dev);
    // Initialize the device
    s->input_A = 0;
    s->input_B = 0;
    s->output = 0;
}


/* Enclosure the device into class */
static void adder_class_init(ObjectClass *klass, void *data)
{
    printf("[DEBUG] adder_class_init\n");
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = adder_realize;
    dc->reset = adder_reset;
}

/* setting device info and class link */
static const TypeInfo adder_info = {
    .name          = TYPE_ADDER,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .class_init    = adder_class_init,
    .instance_size = sizeof(AdderDevice),
    .class_size    = sizeof(AdderDeviceClass),
    .instance_init = adder_instance_init,
    .instance_finalize = adder_instance_finalize,
};

/* register device */
static void adder_register_types(void)
{
    type_register_static(&adder_info);
}

type_init(adder_register_types);