#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "requests.h"
#include "debug.h"
#include "server/games_handler.h"
#include "server/server_utils.h"
#include "game.h"

void init_serverGames(ServerGames **server_games) {
    *server_games = (ServerGames*)calloc(1, sizeof(ServerGames));
    
    if (server_games == NULL) {
        perror("Erreur lors de l'allocation de la mémoire pour ServerGames");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&((*server_games)->sgames_mtx), NULL);
}

void free_serverGames(ServerGames **server_games) {
    ServerGames *sg = *server_games;
    pthread_mutex_destroy(&sg->sgames_mtx);
    for(size_t i = 0; i < sg->nb_games; ++i) {
        pthread_mutex_destroy(&sg->games[i].game_mtx);
    }
    free(sg);
}

int add_player_to_game(Game *game, int tcp_sock) {
    int player_id = -1;
    for (size_t j = 0; j < NB_PLAYERS; ++j) {
        if (game->players[j].player_status == DISCONNECTED) {
            player_id = j;
            game->players[j].player_status = CONNECTING;
            game->players[j].player_id = player_id;
            game->players[j].game_id = game->game_id;
            game->clients_tcp_sockets[game->nb_players] = tcp_sock;
            game->nb_players++;
            break;
        }
    }
    return player_id;
}

void initialize_new_game(Game *game, game_mode_t mode, uint16_t *port_udp, uint16_t *port_multicast, char *addr_mdiff, int tcp_sock) {
    game->game_mode = mode;
    game->nb_players = 0;
    pthread_mutex_init(&game->game_mtx, NULL);
    *port_udp = generate_udpPort();
    *port_multicast = generate_multicastPort();
    generate_multicast_addr(addr_mdiff);
    game->port_udp = *port_udp;
    game->port_multicast = *port_multicast;
    strcpy(game->addr_mdiff, addr_mdiff);

    int sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création de la socket de multidiffusion du serveur");
        pthread_exit(NULL);
    }
    struct sockaddr_in6 gradr = {0};
    gradr.sin6_family = AF_INET6;
    gradr.sin6_port = htons(*port_multicast);
    inet_pton(AF_INET6, multicast_addr, &gradr.sin6_addr);
    gradr.sin6_scope_id = 0;

    game->multicast_sock = sock;
    game->gradr = gradr;

    add_player_to_game(game, tcp_sock);
}

int add_client(ServerGames **sg, game_mode_t mode, uint16_t* port_udp, uint16_t* port_multicast, char* addr_mdiff, int tcp_sock) {
    ServerGames *server_games = *sg;
    if (server_games == NULL) {
        log_error("server_games est NULL");
        return 1;
    }

    debug("Lock server_games mutex");
    pthread_mutex_lock(&(server_games->sgames_mtx));

    for (size_t i = 0; i < server_games->nb_games; ++i) {
        if (server_games->games[i].game_mode == mode && server_games->games[i].nb_players < NB_PLAYERS) {
            debug("Lock server_games->games[%d] mutex", i);
            pthread_mutex_lock(&server_games->games[i].game_mtx);
            pthread_mutex_unlock(&server_games->sgames_mtx);
            debug("Unlocked server_games mutex");

            int player_id = add_player_to_game(&server_games->games[i], tcp_sock);
            if (player_id != -1) {
                *port_udp = server_games->games[i].port_udp;
                *port_multicast = server_games->games[i].port_multicast;
                strcpy(addr_mdiff, server_games->games[i].addr_mdiff);
                pthread_mutex_unlock(&server_games->games[i].game_mtx);
                debug("Unlocked server_games->games[%d] mutex", i);
                debug("Add player to game %d with player_id %d", server_games->games[i].game_id, player_id);
                debug("The number of players in the game %d is %d", i, server_games->games[i].nb_players);
                return player_id;
            }
        }
    }

    if (server_games->nb_games < MAX_GAMES) {
        int game_index = server_games->nb_games;
        server_games->games[game_index].game_id = game_index;
        initialize_new_game(&server_games->games[game_index], mode, port_udp, port_multicast, addr_mdiff, tcp_sock);
        server_games->nb_games++;
        pthread_mutex_unlock(&server_games->sgames_mtx);
        debug("Unlocked server_games mutex");

        debug("Start a new game with id %d and player_id is: %d", game_index, 0);
        return 0;
    }

    pthread_mutex_unlock(&server_games->sgames_mtx);
    return -1;
}

char *generate_multicast_addr(char *addr) {
    struct sockaddr_in6 addr_mdiff = {0};

    memset(addr, 0, sizeof(addr));

    inet_pton(AF_INET6, multicast_addr, &addr_mdiff.sin6_addr);
    for(int i = 15; i >= 0; --i) {
        if(++addr_mdiff.sin6_addr.s6_addr[i] != 0) break;
    }

    inet_ntop(AF_INET6, &addr_mdiff.sin6_addr, addr, INET6_ADDRSTRLEN);
    strcpy(multicast_addr, addr);
    debug("Generated multicast address: %s", addr);
    debug("La valeur de multicast_addr %s", multicast_addr);
    return addr;
}

uint16_t generate_udpPort() {
    return PORTUDP++;
}

uint16_t generate_multicastPort() {
    return PORTMULTICAST++;
}

int remove_client(ServerGames **sg, uint16_t portudp, uint8_t id_player) {
    debug("Call to remove client %d from game with udp port %d", id_player, portudp);
    ServerGames *server_games = *sg;
    if (server_games == NULL) {
        log_error("server_games est NULL");
        return 1;
    }

    debug("Lock server_games mutex");
    pthread_mutex_lock(&(server_games->sgames_mtx));

    for (size_t i = 0; i < server_games->nb_games; ++i) {
        Game game = server_games->games[i];
        for (size_t j = 0; j < game.nb_players; ++j) {
            debug("Lock server_games->games[%d] mutex", i);
            pthread_mutex_lock(&game.game_mtx);
            if (game.players[j].player_id == id_player && game.game_id == portudp - PORTUDP+1) {
                set_player_status(sg, game.port_udp, id_player, DISCONNECTED);
                game.nb_players--;
                debug("Client removed");
                pthread_mutex_unlock(&game.game_mtx);
                debug("Unlocked server_games->games[%d] mutex", i);
                pthread_mutex_unlock(&(server_games->sgames_mtx));
                debug("Unlocked server_games mutex");
                return 0;
            }
            pthread_mutex_unlock(&game.game_mtx);
            debug("Unlocked server_games->games[%d] mutex", i);
        }
    }

    pthread_mutex_unlock(&(server_games->sgames_mtx));
    debug("Unlocked server_games mutex");

    return 1;
}

ssize_t get_game_id(ServerGames **sg, uint16_t game_udp_port) {
    ServerGames *server_games = *sg;
    size_t game_id = game_udp_port - server_games->games[0].port_udp;
    if(server_games->games[game_id].port_udp == game_udp_port) {
        debug("The game with udp port %d was found at index %d", game_udp_port, game_id);
        return game_id;
    }
    log_error("The game with udp port %d was not found");
    return -1;
}

void set_player_status(ServerGames **sg, uint16_t  game_udp_port, uint8_t player_id, player_status_t status) {
    ServerGames *server_games = *sg;
    ssize_t game_id = get_game_id(&server_games, game_udp_port);
    if(game_id != -1) {
        pthread_mutex_lock(&server_games->games[game_id].game_mtx);
        if(status != server_games->games[game_id].players[player_id].player_status) {
            if(server_games->games[game_id].players[player_id].player_status == READY) {
                server_games->games[game_id].nb_ready_players -= 1;
            }
            server_games->games[game_id].players[player_id].player_status = status;
            if(status == READY) {
                server_games->games[game_id].nb_ready_players += 1;
            }
            debug("Le nombre de joueurs prêts dans la partie %d est %d", game_id, server_games->games[game_id].nb_ready_players);
            if (server_games->games[game_id].nb_ready_players == NB_PLAYERS) {
                server_games->games[game_id].game_status = READY_TO_START;
                pthread_cond_signal(&server_games->games[game_id].game_cond);
            }
        }
        pthread_mutex_unlock(&server_games->games[game_id].game_mtx);
        debug("Set player status to %d was successful", status);
    } else {
        log_error("The game was not found to set the player's ready state");
    }
}

void* games_supervisor_handler(void* arg) {
    ServerGames* server_games = *((ServerGames**)arg);

    while (1) {
        for (int i = 0; i < server_games->nb_games; ++i) {
            pthread_mutex_lock(&server_games->games[i].game_mtx);
            while (server_games->games[i].game_status == WAITING_CONNECTIONS) {
                pthread_cond_wait(&server_games->games[i].game_cond, &server_games->games[i].game_mtx);
            }

            if(server_games->games[i].game_status == READY_TO_START) {
                debug("Le serveur est prêt à lancer la partie %d", server_games->games[i].game_id);
                debug("Lancement de la partie %d", i);
                init_board("sources/maps/map1.txt", &server_games->games[i].game_board);
                debug_board(server_games->games[i].game_board);
                server_games->games[i].game_status = ON_GOING;
                pthread_create(&server_games->games[i].game_thread, NULL, launch_game, (void*) &server_games->games[i]);
            } else if(server_games->games[i].game_status == GAME_OVER) {
                pthread_mutex_unlock(&server_games->games[i].game_mtx);
                debug("La partie %d est terminée", server_games->games[i].game_id);
                pthread_join(server_games->games[i].game_thread, NULL);
                pthread_cond_destroy(&server_games->games[i].game_cond);
                pthread_mutex_destroy(&server_games->games[i].game_mtx);
                memset(&server_games->games[i], 0, sizeof(Game));
                server_games->nb_games--;
                continue;
            }

            pthread_mutex_unlock(&server_games->games[i].game_mtx);
        }
        sleep(10);
    }
    return NULL;
}

size_t compare_boards(GameBoard board1, GameBoard board2) {
    if (board1.height != board2.height || board1.width != board2.width) {
        return 0;
    }

    size_t res = 0;

    for (size_t i = 0; i < board1.height; ++i) {
        for (size_t j = 0; j < board1.width; ++j) {
            if (board1.cells[i][j] != board2.cells[i][j]) {
                res++;
            }
        }
    }

    return res;
}

void init_players_positions(GameBoard board, player_pos_t *player_positions) {
    for(size_t player = 0; player < NB_PLAYERS; ++player) {
        for (size_t i = 0; i < board.height; ++i) {
            for (size_t j = 0; j < board.width; ++j) {
                if (board.cells[i][j] == player + PLAYER1) {
                    player_positions[player].x = i;
                    player_positions[player].y = j;
                    break;
                }
            }
        }
    }
}

void process_players_actions(PlayerAction *actions, size_t nb_actions, player_pos_t *player_positions, BombInfo *bomb_infos, Game *game) {
    debug("Appel à la fonction process players action avec nb_actions=%d", nb_actions);
    bool new_bomb_drop[NB_PLAYERS] = {false};
    PlayerAction last_move_action[NB_PLAYERS] = {0};
    PlayerAction bomb_action[NB_PLAYERS] = {0};
    int max_move_num[NB_PLAYERS];
    bool undo_move[NB_PLAYERS] = {false};

    GameBoard *board = &game->game_board;

    if(nb_actions > 0) {
        for(size_t i = 0; i < NB_PLAYERS; ++i) {
            max_move_num[i] = -1;
        }

        uint8_t player_id;

        for (size_t i = 0; i < nb_actions; ++i) {
            PlayerAction action = actions[i];
            player_id = action.paleyr_id;

            log_info("player_id = %d", player_id);

            switch (action.action) {
                case GO_NORTH:
                case GO_SOUTH:
                case GO_EAST:
                case GO_OUEST:
                    if ((action.num > max_move_num[player_id]) || (max_move_num[player_id] == 8191 && max_move_num[player_id] < 1000)) {
                        last_move_action[player_id] = action;
                        max_move_num[player_id] = action.num;
                        debug("L'action est un mouvement");
                    }
                    break;
                case DROP_BOMB:
                    if (!bomb_infos[player_id].bomb_dropped) {
                        bomb_action[player_id] = action;
                        new_bomb_drop[player_id] = true;
                        debug("L'action est un placement de bomb");
                    }
                    break;
                case UNDO:
                    undo_move[player_id] = true;
                    break;
                default:
                    debug("Type d'action non reconnu %d.\n", action.action);
                    break;
            }
        }

        pthread_mutex_lock(&game->game_mtx);

        for(size_t player = 0; player < NB_PLAYERS; ++player) {
            if (max_move_num[player] != -1 && game->players[player].player_status == PLAYING && !undo_move[player]) {
                size_t player_x = player_positions[player].x;
                size_t player_y = player_positions[player].y;

                debug("The player is at position %d %d", player_x, player_y);

                int desti = player_x;
                int destj = player_y;

                switch(last_move_action[player].action) {
                    case GO_NORTH:
                        desti -= 1;
                        break;

                    case GO_SOUTH:
                        desti += 1;
                        break;

                    case GO_EAST:
                        destj -= 1;
                        break;

                    case GO_OUEST:
                        destj += 1;
                        break;
                }

                if((desti >= 0 && desti < board->height && destj >= 0 && destj < board->width) && (board->cells[desti][destj] == EMPTY || board->cells[desti][destj] == BOMB)) {
                    if(board->cells[player_x][player_y] != BOMB)
                        board->cells[player_x][player_y] = EMPTY;
                    if(board->cells[desti][destj] == EMPTY)
                        board->cells[desti][destj] = player + PLAYER1;
                    player_positions[player].x = desti;
                    player_positions[player].y = destj;
                }
            }

            if (new_bomb_drop[player] && game->players[player].player_status == PLAYING) {
                int i = player_positions[player].x;
                int j = player_positions[player].y;
                debug("DROP BOMB AT %d %d", i, j);
                board->cells[i][j] = BOMB;
                bomb_infos[player_id].bomb_dropped = true;
                bomb_infos[player_id].bomb_x = i;
                bomb_infos[player_id].bomb_y = j;
                bomb_infos[player_id].explosion_time = time(NULL) + EXPLOSION_DELAY;
            }
        }
    }

    for(size_t i = 0; i < NB_PLAYERS; ++i) {
        if(bomb_infos[i].bomb_dropped && bomb_infos[i].explosion_time <= time(NULL)) {
            handle_explosion(bomb_infos, i, game, player_positions);
            bomb_infos[i].bomb_dropped = false;
        }
    }
    debug_board(*board);
    pthread_mutex_unlock(&game->game_mtx);
}


void handle_explosion(BombInfo *bomb_infos, size_t bomb_pos, Game * game, player_pos_t *player_positions) {
    GameBoard* board = &(game->game_board);
    BombInfo bomb_info = bomb_infos[bomb_pos];
    int i = bomb_info.bomb_x;
    int j = bomb_info.bomb_y;
    board->cells[i][j] = EXPLOSE;

    for (int dx = 0; dx <= 2; dx+=1) {
        int x = i + dx;

        debug("EXPLOSION: test %d %d", x, j);

        if (x >= 0 && x < board->height) {
            if (board->cells[x][j] == IWALL) {
                break;
            }

            if (board->cells[x][j] == DWALL) {
                board->cells[x][j] = EXPLOSE;
                break;
            }
            if (board->cells[x][j] == BOMB) {
                debug("recursive explosion");
                for(size_t c = 0; c < NB_PLAYERS; c += 1){
                    if(bomb_infos[c].bomb_dropped && bomb_infos[c].bomb_x == x && bomb_infos[c].bomb_y == j){
                        handle_explosion(bomb_infos, c, game, player_positions);
                        bomb_infos[c].bomb_dropped = false;
                    }
                }
            }
            if(board->cells[x][j] == EMPTY) {
                board->cells[x][j] = EXPLOSE;
            }

            for(size_t player = 0; player < NB_PLAYERS; player++) {
                if(player_positions[player].x == x && player_positions[player].y == j) {
                    board->cells[x][j] = EXPLOSE;
                    game->players[player].player_status = DEAD;
                }
            } 
        }
    }

    for (int dx = -1; dx >= -2; dx--) {
        int x = i + dx;

        debug("EXPLOSION: test %d %d", x, j);

        if (x >= 0 && x < board->height) {
            if (board->cells[x][j] == IWALL) {
                break;
            }

            if (board->cells[x][j] == DWALL) {
                board->cells[x][j] = EXPLOSE;
                break;
            }
            if (board->cells[x][j] == BOMB){
                debug("recursive explosion");
                for(size_t c = 0; c < NB_PLAYERS; c += 1){
                    if(bomb_infos[c].bomb_dropped && bomb_infos[c].bomb_x == x && bomb_infos[c].bomb_y == j){
                        handle_explosion(bomb_infos, c, game, player_positions);
                        bomb_infos[c].bomb_dropped = false;
                    }
                }
            }
            if(board->cells[x][j] == EMPTY) {
                board->cells[x][j] = EXPLOSE;
            }

            for(size_t player = 0; player < NB_PLAYERS; player++) {
                if(player_positions[player].x == x && player_positions[player].y == j) {
                    game->players[player].player_status = DEAD;
                    board->cells[x][j] = EXPLOSE;
                }
            }
        }
    }

    for(int dy = 1; dy <= 2; dy++) {
        int y = j + dy;

        debug("EXPLOSION: test %d %d", i, y);

        if(y >= 0 && y < board->width) {
            if (board->cells[i][y] == IWALL) {
                break;
            }

            if (board->cells[i][y] == DWALL) {
                board->cells[i][y] = EXPLOSE;
                break;
            }
            if (board->cells[i][y] == BOMB){
                debug("recursive explosion");
                for(size_t c = 0; c < NB_PLAYERS; c += 1){
                    if(bomb_infos[c].bomb_dropped && bomb_infos[c].bomb_x == i && bomb_infos[c].bomb_y == y){
                        handle_explosion(bomb_infos, c, game, player_positions);
                        bomb_infos[c].bomb_dropped = false;
                    }
                }
            }
            if(board->cells[i][y] == EMPTY) {
                board->cells[i][y] = EXPLOSE;
            }

            for(size_t player = 0; player < NB_PLAYERS; player++) {
                if(player_positions[player].x == i && player_positions[player].y == y) {
                    board->cells[i][y] = EXPLOSE;
                    game->players[player].player_status = DEAD;
                }
            }
        }
    }

    for (int dy = -1; dy >= -2; dy--) {
        int y = j + dy;

        debug("EXPLOSION: test %d %d", i, y);

        if(y >= 0 && y < board->width) {
            if (board->cells[i][y] == IWALL) {
                break;
            }

            if (board->cells[i][y] == DWALL) {
                board->cells[i][y] = EXPLOSE;
                break;
            }
            if (board->cells[i][y] == BOMB){
                debug("recursive explosion");
                for(size_t c = 0; c < NB_PLAYERS; c += 1){
                    if(bomb_infos[c].bomb_dropped && bomb_infos[c].bomb_x == i && bomb_infos[c].bomb_y == y){
                        handle_explosion(bomb_infos, c, game, player_positions);
                        bomb_infos[c].bomb_dropped = false;
                    }
                }
            }
            if(board->cells[i][y] == EMPTY) {
                board->cells[i][y] = EXPLOSE;
            }

            for(size_t player = 0; player < NB_PLAYERS; player++) {
                if(player_positions[player].x == i && player_positions[player].y == y) {
                    game->players[player].player_status = DEAD;
                    board->cells[i][y] = EXPLOSE;
                }
            }
        }
    }

    for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {

            if(dx == 0 || dy == 0) continue;

            int x = i + dx;
            int y = j + dy;

            if(x >= 0 && x < board->height && y >= 0 && y < board->width) {

                if (board->cells[x][y] == DWALL) {
                    board->cells[x][y] = EXPLOSE;
                } else if (board->cells[x][y] == BOMB){
                    debug("recursive explosion");
                    for(size_t c = 0; c < NB_PLAYERS; c += 1){
                        if(bomb_infos[c].bomb_dropped && bomb_infos[c].bomb_x == x && bomb_infos[c].bomb_y == y){
                            handle_explosion(bomb_infos, c, game, player_positions);
                            bomb_infos[c].bomb_dropped = false;
                        }
                    }
                } else if(board->cells[x][y] == EMPTY) {
                    board->cells[x][y] = EXPLOSE;
                }

                for(size_t player = 0; player < NB_PLAYERS; player++) {
                    if(player_positions[player].x == x && player_positions[player].y == y) {
                        game->players[player].player_status = DEAD;
                        board->cells[x][y] = EXPLOSE;
                    }
                }
            }
        }
    }
}

int check_game_over(Game *game) {
    game_mode_t mode = game->game_mode;
    if(mode == MODE4) {
        int players_alive = 0;
        int id_winner = 0;
        for (int i = 0; i < NB_PLAYERS; ++i) {
            if (game->players[i].player_status != DEAD) {
                players_alive++;
                id_winner = game->players[i].player_id;
            }
        }
        if(players_alive <= 1) {
            return id_winner;
        }
        return -1;
    } else {
        int team1_dead = 0;
        int team2_dead = 0;

        for (int i = 0; i < NB_PLAYERS; ++i) {
            if (game->players[i].player_status == DEAD) {
                if (game->players[i].player_id % 2 == 0) {
                    team1_dead++;
                } else {
                    team2_dead++;
                }
            }
        }
        if(team2_dead == 2) return 0;
        if(team1_dead == 2) return 1;
        return -1;
    }
}
