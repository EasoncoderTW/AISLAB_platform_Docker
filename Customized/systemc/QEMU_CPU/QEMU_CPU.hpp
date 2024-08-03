#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

/* vpipc */
#include "vpipc/vpipc.h"

class QEMU_CPU : public sc_core::sc_module {
private:
    struct vp_ipc_module vpm;
    struct vp_transfer vpt[3];
    struct vp_transfer vpt_resp;
public:
    tlm_utils::simple_initiator_socket<QEMU_CPU> socket;

    SC_CTOR(QEMU_CPU) : socket("socket") {
        SC_THREAD(thread_process);
    }

    void thread_process() {
        // vpipc
        vpm = create_vp_module(MODULE_TYPE_CLIENT);
        int num;
    
        while(1)
        {
            num = vp_wait(&vpm, vpt, 1);
            if(num > 0)
            {
                //printf("Type: %s, Status: %ld, Address: 0x%016lx, Data: 0x%016lx\n", VP_Type_str[vpt[0].data.type], vpt[0].data.status, vpt[0].data.addr, vpt[0].data.data);
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

        socket->b_transport(trans, delay);

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

        socket->b_transport(trans, delay);

        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        }

    }
};