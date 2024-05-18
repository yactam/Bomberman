#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "client/client_utils.h"
#include "socket_utils.h"
#include "debug.h"
#include "game.h"

int init_client(char *port_tcp, char *hostname) {

    struct addrinfo hints, *r, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL;

    int status = getaddrinfo(hostname, port_tcp, &hints, &r);
    if (status != 0) {
        dprintf(STDERR_FILENO, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

	p = r;
	int tcp_socket = -1;
	while(p != NULL) {
		tcp_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (tcp_socket > 0) {
			if (connect(tcp_socket, p->ai_addr, sizeof(struct sockaddr_in6)) == 0) {
				break;
			}
		}
        p = p->ai_next;
	}

    if (p == NULL) {
		dprintf(STDERR_FILENO, "connection failed\n");
		freeaddrinfo(r);
		return -1;
	}

    freeaddrinfo(r);
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
    int sockfd;

    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Erreur while creating UDP socket");
        return NULL;
    }

    struct sockaddr_in6 serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(port_udp);
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_scope_id = 0;

    int ok = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
		perror("echec de SO_REUSEADDR");
		close(sockfd);
		return NULL;
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

void clearMessage(Message *msg) {
    memset(msg->data, 0, sizeof(msg->data));
    msg->length = 0;
}
void freeMessage(Message* msg){
    free(msg->data);
    free(msg);
}