#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "client/network.h"

Buf_t hton_integrationrq(CReq_Join *join_rq) {
    Buf_t buf;
    initbuf(&buf);

    Header_t header = htons(join_rq->header);
    appendbuf(&buf, &header, sizeof(header));

    return buf;
}

Buf_t hton_ongamerq(CReq_Play *play_rq) {
    Buf_t buf;
    initbuf(&buf);

    Header_t header = htons(play_rq->header);
    Message_t message = htons(play_rq->message);
    appendbuf(&buf, &header, sizeof(header));
    appendbuf(&buf, &message, sizeof(message));

    return buf;
}

Buf_t hton_tchat(Creq_Tchat *tchat_rq){
    Buf_t buf;
    initbuf(&buf);

    Header_t header = htons(tchat_rq->header);
    appendbuf(&buf, &header, sizeof(header));
    appendbuf(&buf,&tchat_rq->len,sizeof(tchat_rq->len));
    appendbuf(&buf,tchat_rq->data,tchat_rq->len);
    
    return buf;
}

SReq_Start ntoh_start(Buf_t *start) {
    SReq_Start start_rq = {0};

    Header_t header;
    uint16_t port_udp;
    uint16_t port_multicast;
    unsigned char adrmulticast[16];

    size_t from = 0;
    copyfrombuf(start, &from, sizeof(header), &header);
    copyfrombuf(start, &from, sizeof(port_udp), &port_udp);
    copyfrombuf(start, &from, sizeof(port_multicast), &port_multicast);
    copyfrombuf(start, &from, sizeof(adrmulticast), &adrmulticast);

    start_rq.header = ntohs(header);
    start_rq.portudp = ntohs(port_udp);
    start_rq.portmdiff = ntohs(port_multicast);
    memcpy(start_rq.adrmdiff, adrmulticast, sizeof(adrmulticast));

    return start_rq;
}


SReq_Grid ntoh_grid(Buf_t *grid) {
    SReq_Grid grid_rq = {0};

    Header_t header;
    uint16_t num;
    uint8_t hauteur;
    uint8_t largeur;

    size_t from = 0;
    copyfrombuf(grid, &from, sizeof(header), &header);
    copyfrombuf(grid, &from, sizeof(num), &num);
    copyfrombuf(grid, &from, sizeof(hauteur), &hauteur);
    copyfrombuf(grid, &from, sizeof(largeur), &largeur);

    grid_rq.header = ntohs(header);
    grid_rq.num = ntohs(num);
    grid_rq.hauteur = hauteur;
    grid_rq.largeur = largeur;

    for (size_t i = 0; i < hauteur; ++i) {
        for (size_t j = 0; j < largeur; ++j) {
            cases_t cell;
            copyfrombuf(grid, &from, sizeof(cell), &cell);
            grid_rq.cells[i][j] = cell;
        }
    }

    return grid_rq;
}

SReq_Cell ntoh_cell(Buf_t *cells) {
    SReq_Cell cell_rq = {0};

    Header_t header;
    uint16_t num;
    uint8_t nb;

    size_t from = 0;
    copyfrombuf(cells, &from, sizeof(header), &header);
    copyfrombuf(cells, &from, sizeof(num), &num);
    copyfrombuf(cells, &from, sizeof(nb), &nb);

    cell_rq.header = ntohs(header);
    cell_rq.num = ntohs(num);
    cell_rq.nb = nb;

    for(size_t i = 0; i < nb; ++i) {
        Cell cell;
        copyfrombuf(cells, &from, sizeof(cell), &cell);
        cell_rq.cells[i] = cell;
    }

    return cell_rq;
}

SReq_Tchat ntoh_tchat(Buf_t *buf_rq){
    SReq_Tchat tchat = {0};
    Header_t header;
    
    size_t start = 0;
    copyfrombuf(buf_rq, &start, sizeof(header), &header);
    copyfrombuf(buf_rq, &start, sizeof(tchat.len), &tchat.len);
    copyfrombuf(buf_rq, &start, tchat.len, tchat.data);

    tchat.header = ntohs(header);
    
    return tchat;
}

SReq_End ntoh_end(Buf_t *buf_rq){
    SReq_End endrq = {0};
    Header_t header;
    
    size_t start = 0;
    copyfrombuf(buf_rq, &start, sizeof(header), &header);

    endrq.header = ntohs(header);
    
    return endrq;
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

uint8_t send_client_request(int sockfd, CReq *client_rq) {
    Buf_t bytes_rq;
    initbuf(&bytes_rq);

    debug("The type of the request to send is %d", client_rq->type);

    if(client_rq->type == CREQ_MODE4 || client_rq->type == CREQ_TEAMS || client_rq->type == CCONF_MODE4 || client_rq->type == CCONF_TEAMS) {
        bytes_rq = hton_integrationrq(&client_rq->req.join);
    } else if(client_rq->type == CALL_CHAT || client_rq->type == CCOP_CHAT){
        bytes_rq = hton_tchat(&client_rq->req.tchat);
    }
    else{
        log_error("Type de requête non reconnu %d", client_rq->type);
        return 1;
    }

    if(send_tcp(sockfd, bytes_rq.content, bytes_rq.size) < 0) {
        return 1;
    }

    return 0;
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


ssize_t recv_server_request(int sockfd, SReq *server_rq) {
    Buf_t buf_recv;

    ssize_t ret = recv_tcp(sockfd, sizeof(Header_t), &buf_recv);
    if(ret < 0) {
        return -1;
    } else if(ret == 0) {
        return 0;
    }

    Header_t header;
    size_t start = 0;
    copyfrombuf(&buf_recv, &start, sizeof(header), &header);
    server_rq->type = get_codereq(ntohs(header));

    debug("The type of the received request is %d", server_rq->type);

    if(server_rq->type == SREQ_MODE4 || server_rq->type == SREQ_TEAMS) {
        Buf_t start_rq;
        ret = recv_tcp(sockfd, sizeof(SReq_Start) - sizeof(Header_t), &start_rq);
        if(ret < 0) {
            return -1;
        } else if(ret == 0) {
            return 0;
        }
        debug("La taille : %ld", sizeof(start_rq.content));
        appendbuf(&buf_recv, start_rq.content, start_rq.size);
        server_rq->req.start = ntoh_start(&buf_recv);

    } else if(server_rq->type == SALL_CHAT || server_rq->type == SCOP_CHAT) {
        Buf_t tchat_buf;
        size_t tchat_start = 0;
        u_int8_t len;

        ret = recv_tcp(sockfd, sizeof(uint8_t), &tchat_buf);
        if(ret < 0) {
            return -1;
        } else if(ret == 0) {
            return 0;
        }
        appendbuf(&buf_recv, tchat_buf.content, ret);
        copyfrombuf(&tchat_buf, &tchat_start, sizeof(uint8_t), &len);

        ret = recv_tcp(sockfd, len, &tchat_buf);
        if(ret < 0) {
            return -1;
        } else if(ret == 0) {
            return 0;
        }
        appendbuf(&buf_recv, &tchat_buf.content, ret);

        server_rq->req.tchat = ntoh_tchat(&buf_recv);
    } else if(server_rq->type == SGAMEOVER_MODE4 || server_rq->type == SGAMEOVER_TEAMS) {
        server_rq->req.end = ntoh_end(&buf_recv);
    } else {
        log_error("Type de requête non reconnu %d", server_rq->type);
        return 1;
    }

    return 0;
}

uint8_t recv_server_datagram(int sockfd, SReq *server_rq, size_t max_recv) {
    Buf_t buf_recv;
    initbuf(&buf_recv);

    ssize_t nbytes = recvfrom(sockfd, buf_recv.content, max_recv, 0, NULL, NULL);
    if (nbytes < 0) {
        return 1;
    }

    buf_recv.size = (size_t) nbytes;

    Header_t header;
    size_t start = 0;
    copyfrombuf(&buf_recv, &start, sizeof(header), &header);
    server_rq->type = get_codereq(htons(header));

    debug("The type of the received udp request is %d", server_rq->type);

    if(server_rq->type == SDIFF_GRID) {
        server_rq->req.grid = ntoh_grid(&buf_recv);
    } else if(server_rq->type == SDIFF_CASES) {
        server_rq->req.cell = ntoh_cell(&buf_recv);
    } else {
        log_error("Erreur %d\n", server_rq->type);
    }

    return 0;
}

uint8_t send_datagram(int sfd, struct sockaddr_in6 serv_addr, CReq *client_rq) {
    Buf_t bytes_rq;
    uint16_t type = client_rq->type;

    debug("Send client request datagram of type %d", type);

    if(type == CON_MODE4 || type == CON_TEAMS) {
        bytes_rq = hton_ongamerq(&client_rq->req.play);

        if(sendto(sfd, bytes_rq.content, bytes_rq.size, 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("Erreur send client request datagram grid");
            return 1;
        }
    } else {
        log_error("Erreur %d\n", type);
    }
    return 0;
}
