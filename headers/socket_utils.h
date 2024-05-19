#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

int set_reuseaddr(int sockfd);
int set_join_group(int sock_multicast, char *adr_multicast);
int set_non_blocking(int sockfd);

#endif