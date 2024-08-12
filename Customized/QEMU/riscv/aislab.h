/*
 * QEMU RISC-V AislabIO machine interface
 *
 * Copyright (c) 2017 SiFive, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_RISCV_AISLAB_H
#define HW_RISCV_AISLAB_H

#include "hw/boards.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/sysbus.h"
#include "hw/block/flash.h"
#include "hw/intc/riscv_imsic.h"

#define AISLAB_CPUS_MAX_BITS             9
#define AISLAB_CPUS_MAX                  (1 << AISLAB_CPUS_MAX_BITS)
#define AISLAB_SOCKETS_MAX_BITS          2
#define AISLAB_SOCKETS_MAX               (1 << AISLAB_SOCKETS_MAX_BITS)

#define TYPE_RISCV_AISLAB_MACHINE MACHINE_TYPE_NAME("aislab")
typedef struct RISCVAislabState RISCVAislabState;
DECLARE_INSTANCE_CHECKER(RISCVAislabState, RISCV_AISLAB_MACHINE,
                         TYPE_RISCV_AISLAB_MACHINE)

typedef enum RISCVAislabAIAType {
    AISLAB_AIA_TYPE_NONE = 0,
    AISLAB_AIA_TYPE_APLIC,
    AISLAB_AIA_TYPE_APLIC_IMSIC,
} RISCVAislabAIAType;

struct RISCVAislabState {
    /*< private >*/
    MachineState parent;

    /*< public >*/
    Notifier machine_done;
    DeviceState *platform_bus_dev;
    RISCVHartArrayState soc[AISLAB_SOCKETS_MAX];
    DeviceState *irqchip[AISLAB_SOCKETS_MAX];
    PFlashCFI01 *flash[2];
    FWCfgState *fw_cfg;

    int fdt_size;
    bool have_aclint;
    RISCVAislabAIAType aia_type;
    int aia_guests;
    char *oem_id;
    char *oem_table_id;
    OnOffAuto acpi;
    const MemMapEntry *memmap;
    struct GPEXHost *gpex_host;
};

enum {
    AISLAB_DEBUG,
    AISLAB_MROM,
    AISLAB_TEST,
    AISLAB_RTC,
    AISLAB_CLINT,
    AISLAB_ACLINT_SSWI,
    AISLAB_PLIC,
    AISLAB_APLIC_M,
    AISLAB_APLIC_S,
    AISLAB_UART0,
    AISLAB_VIRTIO,
    AISLAB_FW_CFG,
    AISLAB_IMSIC_M,
    AISLAB_IMSIC_S,
    AISLAB_FLASH,
    AISLAB_DRAM,
    AISLAB_PCIE_MMIO,
    AISLAB_PCIE_PIO,
    AISLAB_PLATFORM_BUS,
    AISLAB_PCIE_ECAM,
    AISLAB_MMIO_BUS,
};

enum {
    UART0_IRQ = 10,
    RTC_IRQ = 11,
    VIRTIO_IRQ = 1, /* 1 to 8 */
    VIRTIO_COUNT = 8,
    PCIE_IRQ = 0x20, /* 32 to 35 */
    AISLAB_PLATFORM_BUS_IRQ = 64, /* 64 to 95 */
    MMIO_BUS_IRQ = 96, /* 96 */
};

#define AISLAB_PLATFORM_BUS_NUM_IRQS 32

#define AISLAB_IRQCHIP_NUM_MSIS 255
#define AISLAB_IRQCHIP_NUM_SOURCES 97 // mmio bus
#define AISLAB_IRQCHIP_NUM_PRIO_BITS 3
#define AISLAB_IRQCHIP_MAX_GUESTS_BITS 3
#define AISLAB_IRQCHIP_MAX_GUESTS ((1U << AISLAB_IRQCHIP_MAX_GUESTS_BITS) - 1U)

#define AISLAB_PLIC_PRIORITY_BASE 0x00
#define AISLAB_PLIC_PENDING_BASE 0x1000
#define AISLAB_PLIC_ENABLE_BASE 0x2000
#define AISLAB_PLIC_ENABLE_STRIDE 0x80
#define AISLAB_PLIC_CONTEXT_BASE 0x200000
#define AISLAB_PLIC_CONTEXT_STRIDE 0x1000
#define AISLAB_PLIC_SIZE(__num_context) \
    (AISLAB_PLIC_CONTEXT_BASE + (__num_context) * AISLAB_PLIC_CONTEXT_STRIDE)

#define FDT_PCI_ADDR_CELLS    3
#define FDT_PCI_INT_CELLS     1
#define FDT_PLIC_ADDR_CELLS   0
#define FDT_PLIC_INT_CELLS    1
#define FDT_APLIC_INT_CELLS   2
#define FDT_IMSIC_INT_CELLS   0
#define FDT_MAX_INT_CELLS     2
#define FDT_MAX_INT_MAP_WIDTH (FDT_PCI_ADDR_CELLS + FDT_PCI_INT_CELLS + \
                                 1 + FDT_MAX_INT_CELLS)
#define FDT_PLIC_INT_MAP_WIDTH  (FDT_PCI_ADDR_CELLS + FDT_PCI_INT_CELLS + \
                                 1 + FDT_PLIC_INT_CELLS)
#define FDT_APLIC_INT_MAP_WIDTH (FDT_PCI_ADDR_CELLS + FDT_PCI_INT_CELLS + \
                                 1 + FDT_APLIC_INT_CELLS)

bool aislab_is_acpi_enabled(RISCVAislabState *s);
void virt_acpi_setup(RISCVAislabState *vms);
uint32_t aislab_imsic_num_bits(uint32_t count);

/*
 * The aislab machine physical address space used by some of the devices
 * namely ACLINT, PLIC, APLIC, and IMSIC depend on number of Sockets,
 * number of CPUs, and number of IMSIC guest files.
 *
 * Various limits defined by AISLAB_SOCKETS_MAX_BITS, AISLAB_CPUS_MAX_BITS,
 * and AISLAB_IRQCHIP_MAX_GUESTS_BITS are tuned for maximum utilization
 * of aislab machine physical address space.
 */

#define AISLAB_IMSIC_GROUP_MAX_SIZE      (1U << IMSIC_MMIO_GROUP_MIN_SHIFT)
#if AISLAB_IMSIC_GROUP_MAX_SIZE < \
    IMSIC_GROUP_SIZE(AISLAB_CPUS_MAX_BITS, AISLAB_IRQCHIP_MAX_GUESTS_BITS)
#error "Can't accommodate single IMSIC group in address space"
#endif

#define AISLAB_IMSIC_MAX_SIZE            (AISLAB_SOCKETS_MAX * \
                                        AISLAB_IMSIC_GROUP_MAX_SIZE)
#if 0x4000000 < AISLAB_IMSIC_MAX_SIZE
#error "Can't accommodate all IMSIC groups in address space"
#endif

#endif
