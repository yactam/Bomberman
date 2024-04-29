#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server/server_requests.h"
#include "server/server_utils.h"
#include "server/network.h"
#include "game.h"

#define FREQ 100

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

    int ok = 1;
	if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
		perror("echec de SO_REUSEADDR");
		close(tcp_socket);
		return -1;
	}

    if (bind(tcp_socket, (struct sockaddr *)&server_tcp, sizeof(server_tcp)) == -1) {
        perror("Error binding TCP socket");
        exit(EXIT_FAILURE);
    }

    if (listen(tcp_socket, 0) == -1) {
        perror("Error listening for TCP connections");
        exit(EXIT_FAILURE);
    }

    printf("Le serveur écoute sur le port %d\n", TCP_PORT);
    return tcp_socket;
}

void* multicast_grid(void* args) {
    Game game = *((Game*) args);
    GameBoard board = game.game_board;
    int sock = game.multicast_sock;
    struct sockaddr_in6 gradr = game.gradr;

    uint32_t num = 0;

    while(1) {

        // multidiffuser la grille
        SReq grid_rq = {0};
        pthread_mutex_lock(&game.game_mtx);
        create_multicastrq(&grid_rq, board, num++);
        pthread_mutex_unlock(&game.game_mtx);
        debug_sreq(&grid_rq);
        
        if(send_datagram(sock, gradr, &grid_rq)) {
            perror("Erreur lors de la multidiffusion de la grille du jeu");
            break;
        }

        sleep(1);
    }

    return NULL;
}

void* multicast_updates(void* args) {
    Game game = *((Game*) args);
    GameBoard board = game.game_board;
    int sock = game.multicast_sock;
    struct sockaddr_in6 gradr = game.gradr;

    uint32_t num = 0;

    while(1) {

        // multidiffuser la grille
        SReq grid_rq = {0};
        pthread_mutex_lock(&game.game_mtx);
        create_multicastrq(&grid_rq, board, num++);
        pthread_mutex_unlock(&game.game_mtx);
        debug_sreq(&grid_rq);
        
        if(send_datagram(sock, gradr, &grid_rq)) {
            perror("Erreur lors de la multidiffusion de la grille du jeu");
            break;
        }

        sleep(1);
    }

    return NULL;
}

int close_server(int sockfd, ServerGames **server_games) {
    close(sockfd);
    free_serverGames(server_games);
}