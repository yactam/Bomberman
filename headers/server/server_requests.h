#ifndef __SERVER_REQUESTS_H__
#define __SERVER_REQUESTS_H__

#include "requests.h"
#include "games_handler.h"
#include "game.h"

/**
 * @file server_requests.h
 * @brief Functions for creating server requests based on client actions and game state.
 */

/**
 * @brief Creates a registration request for a new client joining the game.
 * 
 * @param[in, out] server_games Pointer to the ServerGames structure.
 * @param[out] serverrq Pointer to the server request structure to be populated.
 * @param[in] clientrq Pointer to the client join request.
 * @param[in] sfd Socket file descriptor for the client.
 * @param[in] ssl SSL context for the client connection.
 * @return int 0 on success, -1 on failure.
 */
int create_regestartionrq(ServerGames **server_games, SReq *serverrq, CReq_Join *clientrq, int sfd, SSL* ssl);

/**
 * @brief Creates a multicast request for broadcasting the game board state.
 * 
 * @param[out] serverrq Pointer to the server request structure to be populated.
 * @param[in] gameboard The current state of the game board.
 * @param[in] num The game number or identifier.
 * @return int 0 on success, -1 on failure.
 */
int create_multicastrq(SReq *serverrq, GameBoard gameboard, uint16_t num);

/**
 * @brief Creates a request for updating specific cells on the game board.
 * 
 * @param[out] serverrq Pointer to the server request structure to be populated.
 * @param[in] prev_board The previous state of the game board.
 * @param[in] board The current state of the game board.
 * @param[in] num The game number or identifier.
 * @param[in] nb_cases The number of cases (cells) to update.
 * @return int 0 on success, -1 on failure.
 */
int create_cellrq(SReq *serverrq, GameBoard prev_board, GameBoard board, uint16_t num, uint16_t nb_cases);

/**
 * @brief Creates a chat request for broadcasting a chat message from a client.
 * 
 * @param[out] serverrq Pointer to the server request structure to be populated.
 * @param[in] clientrq Pointer to the client chat request.
 * @return int 0 on success, -1 on failure.
 */
int create_tchatrq(SReq *serverrq, Creq_Tchat *clientrq);

/**
 * @brief Creates a game end request indicating the winner and game mode.
 * 
 * @param[out] serverrq Pointer to the server request structure to be populated.
 * @param[in] winner The identifier of the winning player.
 * @param[in] mode The game mode (e.g., MODE4, TEAMS).
 * @return int 0 on success, -1 on failure.
 */
int create_endrq(SReq *serverrq, uint8_t winner, game_mode_t mode);

#endif /* __SERVER_REQUESTS_H__ */