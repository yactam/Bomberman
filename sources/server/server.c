#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include "server/network.h"
#include "server/server_requests.h"
#include "server/server_utils.h"
#include "debug.h"
#include "game.h"
#include "socket_utils.h"
#include "data_structures.h"

#define TIMEOUT 1000
#define CLIENT_TIMEOUT 5000

void print_usage(const char *prog_name);

int main(int argc, char** argv) {

    int opt;

    while ((opt = getopt(argc, argv, "p:b:f:m:")) != -1) {
        switch (opt) {
            case 'b':
                EXPLOSION_DELAY = atoi(optarg);
                break;
            case 'f':
                FREQ = atoi(optarg);
                break;
            case 'm':
                ALL_FREQ = atoi(optarg);
                break;
            case 'p':
                TCP_PORT = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    debug("EXPLOSION DELAY = %d, FREQ = %d, TCP_PORT = %d", EXPLOSION_DELAY, FREQ, TCP_PORT);

    pthread_t games_supervisor;
    ServerGames *server_games;
    int tcp_socket = init_server(TCP_PORT, &server_games);

    if(tcp_socket < 0) {
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&games_supervisor, NULL, games_supervisor_handler, (void*)&server_games) != 0) {
        perror("Erreur lors de la création du thread de gestion du démarrage des parties");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 client_tcp;
    socklen_t client_tcp_len = sizeof(client_tcp);

    Array *fds = create_array(sizeof(struct pollfd));
    Array *clients_infos = create_array(sizeof(Client_Infos));

    if(!fds || !clients_infos) {
        exit(EXIT_FAILURE);
    }

    struct pollfd fd = {0};
    fd.fd = tcp_socket;
    fd.events = POLLIN;
    append_to_array(fds, &fd);

    while(1) {
        int ret = poll(fds->data, (nfds_t) fds->size, TIMEOUT);
        if(ret < 0) {
            perror("Error in poll");
            break;
        } else if(ret == 0) {

            time_t current_time = time(NULL);
            for (size_t i = 0; i < clients_infos->size; ++i) {
                Client_Infos* cli_info = get_from_array(clients_infos, i);
                if (difftime(current_time, cli_info->last_activity) >= CLIENT_TIMEOUT && cli_info->status == CONNECTING) {
                    debug("Client %d timed out, deconnexion...", cli_info->client_tcp_sock);
                    close(cli_info->client_tcp_sock);
                    remove_client(&server_games, cli_info->game_udp_port, cli_info->client_id);
                    for(size_t j = 0; j < fds->size; ++j) {
                        struct pollfd *pfd = get_from_array(fds, j);
                        if(pfd->fd == cli_info->client_tcp_sock) {
                            remove_from_array_at(fds, j);
                            break;
                        }
                    }
                    remove_from_array_at(clients_infos, i);
                    i--;
                }
            }

        } else {
            for(size_t i = 0; i < fds->size; ++i) {
                struct pollfd *pfd = get_from_array(fds, i);
                if(!(pfd->revents & POLLIN)) continue;
    
                if(pfd->fd == tcp_socket) {
                    
                    int client_tcp_socket = accept(tcp_socket, (struct sockaddr *)&client_tcp, &client_tcp_len);
                    if (client_tcp_socket == -1) {
                        perror("Error accepting TCP connection");
                        continue;
                    }

                    log_info("Le client c'est bien connecté client_tcp_socket = %d", client_tcp_socket);

                    struct pollfd poll_fd = {0};
                    poll_fd.fd = client_tcp_socket;
                    poll_fd.events = POLLIN;
                    append_to_array(fds, &poll_fd);

                    Client_Infos ci = {0};
                    ci.client_tcp_sock = client_tcp_socket;
                    ci.status = CONNECTING;
                    ci.last_activity = time(NULL);
                    append_to_array(clients_infos, &ci);

                } else {
                    int client_tcp_socket = pfd->fd;
                    Client_Infos *client_infos;

                    for(size_t j = 0; j < clients_infos->size; ++j) {
                        Client_Infos *ci = get_from_array(clients_infos, j);
                        if(ci->client_tcp_sock == client_tcp_socket) {
                            client_infos = ci;
                        }
                    }

                    CReq tcp_rq = {0};
                    ssize_t ret = recv_client_request(client_tcp_socket, &tcp_rq);
                    if(ret < 0) {
                        perror("Erreur recv join request");
                        continue;
                    } else if(ret == 0) {
                        log_info("Le client avec la socket tcp %d s'est déconnecté", client_tcp_socket);
                        close(client_tcp_socket);
                        size_t sock_index = 0;
                        for(size_t i = 0; i < fds->size; ++i) {
                            struct pollfd *pfd = get_from_array(fds, i);
                            if(pfd->fd == client_tcp_socket) {
                                sock_index = i;
                                break;
                            }
                        }
                        remove_from_array_at(fds, sock_index);
                        continue;
                    }
                    

                    debug("TCP request received");
                    debug_creq(&tcp_rq);
                    uint16_t codereq = tcp_rq.type;

                    if(codereq == CREQ_MODE4 || codereq == CREQ_TEAMS) {
                        SReq start_rq = {0};
                        if(create_regestartionrq(&server_games, &start_rq, &tcp_rq.req.join, client_tcp_socket)) {
                            perror("Mode inconnue");
                            continue;
                        }

                        debug_sreq(&start_rq);

                        if(send_server_request(&client_tcp_socket, 1, &start_rq)) {
                            perror("Erreur send request");
                            close(client_tcp_socket);
                            remove_client(&server_games, start_rq.req.start.portudp, get_id(start_rq.req.start.header));
                            continue;
                        }

                        debug("Server start request sent");

                        client_infos->game_udp_port = start_rq.req.start.portudp;
                        client_infos->client_id = get_id(start_rq.req.start.header);
                        client_infos->last_activity = time(NULL);

                    } else if(codereq == CCONF_MODE4 || codereq == CCONF_TEAMS) {
                        debug("Le client a confirmé qu'il veut rejoindre la partie");
                        set_player_status(&server_games, client_infos->game_udp_port, client_infos->client_id, READY);
                    } else if(codereq == CALL_CHAT || codereq == CCOP_CHAT){
                        SReq tchat_rq = {0};
                        create_tchatrq(&tchat_rq, &tcp_rq.req.tchat);
                        debug_sreq(&tchat_rq);

                        int sock_fds[NB_PLAYERS] = {0};
                        int id_team = get_eq(tchat_rq.req.tchat.header);
                        int nb_socks = get_tcp_sockets(clients_infos, client_infos->game_udp_port, sock_fds, codereq, id_team);
                        if(nb_socks < 0) {
                            perror("Erreur get tcp sockets");
                            continue;
                        }

                        if(send_server_request(sock_fds, nb_socks, &tchat_rq)) {
                            perror("Erreur send tchat request");
                            continue;
                        }
                    }
                    else{
                    }
                }
            }
        }
    }

    pthread_join(games_supervisor, NULL);
    close_server(tcp_socket, &server_games);
    free_array(fds);
    free_array(clients_infos);
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [-p port_tcp] [-b bomb_timer] [-m multicast_frequency] [-f request_frequency]\n", prog_name);
    printf("    -p port_tcp             : The tcp_port where the server will listen to connection (default: 8888)\n");
    printf("    -b bomb_timer           : Time between bomb drop and explosion in seconds (default: 3)\n");
    printf("    -f multicast_frequency  : Frequency of multicast of the complete grid in seconds (default: 1)\n");
    printf("    -r request_frequency    : Frequency of request processing and differential multicast in milliseconds (default: 10)\n");
}