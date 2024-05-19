#ifndef __CLIENT_REQUESTS_H__
#define __CLIENT_REQUESTS_H__

#include "requests.h"
#include "game.h"

/**
 * @file client_requests.h
 * @brief Functions for creating various types of client requests.
 */

/**
 * @brief Creates a join request for integrating the client into the game.
 * 
 * @param clientrq Pointer to the CReq structure to store the created client request.
 * @return int 0 on success, -1 on error.
 */
int create_integrationrq(CReq *clientrq);

/**
 * @brief Creates a confirmation request for the client's game configuration.
 * 
 * @param clientrq Pointer to the CReq structure to store the created client request.
 * @param game_mode The game mode (MODE4 or TEAMS).
 * @param id_player The ID of the player.
 * @param id_team The ID of the team.
 * @return int 0 on success, -1 on error.
 */
int create_confrq(CReq *clientrq, game_mode_t game_mode, uint8_t id_player, uint8_t id_team);

/**
 * @brief Creates an on-game action request for the client.
 * 
 * @param clientrq Pointer to the CReq structure to store the created client request.
 * @param game_mode The game mode (MODE4 or TEAMS).
 * @param id_player The ID of the player.
 * @param id_team The ID of the team.
 * @param num The sequence number.
 * @param action The action to perform (e.g., GO_NORTH, DROP_BOMB).
 * @return int 0 on success, -1 on error.
 */
int create_ongamerq(CReq *clientrq, game_mode_t game_mode, uint8_t id_player, uint8_t id_team, uint16_t num, action_t action);

/**
 * @brief Creates a chat request for the client.
 * 
 * @param clientrq Pointer to the CReq structure to store the created client request.
 * @param id_player The ID of the player.
 * @param id_team The ID of the team.
 * @param tchat Pointer to the Message structure containing the chat message.
 * @param codereq The request code.
 * @return int 0 on success, -1 on error.
 */
int create_chatrq(CReq *clientrq, uint8_t id_player, uint8_t id_team, Message * tchat, u_int16_t codereq);

#endif /* __CLIENT_REQUESTS_H__ */