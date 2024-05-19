#ifndef __CLIENT_UTILS_H__
#define __CLIENT_UTILS_H__

#include <stdint.h>
#include "client/client_requests.h"

/**
 * @file client_utils.h
 * @brief Utility functions for the client application.
 */

/**
 * @brief Initializes the client TCP connection to the server.
 * 
 * @param port_tcp The TCP port to connect to.
 * @param server The server hostname or IP address.
 * @return int The socket file descriptor for the TCP connection, or -1 on error.
 */
int init_client(char *port_tcp, char *server);

/**
 * @brief Subscribes to a multicast group.
 * 
 * @param port_multicast The multicast port to subscribe to.
 * @param adr_multicast The multicast address to subscribe to.
 * @return int The socket file descriptor for the multicast subscription, or -1 on error.
 */
int subscribe_multicast(uint16_t port_multicast, char* adr_multicast);

/**
 * @brief Initializes the UDP connection for the client.
 * 
 * @param hostname The hostname or IP address of the server.
 * @param port_udp The UDP port for the connection.
 * @return UDP_Infos* Pointer to the UDP_Infos structure containing the initialized UDP connection information, or NULL on error.
 */
UDP_Infos *init_udp_connection(char *hostname, uint16_t port_udp);

/**
 * @brief Clears the content of a Message structure.
 * 
 * @param msg Pointer to the Message structure to clear.
 */
void clearMessage(Message* msg);

#endif /* __CLIENT_UTILS_H__ */