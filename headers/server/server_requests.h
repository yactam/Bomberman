#ifndef __SERVER_REQUESTS_H__
#define __SERVER_REQUESTS_H__

#include "requests.h"
#include "games_handler.h"
#include "game.h"

int create_regestartionrq(ServerGames **server_games, SReq *serverrq, CReq_Join *clientrq, int sfd);
int create_multicastrq(SReq *serverrq, GameBoard gameboard, uint16_t num);
int create_cellrq(SReq *serverrq, GameBoard prev_board, GameBoard board, uint16_t num, uint16_t nb_cases);
int create_tchatrq(SReq *serverrq, Creq_Tchat *clientrq);
int create_endrq(SReq *serverrq, uint8_t winner, game_mode_t mode);

#endif