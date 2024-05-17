#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "debug.h"
#include "client/input.h"
#include "server/games_handler.h"

int create_integrationrq(CReq *clientrq) {
    uint8_t gametype = input_game_type();
    Header_t header = (gametype << (EQ_LEN + ID_LEN) | 0);
    clientrq->type = (gametype == 1) ? CREQ_MODE4 : CREQ_TEAMS;
    clientrq->req.join.header = header;
    return 0;
}

int create_confrq(CReq *clientrq, game_mode_t gametype, uint8_t id_player, uint8_t id_team) {
    uint16_t codereq = 0;
    if(gametype == MODE4) {
        codereq = CCONF_MODE4;
    } else if(gametype == TEAMS) {
        codereq = CCONF_TEAMS;
    } else {
        return 1;
    }

    Header_t header = (codereq << (EQ_LEN + ID_LEN) | (id_player << EQ_LEN) | (id_team));
    
    clientrq->type = codereq;
    clientrq->req.join.header = header;

    return 0;
}

int create_ongamerq(CReq *clientrq, game_mode_t game_mode, uint8_t id_player, uint8_t id_team, uint16_t num, action_t action) {
    uint16_t codereq = 0;
    if(game_mode == MODE4) {
        codereq = CON_MODE4;
    } else if(game_mode == TEAMS) {
        codereq = CON_TEAMS;
    } else {
        return 1;
    }

    Header_t header = (codereq << (EQ_LEN + ID_LEN) | (id_player << EQ_LEN) | (id_team));
    
    clientrq->type = codereq;
    clientrq->req.play.header = header;

    Message_t message = (num << ACTION_LEN | action);
    clientrq->req.play.message = message;
    return 0;
}

int create_chatrq(CReq *clientrq, u_int8_t id_player, u_int8_t id_team, Message *tchat, u_int16_t codereq) {
    Header_t header = (codereq | (id_player << CODEREQ_LEN) | (id_team << (CODEREQ_LEN + ID_LEN)));

    clientrq->type = codereq;
    clientrq->req.tchat.header = header;
    clientrq->req.tchat.len = tchat->len-1;

    memcpy(clientrq->req.tchat.data,tchat->data+1,tchat->len-1);

    return 0;
}