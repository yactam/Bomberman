#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <ncurses.h>
#include "debug.h"
#include "client/client_requests.h"
#include "client/network.h"
#include "client/client_utils.h"
#include "game.h"

static char* SERVER_NAME = "::1";
static char* TCP_PORT = "8888";
#define TIMEOUT -1

void print_usage(const char *prog_name);

int main(int argc, char** argv) {
    int opt;

    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
            case 'h':
                SERVER_NAME = optarg;
                break;
            case 'p':
                TCP_PORT = optarg;
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int tcp_socket = init_client(TCP_PORT, SERVER_NAME);

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
    if(recv_server_request(tcp_socket, &start_rq) < 0) {
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

    UDP_Infos *udp_infos = init_udp_connection(SERVER_NAME, port_udp); // pour envoyer les actions au serveur
    if (!udp_infos) {
        perror("Erreur while initialising the udp connection with the client");
        exit(EXIT_FAILURE);
    }

    int sock_udp = udp_infos->sock_udp;
    struct sockaddr_in6 server_addr = udp_infos->server_addr;
    free(udp_infos);

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
    Message tchat = {0};
    uint16_t num = 0;
    bool game_initilized = false;

    struct pollfd fds[3];
    fds[0].fd = sock_multicast;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    fds[2].fd = tcp_socket;
    fds[2].events = POLLIN;
    int ret_poll = 0;
    int pressed_key = 0;

    while(1) {
        ret_poll = poll(fds, 3, TIMEOUT);
        if(ret_poll == -1) {
            perror("Erreur lors de l'appel à poll");
            break;
        } else if(ret_poll > 0) {
            if(fds[0].revents & POLLIN) {
                SReq server_rq = {0};
                if(recv_server_datagram(sock_multicast, &server_rq, sizeof(server_rq.req.grid))) {
                    perror("Erreur recv client grid request");
                }

                debug_sreq(&server_rq);

                if(server_rq.type == SDIFF_GRID) {
                    log_info("IT A MULTICAST OF ALL THE GRID");
                    board.height = server_rq.req.grid.hauteur;
                    board.width = server_rq.req.grid.largeur;

                    if(!game_initilized) {
                        init_game(board.height, board.width);
                        game_initilized = true;
                    }

                    
                    for(size_t i = 0; i < board.height; ++i) {
                        for(size_t j = 0; j < board.width; ++j) {
                            board.cells[i][j] = server_rq.req.grid.cells[i][j];
                        }
                    }
                } else if(server_rq.type == SDIFF_CASES) {
                    log_info("IT A MULTICAST OF SOME CELLS");
                    for(size_t i = 0; i< server_rq.req.cell.nb; ++i) {
                        uint8_t x = server_rq.req.cell.cells[i].coord.row;
                        uint8_t y = server_rq.req.cell.cells[i].coord.col;
                        uint8_t content = server_rq.req.cell.cells[i].content;
                        board.cells[x][y] = content;
                    }
                } else {
                    log_error("%d", server_rq.type);
                }
            } else if(fds[1].revents & POLLIN) {
                pressed_key = getch();
                CReq client_req = {0};
                debug("Le client a tapé sur la touche %d\n", pressed_key);
                
                switch(pressed_key) {
                    case KEY_UP:
                        create_ongamerq(&client_req, gametype, id_player, id_team, num, GO_NORTH);
                        break;
                    case KEY_DOWN:
                        create_ongamerq(&client_req, gametype, id_player, id_team, num, GO_SOUTH);
                        break;
                    case KEY_RIGHT:
                        create_ongamerq(&client_req, gametype, id_player, id_team, num, GO_OUEST);
                        break;
                    case KEY_LEFT:
                        create_ongamerq(&client_req, gametype, id_player, id_team, num, GO_EAST);
                        break;
                    case '!':
                        create_ongamerq(&client_req, gametype, id_player, id_team, num, DROP_BOMB);
                        break;
                    case '$':
                        create_ongamerq(&client_req, gametype, id_player, id_team, num, UNDO);
                        break;
                    case KEY_BACKSPACE:
                        if(tchat.length > 0) {
                            tchat.data[tchat.length - 1] = 0;
                            tchat.length--;
                        }
                        break;
                    case '*':
                        if(tchat.length > 0) {
                            create_chatrq(&client_req, id_player, id_team, &tchat, CALL_CHAT);
                            clearMessage(&tchat);
                        }
                        break;
                    case '#':
                        if(tchat.length > 0) {
                            create_chatrq(&client_req, id_player, id_team, &tchat, gametype == MODE4 ? CALL_CHAT : CCOP_CHAT);
                            clearMessage(&tchat);
                        }
                        break;
                    default:
                        if(tchat.is_sent) {
                            clearMessage(&tchat);
                            tchat.is_sent = false;
                        }
                        if(tchat.length == 0) {
                            tchat.data[tchat.length++] = '>';
                        }
                        if(tchat.length <= MAX_MESS_LENGTH + 1) {
                            tchat.data[tchat.length++] = pressed_key;
                        }
                        break;
                }
                num = (num + 1) % (1 << CNUM_LEN);

                if(client_req.type != 0) {
                    debug_creq(&client_req);
                    if(client_req.type == CALL_CHAT || client_req.type == CCOP_CHAT) {
                        debug("Envoyer une requete TCHAT");
                        send_client_request(tcp_socket, &client_req);
                    } else {
                        send_datagram(sock_udp, server_addr, &client_req);
                    }
                }
                
            } else if(fds[2].revents & POLLIN) {
                debug("Il ya des données tcp à recevoir");
                SReq server_rq = {0};
                if(recv_server_request(tcp_socket, &server_rq) < 0) {
                    perror("Erreur recv client start request");
                    close(tcp_socket);
                    exit(EXIT_FAILURE);
                }
                debug_sreq(&server_rq);

                if(server_rq.type == SALL_CHAT || server_rq.type == SCOP_CHAT) {
                    debug("Une requete TCHAT est reçue");
                    tchat.is_sent = true;
                    uint8_t len = server_rq.req.tchat.len;

                    char data[len + 1];
                    memset(data, 0, (len + 1) * sizeof(char));
                    memcpy(data, server_rq.req.tchat.data, len * sizeof(char));

                    uint8_t auteur = get_id(server_rq.req.tchat.header) + 1;
                    char affichage[len + 4];
                    memset(affichage, 0, sizeof(affichage));
                    sprintf(affichage, "%d: %s", auteur, data);

                    memcpy(tchat.data, affichage, (len + 3) * sizeof(char));
                    tchat.length = len + 3;
                    log_info("LE MESSAGE À AFFICHER EST: %d:%s", tchat.length, affichage);
                } else if(server_rq.type == SGAMEOVER_MODE4 || server_rq.type == SGAMEOVER_TEAMS) {
                    debug("Une requete GAMEOVER est reçue");
                    reqtype_t type = server_rq.type;
                    uint8_t winner = (type == SGAMEOVER_MODE4) ? get_id(server_rq.req.end.header) : get_eq(server_rq.req.end.header);
                    endwin();
                    if(type == SGAMEOVER_MODE4) {
                        printf("Le joueur %d a gagné\n", winner + 1);
                    } else {
                        printf("L'équipe %d a gagné\n", winner + 1);
                    }
                    close(tcp_socket);
                    close(sock_multicast);
                    exit(EXIT_SUCCESS);
                }
            }

            draw_board(board);
            draw_tchat(tchat);
        }
        
        
    }

    endwin();
    return 0;
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [-h hostname] [-p tcp_port]\n", prog_name);
    printf("    -h hostname            : The server hostname (default: ::1)\n");
    printf("    -p tcp_port            : The tcp port where the server is listening (default: 8888)\n");
}