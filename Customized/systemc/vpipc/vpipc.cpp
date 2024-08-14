/*
 *  Virtrul platform Inner-Process Communications
 *
 *  Auther  : HSUAN-YU Yeh (Eason)
 *  Date    : 2024.07.24
 *  Version : v0.1
 *
 *
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h> // for epoll_create1()

#include "vpipc/vpipc.h"

/*
 *
 *  Basic Utils
 *
 */

const char* VP_Type_str[] =
{
    "VP_WRITE", "VP_READ", "VP_RAISE_IRQ",
    "VP_WRITE_RESP", "VP_READ_RESP", "VP_RAISE_IRQ_RESP",
    "VP_ERROR"
};

/* Create a TCP server and return the socket fd */
int create_vpipc_tcp_server(int port, const char* host, int max_connection)
{
    int sock_fd;
    struct sockaddr_in my_addr;
    int status;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("[VPIPC] Server Socket creation error\n");
        exit(1);
    }
    // for "Address already in use" error message
    int on = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) {
        perror("[VPIPC] Server Setsockopt error\n");
        exit(1);
    }
    // server address
    my_addr.sin_family = AF_INET;
    inet_aton(host, &my_addr.sin_addr);
    my_addr.sin_port = htons(port);
    // bind and listen
    status = bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (status == -1) {
        perror("[VPIPC] Server Binding error\n");
        exit(1);
    }
    printf("[VPIPC] server start at: %s:%d\n", inet_ntoa(my_addr.sin_addr), port);

    // Listen
    status = listen(sock_fd, max_connection);
    if (status == -1) {
        perror("[VPIPC] Server Listening error\n");
        exit(1);
    }

    return sock_fd;
}

/* Create a TCP client and return the socket fd */
int create_vpipc_client(int port, const char* host)
{
    struct sockaddr_in serv_addr;
    int sock_fd;
    int status;

    // create a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("[VPIPC] Client Socket creation error\n");
        exit(1);
    }

    // server address
    serv_addr.sin_family = AF_INET;
    inet_aton(host, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(port);

    status = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status == -1) {
        perror("[VPIPC] Client Connection error\n");
        exit(1);
    }

    return sock_fd;
}

/* create epoll fd */
int create_vpipc_epoll(void)
{
    // Create epoll instance
    int epoll_fd;
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("[VPIPC] epoll_create1 error\n");
        exit(EXIT_FAILURE);
    }
    return epoll_fd;
}

void vp_epoll_add(int epoll_fd,int sock_fd, int events)
{
    // Add the server socket to the epoll instance
    struct epoll_event event;

    event.events = events;
    event.data.fd = sock_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
        perror("[VPIPC] epoll_ctl: sock_fd\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

/* initial a vp module */
struct vp_ipc_module create_vp_module(int type)
{
    // create socket
    struct vp_ipc_module vpm;
    vpm.type = type;
    // create epoll
    vpm.epoll_fd = create_vpipc_epoll();
    if (vpm.epoll_fd == -1) {
        perror("[VPIPC] epoll_create1 error\n");
        exit(EXIT_FAILURE);
    }

    if(type== MODULE_TYPE_SERVER)
    {
        vpm.server_fd = create_vpipc_tcp_server(VP_DEFAULT_PORT, VP_DEFAULT_HOST, MAX_CONNECT);
        vpm.sock_fd = -1;
        vp_epoll_add(vpm.epoll_fd,vpm.server_fd,EPOLLIN);
    }
    else
    {
        vpm.server_fd = -1;
        vpm.sock_fd = create_vpipc_client(VP_DEFAULT_PORT, VP_DEFAULT_HOST);
        vp_epoll_add(vpm.epoll_fd,vpm.sock_fd,EPOLLIN);
    }

    return vpm;
}

void cleanup_vp_module(struct vp_ipc_module vpm)
{
    if(vpm.sock_fd != -1) close(vpm.sock_fd);
    if(vpm.server_fd != -1) close(vpm.server_fd);
    close(vpm.epoll_fd);
}


/* Non-blocking Wait for check if there is any transfer event */
int vp_wait(struct vp_ipc_module *vpm, struct vp_transfer *vpt, int timeout)
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    struct vp_transfer_data recv_vpt;

    int trans_count = 0;
    struct epoll_event events[MAX_EVENTS];

    if(vpm->type == MODULE_TYPE_SERVER)
    {
        /* server */
        //printf("[VPIPC] vp_wait server\n");
        int num_fds = epoll_wait(vpm->epoll_fd, events, MAX_EVENTS, 1);
        if (num_fds == -1) {
            perror("[VPIPC] epoll_wait\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < num_fds; i++) {
            if(events[i].data.fd == vpm->server_fd) {
                // new connection
                vpm->sock_fd = accept(vpm->server_fd, (struct sockaddr *)&client_addr, &addrlen);
                printf("\n[VPIPC] connected by %s:%d\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
                vp_epoll_add(vpm->epoll_fd,vpm->sock_fd,EPOLLIN | EPOLLET); // add new fd to epoll
            }
            else
            {
                // client request
                int client_fd = events[i].data.fd;
                int valread = recv(client_fd, (char*)(&recv_vpt), sizeof(recv_vpt), 0);
                if (valread < 0) {
                    perror("[VPIPC] read\n");
                    close(client_fd);
                    exit(EXIT_FAILURE);
                } else if (valread == 0) {
                    // Connection closed by client
                    printf("[VPIPC] Client disconnected, socket fd: %d\n", client_fd);
                    close(client_fd);
                } else if(valread != sizeof(recv_vpt)){
                    printf("[VPIPC] read size error : %d\n", valread);
                } else {
                    vpt[trans_count].sock_fd = client_fd;
                    vpt[trans_count].data = recv_vpt;
                    trans_count++;
                }
            }
        }

    }
    else
    {
        /* client */
        //printf("[VPIPC] vp_wait client\n");
        int num_fds = epoll_wait(vpm->epoll_fd, events, 1, 1);
        if (num_fds == -1) {
            perror("[VPIPC] epoll_wait\n");
            exit(EXIT_FAILURE);
        }
        if(num_fds > 0)
        {
            if(events[0].data.fd != vpm->sock_fd){
                perror("[VPIPC] sock_fd error\n");
                exit(EXIT_FAILURE);
            }
            int valread = recv(vpm->sock_fd, (char*)(&recv_vpt), sizeof(recv_vpt), 0);
            if (valread < 0) {
                perror("[VPIPC] read\n");
                close(vpm->sock_fd);
                exit(EXIT_FAILURE);
            } else if (valread == 0) {
                // Connection closed by client
                printf("[VPIPC] Server disconnected, socket fd: %d\n", vpm->sock_fd);
                close(vpm->sock_fd);
                vpm->sock_fd = -1;
            } else if(valread != sizeof(recv_vpt)){
                printf("[VPIPC] read size error : %d\n", valread);
            } else {
                vpt[trans_count].sock_fd = vpm->sock_fd;
                vpt[trans_count].data = recv_vpt;
                trans_count++;
            }
        }
    }
    return trans_count;
}

/* Non-blocking response for transfer event */
void vp_nb_response(struct vp_transfer *vpt)
{
    send(vpt->sock_fd,(char*)(&(vpt->data)),sizeof(vpt->data), 0);
}

/* Blocking transfer event */
struct vp_transfer_data vp_b_transfer(struct vp_ipc_module *vpm, struct vp_transfer_data vptdata)
{
    struct vp_transfer resp, trans;
    resp.sock_fd = vpm->sock_fd;
    trans.sock_fd = vpm->sock_fd;

    send(trans.sock_fd,(char*)(&vptdata),sizeof(vptdata), 0);
    recv(resp.sock_fd,(char*)(&(resp.data)),sizeof(resp.data),0); // blocking

    return resp.data;
}

/* check if client is connect */
int client_is_connect(struct vp_ipc_module vpm)
{
    return vpm.sock_fd != -1 ;
}

