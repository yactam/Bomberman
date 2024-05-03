#include "socket_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

int set_reuseaddr(int sockfd) {
    int ok = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
		perror("echec de SO_REUSEADDR");
		return 1;
	}
    return 0;
}

int set_join_group(int sock_multicast, char *adr_multicast) {
    struct ipv6_mreq group = {0};
    inet_pton(AF_INET6, adr_multicast, &(group.ipv6mr_multiaddr));
    group.ipv6mr_interface = 0;

    if (setsockopt(sock_multicast, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group)) < 0) {
        perror("setsockopt erreur subscribe to group");
        return 1;
    }
    return 0;
}

int set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0)
		return -1;

	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;

	return flags;
}
