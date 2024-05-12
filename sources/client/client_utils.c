#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "client/client_utils.h"
#include "socket_utils.h"
#include "debug.h"
#include "game.h"

int init_client(uint16_t port_tcp, char *server_ip) {

    int tcp_socket = socket(PF_INET6, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("Error creating TCP socket");
        return -1;
    }

    struct sockaddr_in6 server_tcp = {0};
    server_tcp.sin6_family = AF_INET6;
    inet_pton(AF_INET6, server_ip, &server_tcp.sin6_addr);
    server_tcp.sin6_port = htons(port_tcp);

    if (connect(tcp_socket, (struct sockaddr *)&server_tcp, sizeof(server_tcp)) == -1) {
        perror("Error connecting to server");
        return -1;
    }

    return tcp_socket;
}

int subscribe_multicast(uint16_t port_multicast, char* adr_multicast) {
    int sock_multicast = socket(PF_INET6, SOCK_DGRAM, 0);

    if(sock_multicast < 0) {
        perror("socket creation failed");
        return 1;
    }

    struct sockaddr_in6 multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin6_family = AF_INET6;
    multicast_addr.sin6_addr = in6addr_any;
    multicast_addr.sin6_port = htons(port_multicast);

    if(set_reuseaddr(sock_multicast) < 0) {
        close(sock_multicast);
        exit(EXIT_FAILURE);
    }

    if(bind(sock_multicast, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr))) {
        perror("erreur bind");
        close(sock_multicast);
        return -1;
    }

    if(set_join_group(sock_multicast, adr_multicast)) {
        close(sock_multicast);
        return -1;
    }

    debug("L'abonnement à l'adresse de multidiffusion %s sur le port %d est faite", adr_multicast, port_multicast);
    return sock_multicast;
}

UDP_Infos *init_udp_connection(uint16_t port_udp) {
    int sockfd = socket(AF_INET6, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("Erreur while creating UDP socket");
        return NULL;
    }

    struct sockaddr_in6 serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(port_udp);
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_scope_id = 0;

    if(set_reuseaddr(sockfd)) {
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur while binding socket with the server adresse");
        close(sockfd);
        return NULL;
    }

    debug("La connexion UDP IPv6 sur le port %d a été initialisée avec succès.", port_udp);
    UDP_Infos *udp_infos = malloc(sizeof(UDP_Infos));
    if (udp_infos == NULL) {
        perror("Erreur lors de l'allocation de la structure UDP_Infos");
        close(sockfd);
        return NULL;
    }

    udp_infos->sock_udp = sockfd;
    memcpy(&(udp_infos->server_addr), &serv_addr, sizeof(serv_addr));

    return udp_infos;
}

//envoyer un message via TCP
void sendMessage(Message *msg, int socket) {
    u_int16_t codereq = CALL_CHAT;
    Header_t header = (codereq | ((msg->id) << CODEREQ_LEN) | ((msg->eq) << (CODEREQ_LEN + ID_LEN)));

    send(socket, &header, sizeof(Header_t),0);
    // envoie la structure de message entière
    send(socket, &(msg->len), sizeof(u_int8_t), 0);
    // envoye les données textuelles
    send(socket, msg->data, (msg->len)*sizeof(char), 0);
}
Message *initMessage(u_int8_t id_player, u_int8_t id_team){
    Message *msg = malloc(sizeof(Message));
    msg-> codereq =CODEREQ_ALL_PLAYERS ;
    msg->id = id_player;
    msg->eq = id_team;
    msg->len = 0;
    msg->data = malloc(256*sizeof(char));
    return msg;
}

void clearMessage(Message *msg){
    msg->len = 0;
}
void freeMessage(Message* msg){
    free(msg->data);
    free(msg);
}