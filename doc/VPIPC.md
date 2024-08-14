# VPIPC protocol
This section will illustrate how to develop hardware using `VPIPC` protocol if you do not use systemC to simulate.
- `VPIPC` is a customized protocol that can transfer read and write event between two device, which is simplified from systemC TLM 2.0 protocol.
- Default IP: `127.0.0.1:7000` (localhost)
- Transfer Type
```cpp
// Transfer Type
enum VP_Type
{
    VP_WRITE, VP_READ, VP_RAISE_IRQ,
    VP_WRITE_RESP, VP_READ_RESP, VP_RAISE_IRQ_RESP,
    VP_ERROR
};
```
- Transfer package data
```cpp
struct vp_transfer_data{
    uint64_t type;
    union {
        uint64_t status; // for response
        uint64_t length; // for request
    };
    uint64_t addr;
    uint64_t data;
};
```
- VPIPC examples
```cpp
#include <stdio.h>
#include <stdlib.h>
#include "vpipc_pipe.h"


int main()
{
    struct vp_ipc_module vpm = create_vp_module(MODULE_TYPE_CLIENT);
    struct vp_transfer vpt[3];
    struct vp_transfer vpt_resp;
    int num;
    while(1)
    {
        num = vp_wait(&vpm, vpt, 1);
        if(num > 0)
        {
            printf("Type: %s, Status: %ld, Address: 0x%016lx, Data: 0x%016lx\n", VP_Type_str[vpt[0].data.type], vpt[0].data.status, vpt[0].data.addr, vpt[0].data.data);
            vpt_resp.sock_fd = vpt[0].sock_fd;
            switch (vpt[0].data.type)
            {
                case VP_WRITE:
                    vpt_resp.data.type = VP_WRITE_RESP;
                    vpt_resp.data.status = VP_OK;
                    break;
                case VP_READ:
                    vpt_resp.data.type = VP_READ_RESP;
                    vpt_resp.data.status = VP_OK;
                    vpt_resp.data.addr = vpt[0].data.addr;
                    vpt_resp.data.data = 0xdeadbeef;
                    break;
                case VP_RAISE_IRQ:
                    vpt_resp.data.type = VP_RAISE_IRQ_RESP;
                    vpt_resp.data.status = VP_OK;
                    break;
                default:
                    vpt_resp.data.type = VP_ERROR;
                    vpt_resp.data.status = VP_FAIL;
                    break;
            }
            vp_nb_response(&vpt_resp);
        }
    }
    cleanup_vp_module(vpm);
    return 0;
}
```