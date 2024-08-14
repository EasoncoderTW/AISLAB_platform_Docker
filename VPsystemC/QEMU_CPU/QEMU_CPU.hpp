#ifndef QEMU_CPU_HPP
#define QEMU_CPU_HPP

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#define MMIO_BUS_ADDR 0x6000000
#define MMIO_BUS_SIZE 0x6000000

/* vpipc */
#include "vpipc/vpipc.h"

class QEMU_CPU : public sc_core::sc_module {
private:
    struct vp_ipc_module vpm;
    struct vp_transfer vpt[3];
    struct vp_transfer vpt_resp;
public:
    tlm_utils::simple_initiator_socket<QEMU_CPU> socket_master;
#ifdef DMA
    tlm_utils::simple_target_socket<QEMU_CPU> socket_slave;
#endif

    SC_CTOR(QEMU_CPU) : socket_master("socket_master")
#ifdef DMA
    ,socket_slave("socket_slave")
#endif
    {
#ifdef DMA
        socket_slave.register_b_transport( this, &QEMU_CPU::b_transport);
#endif
        SC_THREAD(thread_process);
    }

#ifdef DMA
    // TLM-2 blocking transport method
  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address() / 4;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    // Obliged to check address range and check for unsupported features,
    //   i.e. byte enables, streaming, and bursts
    // Can ignore extensions

    // *********************************************
    // Generate the appropriate error response
    // *********************************************

    if (adr < sc_dt::uint64(MMIO_BUS_ADDR) || adr > sc_dt::uint64(MMIO_BUS_ADDR+MMIO_BUS_SIZE) ) {
      trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
      return;
    }
    if (byt != 0) {
      trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
      return;
    }
    if (len > 4 || wid < len) {
      trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
      return;
    }

    // Obliged to implement read and write commands
    if ( cmd == tlm::TLM_READ_COMMAND ) // read
    {
        struct vp_transfer_data vpt_send,vpt_recv;
        vpt_send.addr = adr;
        vpt_send.length = len;
        vpt_send.data = 0;
        vpt_send.type = VP_READ;
        vpt_recv =  vp_b_transfer(&vpm, vpt_send);
        if(vpt_recv.type == VP_READ_RESP && vpt_recv.status == VP_OK)
        {
            memcpy(ptr, &vpt_send.data, len);
        }
        else
        {
            printf("[QEMU_CPU] read from qemu error");
        }
    }
    else if ( cmd == tlm::TLM_WRITE_COMMAND ) // write
    {
        struct vp_transfer_data vpt_send,vpt_recv;
        vpt_send.addr = adr;
        memcpy(&vpt_send.data, ptr, len);
        vpt_send.length = len;
        vpt_send.type = VP_WRITE;
        vpt_recv =  vp_b_transfer(&vpm, vpt_send);
        if(vpt_recv.type != VP_WRITE_RESP || vpt_recv.status != VP_OK)
        {
            printf("[QEMU_CPU] write to qemu error");
        }
    }

    // Illustrates that b_transport may block
    wait(delay);

    // Reset timing annotation after waiting
    delay = SC_ZERO_TIME;

    // Obliged to set response status to indicate successful completion
    trans.set_response_status( tlm::TLM_OK_RESPONSE );
  }
#endif

    void thread_process() {
        // vpipc
        vpm = create_vp_module(MODULE_TYPE_CLIENT);
        int num;

        while(1)
        {
            num = vp_wait(&vpm, vpt, 1);
            if(num > 0)
            {
#ifdef DEBUG
                printf("Type: %s, Length: %ld, Address: 0x%016lx, Data: 0x%016lx\n", VP_Type_str[vpt[0].data.type], vpt[0].data.status, vpt[0].data.addr, vpt[0].data.data);
#endif
                vpt_resp.sock_fd = vpt[0].sock_fd;
                switch (vpt[0].data.type)
                {
                    case VP_WRITE:
                        tlm_write(vpt[0].data); // Use systemC TLM 2.0 write transfer
                        vpt_resp.data.type = VP_WRITE_RESP;
                        vpt_resp.data.status = VP_OK;
                        vpt_resp.data.addr = vpt[0].data.addr;
                        break;
                    case VP_READ:
                        vpt_resp.data.type = VP_READ_RESP;
                        vpt_resp.data.status = VP_OK;
                        vpt_resp.data.addr = vpt[0].data.addr;
                        vpt_resp.data.data = tlm_read(vpt[0].data); // Use systemC TLM 2.0 read transfer
                        break;
                    default:
                        vpt_resp.data.type = VP_ERROR;
                        vpt_resp.data.status = VP_FAIL;
                        break;
                }
                vp_nb_response(&vpt_resp);
            }
            /* check if server (QEMU CPU) is still alive */
            if(!client_is_connect(vpm))
            {
                break;
            }
        }
        cleanup_vp_module(vpm);
    }

    uint64_t tlm_read(vp_transfer_data trans_data)
    {
        tlm::tlm_generic_payload trans;
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        // Set up a read transaction
        trans.set_command(tlm::TLM_READ_COMMAND);
        trans.set_address(trans_data.addr); // Address in the MMIO range
        trans.set_data_length(trans_data.length); // Size of the data in bytes
        trans.set_streaming_width(4);
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        uint32_t data = 0;
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));

        socket_master->b_transport(trans, delay);

        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        }

        return (uint64_t)data;
    }

    void tlm_write(vp_transfer_data trans_data)
    {
        tlm::tlm_generic_payload trans;
        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        // Set up a read transaction
        trans.set_command(tlm::TLM_WRITE_COMMAND);
        trans.set_address(trans_data.addr); // Address in the MMIO range
        trans.set_data_length(trans_data.length); // Size of the data in bytes
        trans.set_streaming_width(4);
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        uint32_t data = (uint32_t)trans_data.data;
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));

        socket_master->b_transport(trans, delay);

        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        }

    }
};
#endif