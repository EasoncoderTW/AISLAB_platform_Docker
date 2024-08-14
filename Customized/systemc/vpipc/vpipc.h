/*
 *  Virtrul platform Inner-Process Communications
 *
 *  Auther  : HSUAN-YU Yeh (Eason)
 *  Date    : 2024.07.24
 *  Version : v0.1
 *
 *
 */
#ifndef VPIPC_PIPE_H
#define VPIPC_PIPE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h> // for epoll_create1()

/*
 *
 *  Type and Module define
 *
 */

#define VP_DEFAULT_PORT   7000
#define VP_DEFAULT_HOST    "127.0.0.1"
#define MAX_CONNECT       10
#define MAX_EVENTS        10

// Transfer Type
enum VP_Type
{
    VP_WRITE, VP_READ, VP_RAISE_IRQ,
    VP_WRITE_RESP, VP_READ_RESP, VP_RAISE_IRQ_RESP,
    VP_ERROR
};

extern const char* VP_Type_str[];

// STATUS
#define VP_OK      0
#define VP_FAIL    1

struct vp_transfer_data{
    uint64_t type;
    union {
        uint64_t status;
        uint64_t length;
    };
    uint64_t addr;
    uint64_t data;
};

struct vp_transfer{
    int sock_fd;
    struct vp_transfer_data data;
};

#define MODULE_TYPE_SERVER 0
#define MODULE_TYPE_CLIENT 1

struct vp_ipc_module{
    int type;
    int epoll_fd;
    int sock_fd;
    int server_fd;
};

/*
 *
 *  Basic Utils
 *
 */

/* Create a TCP server and return the socket fd */
int create_vpipc_tcp_server(int port, const char* host, int max_connection);

/* Create a TCP client and return the socket fd */
int create_vpipc_client(int port, const char* host);

/* create epoll fd */
int create_vpipc_epoll(void);

void vp_epoll_add(int epoll_fd,int sock_fd, int events);

/* initial a vp module */
struct vp_ipc_module create_vp_module(int type);

void cleanup_vp_module(struct vp_ipc_module vpm);

/* Non-blocking Wait for check if there is any transfer event */
int vp_wait(struct vp_ipc_module *vpm, struct vp_transfer *vpt, int timeout);

/* Non-blocking response for transfer event */
void vp_nb_response(struct vp_transfer *vpt);

/* Blocking transfer event */
struct vp_transfer_data vp_b_transfer(struct vp_ipc_module *vpm, struct vp_transfer_data vptdata);

/* check if client is connect */
int client_is_connect(struct vp_ipc_module vpm);

#endif