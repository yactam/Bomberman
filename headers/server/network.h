#ifndef __SNETWORK_H__
#define __SNETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "requests.h"

/**
 * @file snetwork.h
 * @brief Network utility functions for handling server and client communications.
 */

/**
 * @brief Converts a network byte order buffer to a SReq_Start structure.
 * 
 * @param start Pointer to the buffer containing the network byte order data.
 * @return SReq_Start The converted SReq_Start structure.
 */
SReq_Start ntoh_start(Buf_t *start);

/**
 * @brief Converts a SReq_Start structure to network byte order buffer.
 * 
 * @param start_rq Pointer to the SReq_Start structure to convert.
 * @return Buf_t The buffer containing the network byte order data.
 */
Buf_t hton_startrq(SReq_Start *start_rq);

/**
 * @brief Converts a network byte order buffer to a CReq_Join structure.
 * 
 * @param buf_rq Pointer to the buffer containing the network byte order data.
 * @return CReq_Join The converted CReq_Join structure.
 */
CReq_Join ntoh_integrationrq(Buf_t *buf_rq);

/**
 * @brief Receives a client request from a socket.
 * 
 * @param sockfd The socket file descriptor to read from.
 * @param client_rq Pointer to the CReq structure to store the received data.
 * @return ssize_t The number of bytes received, or -1 on error.
 */
ssize_t recv_client_request(int sockfd, CReq *client_rq);

/**
 * @brief Sends a server request to multiple client sockets.
 * 
 * @param sockfd Array of socket file descriptors to send to.
 * @param nb_socks Number of sockets in the array.
 * @param server_rq Pointer to the SReq structure containing the data to send.
 * @return uint8_t 0 on success, 1 on error.
 */
uint8_t send_server_request(int *sockfd, size_t nb_socks, SReq *server_rq);

/**
 * @brief Sends a datagram to a specified address.
 * 
 * @param sfd The socket file descriptor to send from.
 * @param gradr The address to send the datagram to.
 * @param server_rq Pointer to the SReq structure containing the data to send.
 * @return uint8_t 0 on success, 1 on error.
 */
uint8_t send_datagram(int sfd, struct sockaddr_in6 gradr, SReq *server_rq);

/**
 * @brief Receives client datagrams on a socket.
 * 
 * @param sfd The socket file descriptor to read from.
 * @param client_rq Pointer to the CReq structure to store the received data.
 * @return uint8_t 0 on success, 1 on error.
 */
uint8_t recv_client_datagrams(int sfd, CReq *client_rq);

#endif /* __SNETWORK_H__ */