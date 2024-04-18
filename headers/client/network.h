#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "requests.h"

Buf_t hton_integrationrq(CReq_Join *join_rq);
uint8_t send_client_request(int sockfd, CReq *client_rq);
uint8_t recv_server_request(int sockfd, SReq *server_rq, size_t sto_recv);
uint8_t recv_server_datagram(int sockfd, SReq *server_rq, size_t max_recv);
uint8_t send_datagram(int sfd, struct sockaddr_in6 serv_addr, CReq *client_rq);

#endif