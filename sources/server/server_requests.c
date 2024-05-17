#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include "debug.h"
#include "requests.h"
#include "server/games_handler.h"
#include "game.h"

int create_regestartionrq(ServerGames **server_games, SReq *serverrq, CReq_Join *clientrq) {
    game_mode_t mode = (get_codereq(clientrq->header) == CREQ_MODE4) ? MODE4 : TEAMS;
    uint16_t portudp, portmulticast;
    char multicast_addr[16] = {0};
    int player_id = add_client(server_games, mode, &portudp, &portmulticast, multicast_addr);
    if(player_id < 0) {
        // TODO : Si on aura le temps on va implémenter des tableaux dynamiques
        log_error("Server is busy :\\ please try later.");
        return 1;
    } else {
        uint8_t team_id = player_id % 2;
        Header_t header = {0};
        uint16_t codereq = (mode == MODE4) ? SREQ_MODE4 : SREQ_TEAMS;
        header = (codereq << (EQ_LEN + ID_LEN) | (player_id << EQ_LEN) | (team_id));
        serverrq->type = codereq;
        serverrq->req.start.header = header;
        serverrq->req.start.portudp = portudp;
        serverrq->req.start.portmdiff = portmulticast;
        memcpy(serverrq->req.start.adrmdiff, multicast_addr, strlen(multicast_addr) * sizeof(char));
        return 0;
    }
}

int create_multicastrq(SReq *serverrq, GameBoard gameboard, uint16_t num) {
    serverrq->type = SDIFF_GRID;
    serverrq->req.grid.header = (SDIFF_GRID << (ID_LEN + EQ_LEN) | 0);
    serverrq->req.grid.hauteur = gameboard.height;
    serverrq->req.grid.largeur  = gameboard.width;
    serverrq->req.grid.num = num;
    for(size_t i = 0; i < gameboard.height; ++i) {
        for(size_t j = 0; j < gameboard.width; ++j) {
            serverrq->req.grid.cells[i][j] = gameboard.cells[i][j];
        }
    }

    return 0;
}

int create_cellrq(SReq *serverrq, GameBoard prev_board, GameBoard board, uint16_t num, u_int16_t nb_cases) {
    serverrq->type = SDIFF_CASES;
    serverrq->req.cell.header = (SDIFF_CASES << (EQ_LEN + ID_LEN) | 0);
    serverrq->req.cell.num = num;
    serverrq->req.cell.nb = nb_cases;

    int index = 0;
    for (size_t i = 0; i < board.height; ++i) {
        for (size_t j = 0; j < board.width; ++j) {
            if (prev_board.cells[i][j] != board.cells[i][j]) {
                serverrq->req.cell.cells[index].coord.row = i;
                serverrq->req.cell.cells[index].coord.col = j;
                serverrq->req.cell.cells[index].content = board.cells[i][j];
                index++;
            }
        }
    }

    return 0;
}