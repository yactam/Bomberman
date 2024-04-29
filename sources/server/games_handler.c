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

int add_client(ServerGames **sg, game_mode_t mode, uint16_t* port_udp, uint16_t* port_multicast, char* addr_mdiff) {
    ServerGames *server_games = *sg;
    if (server_games == NULL) {
        log_error("server_games est NULL");
        return 1;
    }

    debug("Lock server_games mutex");
    pthread_mutex_lock(&(server_games->sgames_mtx));

    for(size_t i = 0; i < server_games->nb_games; ++i) {
        if(server_games->games[i].game_mode == mode && server_games->games[i].nb_players < NB_PLAYERS) {
            debug("Lock server_games->games[%d] mutex", i);
            pthread_mutex_lock(&server_games->games[i].game_mtx);
            pthread_mutex_unlock(&server_games->sgames_mtx);
            debug("Unlocked server_games mutex");
            int player_id;
            for(size_t j = 0; j < NB_PLAYERS; ++j) {
                if(server_games->games[i].players[j].player_status == DISCONNECTED) {
                    player_id = j;
                    server_games->games[i].players[j].player_status = CONNECTING;
                    break;
                }
            }
            server_games->games[i].players[player_id].player_id = player_id;
            server_games->games[i].players[player_id].game_id = i;
            server_games->games[i].nb_players++;
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

    if(server_games->nb_games < MAX_GAMES) {
        int game_index = server_games->nb_games;
        server_games->games[game_index].game_id = game_index;
        server_games->games[game_index].game_mode = mode;
        server_games->games[game_index].nb_players = 0;
        pthread_mutex_init(&server_games->games[game_index].game_mtx, NULL);
        debug("Lock server_games->games[%d] mutex", game_index);
        pthread_mutex_lock(&server_games->games[game_index].game_mtx);
        *port_udp = generate_udpPort();
        *port_multicast = generate_multicastPort();
        generate_multicast_addr(addr_mdiff);
        server_games->games[game_index].port_udp = *port_udp;
        server_games->games[game_index].port_multicast = *port_multicast;
        strcpy(server_games->games[game_index].addr_mdiff, addr_mdiff);
         server_games->nb_games++;
        pthread_mutex_unlock(&server_games->sgames_mtx);
        debug("Unlocked server_games mutex");

        int sock = socket(PF_INET6, SOCK_DGRAM, 0);
        if(sock < 0) {
            perror("Erreur lors de la création de la socket de multidiffusion du serveur");
            pthread_exit(NULL);
        }

        struct sockaddr_in6 gradr = {0};
        gradr.sin6_family = AF_INET6;
        gradr.sin6_port = htons(*port_multicast);
        inet_pton(AF_INET6, multicast_addr, &gradr.sin6_addr);
        gradr.sin6_scope_id = 0; // L'interface par défaut qui permet le multicast

        server_games->games[game_index].multicast_sock = sock;
        server_games->games[game_index].gradr = gradr;
        
        int player_index = server_games->games[game_index].nb_players;
        server_games->games[game_index].players[player_index].player_id = player_index;
        server_games->games[game_index].players[player_index].player_status = CONNECTING;
        server_games->games[game_index].players[player_index].game_id = game_index;
        server_games->games[game_index].nb_players++;
        pthread_mutex_unlock(&server_games->games[game_index].game_mtx);
        debug("Unlocked server_games->games[%d] mutex", game_index);
        debug("Start a new game with id %d and player_id is: %d", game_index, player_index);
        return player_index;
    }

    pthread_mutex_unlock(&server_games->sgames_mtx);
    return -1;
}

char *generate_multicast_addr(char *addr) {
    struct sockaddr_in6 addr_mdiff = {0};
    char addr_mdiff_str[INET6_ADDRSTRLEN];

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
            if (game.players[j].player_id == id_player && game.game_id == portudp - PORTUDP) {
                set_player_status(sg, game.port_udp, id_player, DISCONNECTED);
                game.nb_players--;
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
            pthread_mutex_unlock(&server_games->games[i].game_mtx);

            if(server_games->games[i].game_status == READY_TO_START) {
                debug("Le serveur est prêt à lancer la partie %d", server_games->games[i].game_id);
                debug("Lancement de la partie %d", i);
                init_board("sources/maps/map1.txt", &server_games->games[i].game_board);
                debug_board(server_games->games[i].game_board);
                server_games->games[i].game_status = ON_GOING;
                pthread_create(&server_games->games[i].multicast_thread, NULL, multicast_grid, (void*) &server_games->games[i]);
            }
        }
        sleep(10);
    }
    return NULL;
}

