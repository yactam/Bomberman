#ifndef __GAMES_HANDLER_H__
#define __GAMES_HANDLER_H__

#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "game.h"

#define MAX_GAMES 100
#define NB_PLAYERS 4
#define MULTICAST_ADDR "ff12::1"

typedef enum {
    WAITING_CONNECTIONS = 0,
    READY_TO_START,
    ON_GOING,
    GAME_OVER
} game_status_t;

typedef struct {
    uint8_t player_id;
    uint8_t game_id;
    player_status_t player_status;
} Player;

typedef struct {
    int game_id;
    game_mode_t game_mode;
    game_status_t game_status;
    int nb_players;
    int nb_ready_players;
    Player players[NB_PLAYERS];
    pthread_t game_thread;
    pthread_mutex_t game_mtx;
    pthread_cond_t game_cond;
    uint16_t port_udp;
    uint16_t port_multicast;
    unsigned char addr_mdiff[16];
    GameBoard game_board;
    int multicast_sock;
    struct sockaddr_in6 gradr;
    int clients_tcp_sockets[NB_PLAYERS];
} Game;

typedef struct {
    int nb_games;
    Game games[MAX_GAMES];
    pthread_mutex_t sgames_mtx;
} ServerGames;



static unsigned char multicast_addr[16] = MULTICAST_ADDR;
static uint16_t PORTUDP = 12345;
static uint16_t PORTMULTICAST = 54321; 

void init_serverGames(ServerGames**);
void free_serverGames(ServerGames**);
int add_client(ServerGames**, game_mode_t, uint16_t*, uint16_t*, char*, int);
int remove_client(ServerGames**, uint16_t portudp, uint8_t id_player);
char *generate_multicast_addr();
uint16_t generate_udpPort();
uint16_t generate_multicastPort();
void set_player_status(ServerGames**, uint16_t, uint8_t, player_status_t);
void* games_supervisor_handler(void* arg);
size_t compare_boards(GameBoard board1, GameBoard board2);
void init_players_positions(GameBoard board, player_pos_t *player_positions);
int process_players_actions(PlayerAction *actions, size_t nb_actions, player_pos_t *player_positions, BombInfo *bomb_infos, Game *game);
int handle_explosion(BombInfo *bomb_infos,size_t bomb_pos, Game * game, player_pos_t *player_positions);
int check_game_over(Game *game);

#endif