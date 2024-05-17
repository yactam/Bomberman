#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include "server/server_requests.h"
#include "server/server_utils.h"
#include "server/network.h"
#include "socket_utils.h"
#include "game.h"
#include "debug.h"

#define FREQ 10

int init_server(uint16_t port, ServerGames **server_games) {
    int tcp_socket;
    struct sockaddr_in6 server_tcp;
    init_serverGames(server_games);

    tcp_socket = socket(PF_INET6, SOCK_STREAM, 0);
    if (tcp_socket == -1) {
        perror("Error creating TCP socket");
        return -1;
    }

    server_tcp.sin6_family = AF_INET6;
    server_tcp.sin6_addr = in6addr_any;
    server_tcp.sin6_port = htons(TCP_PORT);

    if(set_reuseaddr(tcp_socket)) {
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    if (bind(tcp_socket, (struct sockaddr *)&server_tcp, sizeof(server_tcp)) == -1) {
        perror("Error binding TCP socket");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(tcp_socket, 0) == -1) {
        perror("Error listening for TCP connections");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    printf("Le serveur écoute sur le port %d\n", TCP_PORT);
    return tcp_socket;
}

UDP_Infos *init_server_udp(uint16_t port) {
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("Error init socket udp for server");
        return NULL;
    }

    struct sockaddr_in6 serv_addr = {0};
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(port);
    serv_addr.sin6_addr = in6addr_any;

    if(set_reuseaddr(sockfd)) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (const struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed while init server udp");
        return NULL;
    }

    UDP_Infos *udp_infos = malloc(sizeof(UDP_Infos));
    udp_infos->sock_udp = sockfd;
    udp_infos->server_addr = serv_addr;

    return udp_infos;
}

void* launch_game(void* args) {
    Game game = *((Game*) args);
    GameBoard board = game.game_board;
    int sock_multicast = game.multicast_sock;
    struct sockaddr_in6 gradr = game.gradr;

    uint32_t num = 0;
    UDP_Infos *udp_infos = init_server_udp(game.port_udp);
    if(udp_infos == NULL) {
        perror("Erreur lors de l'initialisation de la connexion udp dans le serveur");
        return NULL;
    }
    void initMessage(Message* msg, u_int8_t id_player, u_int8_t id_team){
        msg = malloc(sizeof(Message));
        msg-> codereq =CODEREQ_ALL_PLAYERS ;
        msg->id = id_player;
        msg->eq = id_team;
        msg->len = 0;
        msg->data = malloc(256);
    }
    int sock_udp = udp_infos->sock_udp;
    struct sockaddr_in6 serv_addr = udp_infos->server_addr;
    free(udp_infos); 

    GameBoard prev_board;
    memcpy(&prev_board, &board, sizeof(GameBoard));

    struct pollfd fds;
    fds.fd = sock_udp;
    fds.events = POLLIN;
    int ret_poll = 0;
    int elapsed_time = 0;

    // Premier muticast de la grille aux joueurs
    SReq grid_rq = {0};
    pthread_mutex_lock(&game.game_mtx);
    create_multicastrq(&grid_rq, board, num++);
    pthread_mutex_unlock(&game.game_mtx);
    debug_sreq(&grid_rq);
    
    if(send_datagram(sock_multicast, gradr, &grid_rq)) {
        perror("Erreur lors de la multidiffusion de la grille du jeu");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < NB_PLAYERS; ++i) {
        game.players[i].player_status = PLAYING;
    }

    PlayerAction players_actions[MAX_ACTIONS] = {0};
    BombInfo bombs_infos[NB_PLAYERS] = {0};
    player_pos_t player_positions[NB_PLAYERS];
    init_players_positions(board, player_positions);
    size_t nb_actions = 0;

    while(1) {
        ret_poll = poll(&fds, 1, FREQ);
        if (ret_poll < 0) {
            perror("poll failed");
            return NULL;
        } else if(ret_poll == 0) {
            // traiter les actions
            int winner = process_players_actions(players_actions, nb_actions, player_positions, bombs_infos, &game);
            board = game.game_board;
            nb_actions = 0;
            memset(players_actions, 0, sizeof(players_actions));
            
            // multicaster les changements
            pthread_mutex_lock(&game.game_mtx);
            size_t nb_cases = compare_boards(prev_board, board);
            pthread_mutex_unlock(&game.game_mtx);
            if(nb_cases != 0) {
                SReq cell_rq = {0};
                pthread_mutex_lock(&game.game_mtx);
                create_cellrq(&cell_rq, prev_board, board, num++, nb_cases);
                pthread_mutex_unlock(&game.game_mtx);
                debug_sreq(&cell_rq);
                if(send_datagram(sock_multicast, gradr, &cell_rq)) {
                    perror("Erreur lors de la multidiffusion du différentiel sur la grille entre le moment présent et la dernière multi-diffusion du jeu");
                    pthread_mutex_unlock(&game.game_mtx);
                    continue;
                }
                pthread_mutex_lock(&game.game_mtx);
                memcpy(&prev_board, &board, sizeof(GameBoard));
                pthread_mutex_unlock(&game.game_mtx);
            }
            pthread_mutex_lock(&game.game_mtx);
            clear_bombs_explosions(&game.game_board);
            pthread_mutex_unlock(&game.game_mtx);
            pthread_mutex_lock(&game.game_mtx);
            //int winner = check_game_over(&game);
            pthread_mutex_unlock(&game.game_mtx);
            if(winner != -1) {
                debug("CHECK GAME OVER THERE IS A WINNER %d", winner);
                game.game_status == GAME_OVER;
                // TODO informer les joueurs du gagnant
            }

            // multicaster toute la grille si nécessaire
            elapsed_time++;
            if(elapsed_time * FREQ >= 1000) {
                SReq grid_rq = {0};
                pthread_mutex_lock(&game.game_mtx);
                create_multicastrq(&grid_rq, board, num++);
                pthread_mutex_unlock(&game.game_mtx);
                debug_sreq(&grid_rq);
                
                if(send_datagram(sock_multicast, gradr, &grid_rq)) {
                    perror("Erreur lors de la multidiffusion de la grille du jeu");
                    break;
                }
                elapsed_time = 0;
            }
        } else {
            // recevoir les actions des clients
            CReq ongame_rq = {0};
            if(recv_client_datagrams(sock_udp, &ongame_rq)) {
                perror("Error while receiving client datagrams");
                continue;
            }

            debug_creq(&ongame_rq);
            PlayerAction player_action;
            player_action.paleyr_id = get_id(ongame_rq.req.play.header);
            player_action.num = get_num(ongame_rq.req.play.message);
            player_action.action = get_action(ongame_rq.req.play.message);

            players_actions[nb_actions++] = player_action;
        }
    }

    return NULL;
}

int close_server(int sockfd, ServerGames **server_games) {
    close(sockfd);
    free_serverGames(server_games);
}