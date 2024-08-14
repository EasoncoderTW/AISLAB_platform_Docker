#ifndef TOP_H
#define TOP_H

#include "QEMU_CPU/QEMU_CPU.hpp"
#include "target.h"

SC_MODULE(Top)
{
  QEMU_CPU *qemu_cpu;
  Memory    *memory;

  SC_CTOR(Top)
  {
    // Instantiate components
    qemu_cpu = new QEMU_CPU("QEMU_CPU");
    memory    = new Memory   ("memory");

    // One initiator is bound directly to one target with no intervening bus

    // Bind initiator socket to target socket
    qemu_cpu->socket_master.bind(memory->socket);
  }
};

#endif
