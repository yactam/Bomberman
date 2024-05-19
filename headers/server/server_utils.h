#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include "games_handler.h"
#include "data_structures.h"

/**
 * @file server_utils.h
 * @brief Utility functions for server operations.
 */

/** Global variable for the TCP port number. */
extern int TCP_PORT;

/** Global variable for the frequency of server operations. */
extern int FREQ;

/** Global variable for the frequency of sending updates to all clients. */
extern int ALL_FREQ;

/**
 * @brief Initializes the server with the specified port.
 * 
 * @param[in] port The TCP port to initialize the server on.
 * @param[out] serverGames Double pointer to the ServerGames structure.
 * @return int 0 on success, -1 on failure.
 */
int init_server(uint16_t port, ServerGames** serverGames);

/**
 * @brief Launches a game.
 * 
 * @param[in] args Arguments for launching the game.
 * @return void* NULL.
 */
void* launch_game(void* args);

/**
 * @brief Retrieves the TCP sockets for a specified game.
 * 
 * @param[in] ci Array of client information.
 * @param[in] game_udp_port The UDP port of the game.
 * @param[out] res Array to store the retrieved TCP sockets.
 * @param[in] codereq Code request.
 * @param[in] id_team Team ID.
 * @return int The number of sockets retrieved.
 */
int get_tcp_sockets(Array *ci, uint16_t game_udp_port, int *res, uint16_t codereq, uint8_t id_team);

/**
 * @brief Closes the server and frees associated resources.
 * 
 * @param[in] sockfd The socket file descriptor.
 * @param[in, out] serverGames Double pointer to the ServerGames structure.
 * @return int 0 on success, -1 on failure.
 */
int close_server(int sockfd, ServerGames** serverGames);

#endif /* __SERVER_UTILS_H__ */