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

uint8_t recv_client_request(int sockfd, CReq *client_rq, size_t sto_recv) {
    Buf_t req_buf;
    initbuf(&req_buf);

    size_t read = 0;
    while(read < sto_recv) {
        int rec = recv(sockfd, req_buf.content + read, sto_recv - read, 0);
        if(rec < 0) {
            perror("Erreur recv");
            return 1;
        }
        read += rec;
        debug("%d octect reçue, au total: %d, à recevoir: %d", rec, read, sto_recv);
        if(rec == 0) {
            log_info("Le client s'est déconnecté");
            return 1;
        }
    }

    req_buf.size += read;

    Header_t header;
    size_t start = 0;
    copyfrombuf(&req_buf, &start, sizeof(header), &header);
    client_rq->type = get_codereq(ntohs(header));

    debug("Received client request of type %d", client_rq->type);

    if(client_rq->type == CREQ_MODE4 || client_rq->type == CREQ_TEAMS || client_rq->type == CCONF_MODE4 || client_rq->type == CCONF_TEAMS) {
        client_rq->req.join = ntoh_integrationrq(&req_buf);
    } else if(client_rq->type == CALL_CHAT){
        debug("recv fctn");
        client_rq->req.tchat.header =get_header(&req_buf);
        u_int8_t size = 0;
        recv(sockfd,&size,sizeof(u_int8_t),0);
        debug("message size %d",size);
        //char* buf = malloc(size*sizeof(char));
        size_t recvd = 0;
        while(recvd<size){
            int rec = recv(socket,(client_rq->req.tchat.data)+recvd,size,0);
            if(rec < 0) {
                perror("Erreur recv");
                return 1;
            }
            recvd += rec;
            if(rec==0){
                log_info("client disconected");
                return 1;
            }
        }
        client_rq -> req.tchat.len = size;
        //client_rq -> req.tchat.data = buf;
        log_info("message reçu: %s", client_rq->req.tchat.data);
    }
    else {
        debug("aled fctn");
        // TODO : continuer l'implementation avec toutes les requetes possibles 
        log_info("Not yet %d\n", client_rq->type);
    }

    return 0;
}

uint8_t send_server_request(int sockfd, SReq *server_rq) {
    Buf_t bytes_rq;
    uint16_t type = server_rq->type;

    debug("Send server request of type %d", type);

    if(type == SREQ_MODE4 || type == SREQ_TEAMS) {
        bytes_rq = hton_startrq(&server_rq->req.start);

        if(send(sockfd, bytes_rq.content, bytes_rq.size, 0) < 0) {
            perror("Erreur send server request start");
            return 1;
        }
    }else if(type == CALL_CHAT){

    } 
    
    else {
        // TODO : À implementer avec toutes les autres requêtes
        log_info("Not yet %d\n", type);
    }
    return 0;
}

u_int8_t send_server_tchat(int socket, SReq *msg){

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
        // TODO : À implementer avec toutes les autres requêtes
        log_info("Not yet %d\n", type);
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