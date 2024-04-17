#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include "server/network.h"
#include "server/server_requests.h"
#include "server/server_utils.h"
#include "debug.h"

int main(int argc, char** argv) {
    pthread_t games_supervisor;
    ServerGames *server_games;
    int tcp_socket = init_server(TCP_PORT, &server_games);

    if(tcp_socket < 0) {
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&games_supervisor, NULL, games_supervisor_handler, (void*)&server_games) != 0) {
        perror("Erreur lors de la création du thread de gestion du démarrage des parties");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 client_tcp;
    socklen_t client_tcp_len = sizeof(client_tcp);

    while(1) {
        int client_tcp_socket = accept(tcp_socket, (struct sockaddr *)&client_tcp, &client_tcp_len);
        if (client_tcp_socket == -1) {
            perror("Error accepting TCP connection");
            continue;
        }

        log_info("Le client c'est bien connecté");

        CReq join_rq = {0};
        if(recv_client_request(client_tcp_socket, &join_rq, sizeof(join_rq.req.join))) {
            perror("Erreur recv join request");
            close(client_tcp_socket);
            continue;
        }

        debug("Join request received");
        debug_creq(&join_rq);

        SReq start_rq = {0};
        if(create_regestartionrq(&server_games, &start_rq, &join_rq.req.join)) {
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

        struct pollfd fds[1];
        fds[0].fd = client_tcp_socket;
        fds[0].events = POLLIN;

        int ret = poll(fds, 1, TIMEOUT_MS);

        if (ret == -1) {
            perror("Error in poll");
            exit(EXIT_FAILURE);
        } else if (ret == 0) {
            log_info("Le client n'a rien envoyé pendant le délai imparti. Déconnexion...");
            close(client_tcp_socket);
            remove_client(&server_games, start_rq.req.start.portudp, get_id(start_rq.req.start.header));
            continue;
        } else {
            if (fds[0].revents & POLLIN) {
                debug("Socket has data");
                CReq conf_rq = {0};
                if(recv_client_request(client_tcp_socket, &conf_rq, sizeof(conf_rq.req.join))) {
                    perror("Erreur recv conf request");
                    close(client_tcp_socket);
                    exit(EXIT_FAILURE);
                }

                debug_creq(&conf_rq);
                debug("Le client a confirmé qu'il veut rejoindre la partie");
                set_player_status(&server_games, start_rq.req.start.portudp, get_id(conf_rq.req.join.header), READY);
            }
        }  
    }

    pthread_join(games_supervisor, NULL);
    close_server(tcp_socket, &server_games);

}