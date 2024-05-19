#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

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

#endif /* __NETWORK_H__ */