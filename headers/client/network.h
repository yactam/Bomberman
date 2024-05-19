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

/**
 * @file network.h
 * @brief Functions for network communication between client and server.
 */

/**
 * @brief Converts a client's join request structure to network byte order.
 * 
 * @param join_rq Pointer to the CReq_Join structure containing the join request.
 * @return Buf_t Buffer containing the join request in network byte order.
 */
Buf_t hton_integrationrq(CReq_Join *join_rq);

/**
 * @brief Sends a client request over a socket.
 * 
 * @param sockfd The socket file descriptor.
 * @param client_rq Pointer to the CReq structure containing the client request.
 * @return uint8_t 0 on success, non-zero on failure.
 */
uint8_t send_client_request(int sockfd, CReq *client_rq);

/**
 * @brief Receives a server request from a socket.
 * 
 * @param sockfd The socket file descriptor.
 * @param server_rq Pointer to the SReq structure to store the received server request.
 * @return ssize_t Number of bytes received on success, -1 on failure.
 */
ssize_t recv_server_request(int sockfd, SReq *server_rq);

/**
 * @brief Receives a server datagram from a socket.
 * 
 * @param sockfd The socket file descriptor.
 * @param server_rq Pointer to the SReq structure to store the received server request.
 * @param max_recv Maximum number of bytes to receive.
 * @return uint8_t 0 on success, non-zero on failure.
 */
uint8_t recv_server_datagram(int sockfd, SReq *server_rq, size_t max_recv);

/**
 * @brief Sends a datagram over a socket.
 * 
 * @param sfd The socket file descriptor.
 * @param serv_addr The server address.
 * @param client_rq Pointer to the CReq structure containing the client request.
 * @return uint8_t 0 on success, non-zero on failure.
 */
uint8_t send_datagram(int sfd, struct sockaddr_in6 serv_addr, CReq *client_rq);

/**
 * @brief Initializes OpenSSL library.
 */
void init_openSSL();

/**
 * @brief Creates an SSL context.
 * 
 * @return SSL_CTX* Pointer to the SSL context on success, NULL on failure.
 */
SSL_CTX* create_context();

/**
 * @brief Initializes a TLS connection over a socket.
 * 
 * @param sockfd The socket file descriptor.
 * @param hostname The hostname of the server.
 * @param ctx The SSL context.
 * @return SSL* Pointer to the SSL structure on success, NULL on failure.
 */
SSL* init_tls_connection(int sockfd, char *hostname, SSL_CTX *ctx);

/**
 * @brief Sends a client request over a TLS connection.
 * 
 * @param ssl The SSL structure.
 * @param client_rq Pointer to the CReq structure containing the client request.
 * @return uint8_t 0 on success, non-zero on failure.
 */
uint8_t send_client_request_tls(SSL *ssl, CReq *client_rq);

/**
 * @brief Receives a server request over a TLS connection.
 * 
 * @param ssl The SSL structure.
 * @param server_rq Pointer to the SReq structure to store the received server request.
 * @return ssize_t Number of bytes received on success, -1 on failure.
 */
ssize_t recv_server_request_tls(SSL *ssl, SReq *server_rq);

/**
 * @brief Ends an SSL connection.
 * 
 * @param ssl The SSL structure.
 * @param sockfd The socket file descriptor.
 * @param ctx The SSL context.
 */
void end_ssl_connection(SSL *ssl, int sockfd, SSL_CTX *ctx);

#endif /* __NETWORK_H__ */
