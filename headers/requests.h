#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_DATA 255
#define MAX_BUFSIZE 4069
#define MAX_ACTIONS 4069

#define CODEREQ_LEN 13
#define ID_LEN 2
#define CNUM_LEN 13

#define CREQ_MODE4 1
#define CREQ_TEAMS 2
#define CCONF_MODE4 3
#define CCONF_TEAMS 4
#define CON_MODE4 5
#define CON_TEAMS 6
#define CALL_CHAT 7
#define CCOP_CHAT 8

#define SREQ_MODE4 9
#define SREQ_TEAMS 10
#define SDIFF_GRID 11
#define SDIFF_CASES 12
#define SALL_CHAT 13
#define SCOP_CHAT 14
#define SGAMEOVER_MODE4 15
#define SGAMEOVER_TEAMS 16

#define CODEREQ_MASK 0x1FFF
#define MESSAGE_MASK 0x1FFF
#define ID_MASK 0x6000
#define EQ_MASK 0x8000
#define ACTION_MASK 0xE000

#define MAX_WIDTH 256
#define MAX_HEIGHT 256


typedef enum{
	GO_NORTH=0,
	GO_EAST=1,
	GO_SOUTH=2,
	GO_OUEST=3,
	DROP_BOMB=4,
	UNDO=5
} action_t;

typedef enum{
	EMPTY=0,
	IWALL=1,
	DWALL=2,
	BOMB=3,
	EXPLOSE=4,
	PLAYER1=5,
	PLAYER2=6,
	PLAYER3=7,
	PLAYER4=8
} cases_t;


typedef uint16_t Header_t;

typedef struct {
	Header_t header;
} CReq_Join;

typedef uint16_t Message_t;

typedef struct {
	Header_t header;
	Message_t message;	
} CReq_Play;

typedef struct {
	Header_t header;
	uint8_t len;
	char data[MAX_DATA];
} Creq_Tchat;

typedef uint8_t reqtype_t;

/**
 * @struct CReq
 * @brief Represents a client request
 */
typedef struct {
	reqtype_t type; /**< The type of the request */
	union {
		CReq_Join join; /**< join request */
		CReq_Play play; /**< play request */
		Creq_Tchat tchat; /**< tchat request */
	} req;  /**< Union representing different types of client requests */
} CReq;

typedef struct {
	Header_t header;
	uint16_t portudp;
	uint16_t portmdiff;
	unsigned char adrmdiff[16];
} SReq_Start;

typedef struct {
	Header_t header;
	uint16_t num;
	uint8_t hauteur;
	uint8_t largeur;
	cases_t cells[MAX_HEIGHT][MAX_WIDTH];
} SReq_Grid;

typedef struct {
	uint8_t row;
	uint8_t col;
} Coord;

typedef struct {
	Coord coord;
	uint8_t content;
} Cell;

typedef struct {
	Header_t header;
	uint16_t num;
	uint8_t nb;
	Cell cells[MAX_HEIGHT * MAX_WIDTH];
} SReq_Cell;

typedef struct {
	Header_t header;
	uint8_t len;
	unsigned char data[MAX_DATA];
} SReq_Tchat;

typedef struct {
	Header_t header;
} SReq_End;

/**
 * @struct SReq
 * @brief Represents a server request
 */
typedef struct {
	reqtype_t type; /**< The type of the request */
	union {
		SReq_Start start; /**< start game request */
		SReq_Grid grid; /**< multicast grid request */
		SReq_Cell cell; /**< multicast updated cells request */
		SReq_Tchat tchat; /**< chat request */
		SReq_End end; /**< game end request */
	} req;  /**< Union representing different types of server requests */
} SReq;

typedef struct {
	char content[MAX_BUFSIZE];
	size_t size;
} Buf_t;

typedef struct {
    int sock_udp;
    struct sockaddr_in6 server_addr;
} UDP_Infos;

typedef struct {
	int client_tcp_sock;
	uint8_t client_id;
	time_t last_activity;
	uint16_t game_udp_port;
} Client_Infos;

void initbuf(Buf_t *buffer);
int appendbuf(Buf_t *buffer, void *elt, size_t elt_size);
int copyfrombuf(Buf_t *buffer, size_t *start, size_t elt_size, void *dst);

uint16_t get_codereq(Header_t header);
uint16_t get_id(Header_t header);
uint16_t get_eq(Header_t header);
uint8_t get_action(Message_t message);
uint16_t get_num(Message_t message);


void debug_creq(CReq *client_rq);
void debug_sreq(SReq *server_rq);

#endif
