#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "requests.h"

Buf_t hton_integrationrq(CReq_Join *join_rq);
uint8_t send_client_request(int sockfd, CReq *client_rq);
uint8_t recv_server_request(int sockfd, SReq *server_rq);
uint8_t recv_server_datagram(int sockfd, SReq *server_rq, size_t max_recv);
uint8_t send_datagram(int sfd, struct sockaddr_in6 serv_addr, CReq *client_rq);
void init_openSSL();
SSL_CTX* create_context();
SSL* init_tls_connection(int sockfd, char *hostname, SSL_CTX *ctx);
uint8_t send_client_request_tls(SSL *ssl, CReq *client_rq);
ssize_t recv_server_request_tls(SSL *ssl, SReq *server_rq);
void end_ssl_connection(SSL *ssl, int sockfd, SSL_CTX *ctx);

#endif