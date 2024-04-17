#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include "games_handler.h"

#define TCP_PORT 8888
#define TIMEOUT_MS 10000

int init_server(uint16_t port, ServerGames**);
void* multicast_grid(void* args);
void* recv_datas(void* args);
void* multicast_updates(void* args);
int close_server(int sockfd, ServerGames**);

#endif