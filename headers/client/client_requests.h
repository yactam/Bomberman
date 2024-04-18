#ifndef __CLIENT_REQUESTS_H__
#define __CLIENT_REQUESTS_H__

#include "requests.h"
#include "game.h"

int create_integrationrq(CReq *clientrq);
int create_confrq(CReq *clientrq, game_mode_t game_mode, uint8_t id_player, uint8_t id_team);
int create_ongamerq(CReq *clientrq, game_mode_t game_mode, uint8_t id_player, uint8_t id_team, uint32_t num, action_t action);

#endif