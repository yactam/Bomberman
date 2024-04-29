#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "debug.h"
#include "client/client_requests.h"
#include "client/network.h"
#include "client/client_utils.h"
#include "game.h"

#define SERVER_IP "::1"
#define TCP_PORT 8888

int main(int argc, char** argv) {
    int tcp_socket = init_client(TCP_PORT, SERVER_IP);

    if(tcp_socket < 0) {
        perror("Erreur while initialising client connection");
        exit(EXIT_FAILURE);
    }

    CReq join_req = {0};
    create_integrationrq(&join_req);
    debug_creq(&join_req);
    if(send_client_request(tcp_socket, &join_req)) {
        perror("Erreur send client join request");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    debug("Client Join request sent");

    SReq start_rq = {0};
    if(recv_server_request(tcp_socket, &start_rq, sizeof(start_rq.req.start))) {
        perror("Erreur recv client start request");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    debug_sreq(&start_rq);

    game_mode_t gametype = get_codereq(start_rq.req.start.header) == SREQ_MODE4 ? MODE4 : TEAMS;
    uint8_t id_player = get_id(start_rq.req.start.header);
    uint8_t id_team = get_eq(start_rq.req.start.header);

    uint16_t port_multicast = start_rq.req.start.portmdiff;
    uint16_t port_udp = start_rq.req.start.portudp;
    char *ip_multicast = start_rq.req.start.adrmdiff;

    int sock_multicast = subscribe_multicast(port_multicast, ip_multicast); // pour recevoir la grille du jeu
    if (sock_multicast < 0) {
        perror("Erreur subscribe to multicast addresse of the server");
        exit(EXIT_FAILURE);
    }

    UDP_Infos *udp_infos = init_udp_connection(port_udp); // pour envoyer les actions au serveur
    if (!udp_infos) {
        perror("Erreur while initialising the udp connection with the client");
        exit(EXIT_FAILURE);
    }

    CReq conf_rq = {0};
    create_confrq(&conf_rq, gametype, id_player, id_team);
    debug_creq(&conf_rq);

    if(send_client_request(tcp_socket, &conf_rq)) {
        perror("Erreur send client confirmation request");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    printf("Le client a confirmé la connexion au serveur. en attente d'autres joueurs...\n");

    GameBoard board = {0};
    uint16_t num = 0;
    bool game_initilized = false;

    while(1) {
        SReq grid_rq = {0};
        if(recv_server_datagram(sock_multicast, &grid_rq, sizeof(grid_rq.req.grid))) {
            perror("Erreur recv client grid request");
        }

        debug_sreq(&grid_rq);

        if(!game_initilized) {
            init_game();
            game_initilized = true;
        }

        board.height = grid_rq.req.grid.hauteur;
        board.width = grid_rq.req.grid.largeur;
        for(size_t i = 0; i < board.height; ++i) {
            for(size_t j = 0; j < board.width; ++j) {
                board.cells[i][j] = grid_rq.req.grid.cells[i][j];
            }
        }

        draw_board(board);

    }

    endwin();
    return 0;
}