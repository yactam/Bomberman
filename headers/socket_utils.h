#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

/**
 * @file socket_utils.h
 * @brief Utility functions for setting socket options.
 */

/**
 * @brief Sets the SO_REUSEADDR option on a socket.
 * 
 * This option allows the socket to be bound to an address that is already in use.
 * 
 * @param sockfd The file descriptor of the socket.
 * @return int 0 on success, -1 on failure.
 */
int set_reuseaddr(int sockfd);

/**
 * @brief Joins a multicast group.
 * 
 * This function sets the necessary socket options to join a multicast group.
 * 
 * @param sock_multicast The file descriptor of the multicast socket.
 * @param adr_multicast The multicast group address to join.
 * @return int 0 on success, -1 on failure.
 */
int set_join_group(int sock_multicast, char *adr_multicast);

/**
 * @brief Sets a socket to non-blocking mode.
 * 
 * In non-blocking mode, calls to functions such as recv() and send() will return immediately,
 * rather than waiting for data to be received or sent.
 * 
 * @param sockfd The file descriptor of the socket.
 * @return int 0 on success, -1 on failure.
 */
int set_non_blocking(int sockfd);

#endif /* __SOCKET_UTILS_H__ */