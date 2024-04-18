#ifndef __CLIENT_UTILS_H__
#define __CLIENT_UTILS_H__

#include <stdint.h>
#include "client/client_requests.h"

typedef struct {
    int sock_udp;
    struct sockaddr_in6 server_addr;
} UDP_Infos;

int init_client(uint16_t port_tcp, char *server_ip);
int subscribe_multicast(uint16_t port_multicast, char* adr_multicast);
UDP_Infos *init_udp_connection(uint16_t port_udp);
int sock_reuse_addr(int sock);

#endif