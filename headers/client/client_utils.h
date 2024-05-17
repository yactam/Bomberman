#ifndef __CLIENT_UTILS_H__
#define __CLIENT_UTILS_H__

#include <stdint.h>
#include "client/client_requests.h"

int init_client(uint16_t port_tcp, char *server_ip);
int subscribe_multicast(uint16_t port_multicast, char* adr_multicast);
UDP_Infos *init_udp_connection(uint16_t port_udp);
int sock_reuse_addr(int sock);
Message *initMessage(u_int8_t,u_int8_t);
void ntoh_tchat(Message *msg, int socket); // reecrit le msg en le transformant en la struct
void clearMessage(Message* msg);
void freeMessage(Message *msg);
#endif