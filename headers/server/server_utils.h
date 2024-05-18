#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include "games_handler.h"
#include "data_structures.h"

#define TCP_PORT 8888
#define TIMEOUT_MS 10000

int init_server(uint16_t port, ServerGames**);
void* recv_datas(void* args);
void* launch_game(void* args);
int get_tcp_sockets(Array *ci, uint16_t game_udp_port, int *res, uint16_t codereq, uint8_t id_team);
int close_server(int sockfd, ServerGames**);

#endif