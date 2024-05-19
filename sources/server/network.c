#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "requests.h"
#include "debug.h"
#include "server/network.h"

Buf_t hton_startrq(SReq_Start *start_rq) {
    Buf_t bytes_rq;
    initbuf(&bytes_rq);

    Header_t header = htons(start_rq->header);
    uint16_t port_udp = htons(start_rq->portudp);
    uint16_t port_multicast = htons(start_rq->portmdiff);

    appendbuf(&bytes_rq, &header, sizeof(header));
    appendbuf(&bytes_rq, &port_udp, sizeof(port_udp));
    appendbuf(&bytes_rq, &port_multicast, sizeof(port_multicast));
    appendbuf(&bytes_rq, start_rq->adrmdiff, sizeof(start_rq->adrmdiff));

    return bytes_rq;
}

Buf_t hton_gridrq(SReq_Grid *grid_rq) {
    Buf_t bytes_rq;
    initbuf(&bytes_rq);

    Header_t header = htons(grid_rq->header);
    uint16_t num = htons(grid_rq->num);

    appendbuf(&bytes_rq, &header, sizeof(header));
    appendbuf(&bytes_rq, &num, sizeof(num));
    appendbuf(&bytes_rq, &grid_rq->hauteur, sizeof(grid_rq->hauteur));
    appendbuf(&bytes_rq, &grid_rq->largeur, sizeof(grid_rq->largeur));
    for (size_t i = 0; i < grid_rq->hauteur; ++i) {
        for (size_t j = 0; j < grid_rq->largeur; ++j) {
            cases_t cell = grid_rq->cells[i][j];
            appendbuf(&bytes_rq, &cell, sizeof(cell));
        }
    }

    return bytes_rq;
}

Buf_t hton_cellsrq(SReq_Cell *cell_rq) {
    Buf_t bytes_rq;
    initbuf(&bytes_rq);

    Header_t header = htons(cell_rq->header);
    uint16_t num = htons(cell_rq->num);
    uint8_t nb = cell_rq->nb;

    appendbuf(&bytes_rq, &header, sizeof(header));
    appendbuf(&bytes_rq, &num, sizeof(num));
    appendbuf(&bytes_rq, &nb, sizeof(nb));

    for(size_t i = 0; i < nb; ++i) {
        appendbuf(&bytes_rq, &cell_rq->cells[i].coord.row, sizeof(cell_rq->cells[i].coord.row));
        appendbuf(&bytes_rq, &cell_rq->cells[i].coord.col, sizeof(cell_rq->cells[i].coord.col));
        appendbuf(&bytes_rq, &cell_rq->cells[i].content, sizeof(cell_rq->cells[i].content));
    }

    return bytes_rq;

}

Buf_t hton_chatrq(SReq_Tchat *tchat_rq) {
    Buf_t bytes_rq;
    initbuf(&bytes_rq);

    Header_t header = htons(tchat_rq->header);
    uint8_t len = tchat_rq->len;

    appendbuf(&bytes_rq, &header, sizeof(header));
    appendbuf(&bytes_rq, &len, sizeof(len));
    appendbuf(&bytes_rq, tchat_rq->data, len * sizeof(char));

    return bytes_rq;
}

Buf_t hton_endrq(SReq_End *end_rq) {
    Buf_t bytes_rq;
    initbuf(&bytes_rq);

    Header_t header = htons(end_rq->header);

    appendbuf(&bytes_rq, &header, sizeof(header));

    return bytes_rq;
}

CReq_Join ntoh_integrationrq(Buf_t *buf_rq) {
    CReq_Join join = {0};
    Header_t header;
    
    size_t start = 0;
    copyfrombuf(buf_rq, &start, sizeof(header), &header);

    join.header = ntohs(header);
    return join;
}

Header_t get_header(Buf_t *buf_rq){
    
    Header_t header;
    
    size_t start = 0;
    copyfrombuf(buf_rq, &start, sizeof(header), &header);
    
    return ntohs(header);
}

CReq_Play ntoh_ongamerq(Buf_t *buf_rq) {
    CReq_Play play = {0};
    Header_t header;
    Message_t message;

    size_t start = 0;
    copyfrombuf(buf_rq, &start, sizeof(header), &header);
    copyfrombuf(buf_rq, &start, sizeof(message), &message);

    play.header = ntohs(header);
    play.message = ntohs(message);

    return play;
}

Creq_Tchat ntoh_tchat(Buf_t *buf_rq){
    Creq_Tchat tchat = {0};
    Header_t header;
    
    size_t start = 0;
    copyfrombuf(buf_rq, &start, sizeof(header), &header);
    copyfrombuf(buf_rq, &start, sizeof(tchat.len), &tchat.len);
    copyfrombuf(buf_rq, &start, tchat.len, tchat.data);

    tchat.header = ntohs(header);
    
    return tchat;
}

ssize_t recv_tcp(int sockfd, size_t sto_recv, Buf_t *req_buf){
    
    initbuf(req_buf);

    size_t read = 0;
    while(read < sto_recv) {
        int rec = recv(sockfd, req_buf->content + read, sto_recv - read, 0);
        if(rec < 0) {
            perror("Erreur recv");
            return -1;
        }
        read += rec;
        debug("%d octect reçue, au total: %d, à recevoir: %d", rec, read, sto_recv);
        if(rec == 0) {
            log_info("Le client s'est déconnecté");
            return 0;
        }
    }

    req_buf->size += read;
    return read;
}

ssize_t recv_client_request(int sockfd, CReq *client_rq) {
    Buf_t req_buf;

    ssize_t ret = recv_tcp(sockfd, sizeof(Header_t), &req_buf);
    if(ret < 0) {
        return -1;
    } else if(ret == 0) {
        return 0;
    }

    Header_t header;
    size_t start = 0;
    copyfrombuf(&req_buf, &start, sizeof(header), &header);
    client_rq->type = get_codereq(ntohs(header));

    debug("Received client request of type %d", client_rq->type);

    if(client_rq->type == CREQ_MODE4 || client_rq->type == CREQ_TEAMS || client_rq->type == CCONF_MODE4 || client_rq->type == CCONF_TEAMS) {
        client_rq->req.join = ntoh_integrationrq(&req_buf);
    } else if(client_rq->type == CALL_CHAT || client_rq->type == CCOP_CHAT){
        Buf_t tchat_buf;
        size_t tchat_start = 0;
        u_int8_t len;

        ret = recv_tcp(sockfd, sizeof(uint8_t), &tchat_buf);
        if(ret < 0) {
            return -1;
        } else if(ret == 0) {
            return 0;
        }
        appendbuf(&req_buf, tchat_buf.content, ret);
        copyfrombuf(&tchat_buf, &tchat_start, sizeof(uint8_t), &len);

        ret = recv_tcp(sockfd, len, &tchat_buf);
        if(ret < 0) {
            return -1;
        } else if(ret == 0) {
            return 0;
        }
        appendbuf(&req_buf, &tchat_buf.content, ret);

        client_rq->req.tchat = ntoh_tchat(&req_buf);
    }
    else {
        log_info("Erreur %d\n", client_rq->type);
    }

    return 0;
}

ssize_t send_tcp(int sockfd, const void *buf, size_t len) {
    ssize_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = send(sockfd, (const char *)buf + total_sent, len - total_sent, 0);
        if (sent == -1) {
            if (errno == EPIPE || errno == ECONNRESET) {
                perror("Connection closed by the peer");
                return -1;
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("send");
                return -1;
            }
        }
        total_sent += sent;
    }
    return total_sent;
}

uint8_t send_server_request(int *sockfds, size_t nb_socks, SReq *server_rq) {
    Buf_t bytes_rq;
    uint16_t type = server_rq->type;

    if(type == SREQ_MODE4 || type == SREQ_TEAMS) {
        bytes_rq = hton_startrq(&server_rq->req.start);
    } else if(type == SALL_CHAT || type == SCOP_CHAT) {
        bytes_rq = hton_chatrq(&server_rq->req.tchat);
    } else if(type == SGAMEOVER_MODE4 || type == SGAMEOVER_TEAMS) {
        bytes_rq = hton_endrq(&server_rq->req.end);
    }

    for(size_t i = 0; i < nb_socks; ++i) {
        int sfd = sockfds[i];
        if(sfd != 0) {
            debug("Send server request of type %d to %d", type, sfd);
            if(send_tcp(sfd, bytes_rq.content, bytes_rq.size) < 0) {
                perror("Erreur send server request start");
                continue;
            }
        }
    }

    return 0;
}

uint8_t send_datagram(int sfd, struct sockaddr_in6 gradr, SReq *server_rq) {
    Buf_t bytes_rq;
    uint16_t type = server_rq->type;

    debug("Send server request datagram of type %d", type);

    if(type == SDIFF_GRID) {
        bytes_rq = hton_gridrq(&server_rq->req.grid);

        if(sendto(sfd, bytes_rq.content, bytes_rq.size, 0, (struct sockaddr*) &gradr, sizeof(gradr)) < 0) {
            perror("Erreur send server request datagram grid");
            return 1;
        }
    } else if(type == SDIFF_CASES) {
        bytes_rq = hton_cellsrq(&server_rq->req.cell);

        if(sendto(sfd, bytes_rq.content, bytes_rq.size, 0, (struct sockaddr*) &gradr, sizeof(gradr)) < 0) {
            perror("Erreur send server request datagram grid");
            return 1;
        }
    } else {
        log_error("Datagram inconnu %d\n", type);
    }
    return 0;
}

uint8_t recv_client_datagrams(int sfd, CReq *client_rq) {
    Buf_t req_buf;
    initbuf(&req_buf);

    int rec = recvfrom(sfd, req_buf.content, MAX_BUFSIZE, 0, NULL, NULL);
    if(rec < 0) {
        return 1;
    }
    req_buf.size = rec;

    Header_t header;
    size_t start = 0;
    copyfrombuf(&req_buf, &start, sizeof(header), &header);
    client_rq->type = get_codereq(htons(header));
    debug("Received client request of type %d", client_rq->type);

    if(client_rq->type != CON_MODE4 && client_rq->type != CON_TEAMS) {
        log_error("Type mismatch: received a '%d' packet but expected a '%d or %d' one.", client_rq->type, CON_MODE4, CON_TEAMS);
        return 1;
    }
    client_rq->req.play = ntoh_ongamerq(&req_buf);

    return 0;     
}