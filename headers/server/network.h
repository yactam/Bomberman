#ifndef __SNETWORK_H__
#define __SNETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "requests.h"

SReq_Start ntoh_start(Buf_t *start);
Buf_t hton_startrq(SReq_Start *start_rq);
CReq_Join ntoh_integrationrq(Buf_t *buf_rq);
uint8_t recv_client_request(int sockfd, CReq *client_rq, size_t sto_recv);
uint8_t send_server_request(int sockfd, SReq *server_rq);
uint8_t send_datagram(int sfd, struct sockaddr_in6 gradr, SReq *server_rq);

#endif