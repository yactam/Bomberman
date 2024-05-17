#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include "server/network.h"
#include "server/server_requests.h"
#include "server/server_utils.h"
#include "debug.h"
#include "socket_utils.h"
#include "data_structures.h"

#define TIMEOUT 1000
#define CLIENT_TIMEOUT 10000

int main(int argc, char** argv) {
    pthread_t games_supervisor;
    ServerGames *server_games;
    int tcp_socket = init_server(TCP_PORT, &server_games);

    if(tcp_socket < 0) {
        exit(EXIT_FAILURE);
    }

   int flags = set_non_blocking(tcp_socket);

    if(flags < 0) {
        perror("Setting non-blocking failed");
        close(tcp_socket);
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
            debug("Il n'y a pas eu d'action depuis une seconde");

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
                    if(recv_client_request(client_tcp_socket, &tcp_rq)) {
                        perror("Erreur recv join request");
                        close(client_tcp_socket);
                        continue;
                    }
                    

                    debug("TCP request received");
                    debug_creq(&tcp_rq);
                    uint16_t codereq = get_codereq(tcp_rq.req.join.header);

                    if(codereq == CREQ_MODE4 || codereq == CREQ_TEAMS) {
                        SReq start_rq = {0};
                        if(create_regestartionrq(&server_games, &start_rq, &tcp_rq.req.join)) {
                            perror("Mode inconnue");
                            continue;
                        }

                        debug_sreq(&start_rq);

                        if(send_server_request(client_tcp_socket, &start_rq)) {
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
                        //send_server_request(client_tcp_socket, &tcp_rq);
                        log_info("REQUEST TCHAT REÇUE");
                        log_info("Message = %s", tcp_rq.req.tchat.data);
                        // TODO le tchat sinon c'est une erreur refuser le client
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