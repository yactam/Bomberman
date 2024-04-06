#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#include <stdlib.h>
#include <stdint.h>

#define MAX_DATA 255

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


typedef enum : uint8_t {
	GO_NORTH,
	GO_EAST,
	GO_SOUTH,
	GO_OUEST,
	DROP_BOMB,
	UNDO
} action_t;

typedef enum : uint8_t {
	EMPTY,
	IWALL,
	DWALL,
	BOMB,
	EXPLOSE,
	PLAYER1,
	PLAYER2,
	PLAYER3,
	PLAYER4
} cases_t;


typedef struct {
	uint16_t codereq : CODEREQ_LEN;
	uint8_t id : ID_LEN;
	uint8_t eq : 1;
} Header_t;

typedef struct {
	Header_t header;
} CReq_Join;

typedef struct {
	uint16_t num : CNUM_LEN;
	action_t action : 3;
} Message_t;

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

// TODO: Modifier par les vraies tailles du tableau
typedef struct {
	Header_t header;
	uint16_t num;
	uint8_t hauteur;
	uint8_t largeur;
	cases_t cells[1][1];
} SReq_Grid;

typedef struct {
	uint8_t row;
	uint8_t col;
} Coord;

typedef struct {
	Coord coord;
	cases_t content : 8;
} Cell;

// TODO: Modifier par les vraies tailles du tableau
typedef struct {
	Header_t header;
	uint16_t num;
	uint8_t nb;
	Cell cells[1][1];
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

#endif
