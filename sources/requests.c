#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "requests.h"
#include "game.h"

void initbuf(Buf_t *buffer) {
    memset(buffer->content, 0, MAX_BUFSIZE);
    buffer->size = 0;
}

int appendbuf(Buf_t *buffer, void *elt, size_t elt_size) {
    if(buffer->size + elt_size >= MAX_BUFSIZE) return 0;
    memcpy(buffer->content + buffer->size, elt, elt_size);
    buffer->size += elt_size;
    return 1;
}

int copyfrombuf(Buf_t *buffer, size_t *start, size_t elt_size, void *dst) {
    if(buffer->size - *start < elt_size) {
        log_error("Le buffer n'a pas %d octects à partir de l'octects %d", elt_size, *start);
        return 0;
    }
    memcpy(dst, buffer->content + *start, elt_size);
    *start += elt_size;
    return 1;
}

uint16_t get_codereq(Header_t header) {
    return header & CODEREQ_MASK;
}

uint16_t get_id(Header_t header) {
    return (header & ID_MASK) >> CODEREQ_LEN;
}

uint16_t get_eq(Header_t header) {
    return (header & EQ_MASK) >> (CODEREQ_LEN + ID_LEN);
}

uint16_t get_num(Message_t message) {
    return message & MESSAGE_MASK;
}

uint8_t get_action(Message_t message) {
    return (message & ACTION_MASK) >> CNUM_LEN;
}

void debug_header(Header_t header) {
    debug("CODEREQ : %d", get_codereq(header));
    debug("ID      : %d", get_id(header));
    debug("EQ      : %d", get_eq(header));
}

void debug_message(Message_t message) {
    debug("NUM : %d", get_num(message));
    debug("ACTION : ");
    switch (get_action(message))
    {
    case GO_NORTH:
        debug("GO_NORTH");
        break;
    case GO_SOUTH:
        debug("GO_SOUTH");
        break;
    case GO_EAST:
        debug("GO_EAST");
        break;
    case GO_OUEST:
        debug("GO_OUEST");
        break;
    case DROP_BOMB:
        debug("DROP_BOMB");
        break;
    case UNDO:
        debug("UNDO");
        break;
    default:
        break;
    }
}

void debug_cjoinrq(CReq_Join *join_rq) {
    debug("**** CLIENT JOIN/CONF REQUEST ****");
    debug_header(join_rq->header);
}

void debug_congamerq(CReq_Play *ongame_rq) {
    debug("**** CLIENT ONGAME REQUEST ****");
    debug_header(ongame_rq->header);
    debug_message(ongame_rq->message);
}

void debug_sstartrq(SReq_Start *start_rq) {
    debug("**** SERVER START REQUEST ****");
    debug_header(start_rq->header);
    debug("PORTUDP  : %d", start_rq->portudp);
    debug("PORTMDIFF: %d", start_rq->portmdiff);
    debug("ADRMDF   : %s", start_rq->adrmdiff);
}

void debug_sgridrq(SReq_Grid *grid_rq) {
    debug("**** SERVER GRID DIFF REQUEST ****");
    debug_header(grid_rq->header);
    debug("NUM     : %d", grid_rq->num);
    debug("HAUTEUR : %d", grid_rq->hauteur);
    debug("LARGEUR : %d", grid_rq->largeur);
    debug("CASES   :");

    GameBoard gameboard = {0};
    gameboard.height = grid_rq->hauteur;
    gameboard.width  = grid_rq->largeur;
    for(size_t i = 0; i < grid_rq->hauteur; ++i) {
        for(size_t j = 0; j < grid_rq->largeur; ++j) {
            gameboard.cells[i][j] = grid_rq->cells[i][j];
        }
    }
    debug_board(gameboard);
}

void debug_scasesrq(SReq_Cell *cell_rq) {
    debug("**** SERVER CASES DIFF REQUEST ****");
    debug_header(cell_rq->header);
    debug("NUM     : %d", cell_rq->num);
    debug("NB      : %d", cell_rq->nb);
    debug("CASES   :");

    for(size_t i = 0; i < cell_rq->nb; ++i) {
        debug("\tROW: %d, COL: %d, CONTENT: %d", cell_rq->cells[i].coord.row, cell_rq->cells[i].coord.col, cell_rq->cells[i].content);
    }
}

void debug_creq(CReq *client_rq) {
    debug("CReq type is %d", client_rq->type);
    if(client_rq->type == CREQ_MODE4 || client_rq->type == CREQ_TEAMS || client_rq->type == CCONF_MODE4 || client_rq->type == CCONF_TEAMS) {
        debug_cjoinrq(&client_rq->req.join);
    } else if(client_rq->type == CON_MODE4 || client_rq->type == CON_TEAMS) {
        debug_congamerq(&client_rq->req.play);
    } else if(client_rq->type == CALL_CHAT || client_rq->type == CCOP_CHAT){
        //TODO debug
    }
}

void debug_sreq(SReq *server_rq) {
    debug("SReq type is %d", server_rq->type);
    if(server_rq->type == SREQ_MODE4 || server_rq->type == SREQ_TEAMS) {
        debug_sstartrq(&server_rq->req.start);
    } else if(server_rq->type == SDIFF_GRID) {
        //debug_sgridrq(&server_rq->req.grid);
    } else if(server_rq->type == SDIFF_CASES) {
        //debug_scasesrq(&server_rq->req.cell);
    } else {
        log_error("NOT YET %d", server_rq->type);
    }
}