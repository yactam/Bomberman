#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#include <stdlib.h>
#include <stdint.h>

#define CODEREQ_LEN 13
#define ID_LEN 2
#define NUM_LEN 13

#define REQ_MODE4 1
#define REQ_TEAMS 2
#define CONF_MODE4 3
#define CONF_TEAMS 4
#define ON_MODE4 5
#define ON_TEAMS 6
#define ALL_CHAT 7
#define COP_CHAT 8

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define OUEST 3
#define BOMB 4
#define UNDO 5

typedef struct {
	uint16_t codereq : CODEREQ_LEN;
	uint8_t id : ID_LEN;
	uint8_t eq : 1;
} Header_t;

typedef struct {
	Header_t header;
} CReq_Join;

typedef struct {
	uint16_t num : NUM_LEN;
	uint8_t action : 3;
} Message_t;

typedef struct {
	Header_t header;
	Message_t message;	
} CReq_Play;

typedef struct {
	Header_t header;
	uint8_t len;
	char* data;
} Creq_Tchat;

typedef uint8_t reqtype_t;

/**
 * @struct CReq
 * @brief Represents a client request
 */
typedef struct {
	reqtype_t type; /**< The type of the request */
	union {
		CReq_Join join; /**< Join request */
		CReq_Play play; /**< Play request */
		Creq_Tchat tchat; /**< Tchat request */
	} req;  /**< Union representing different types of client requests */
} CReq;

#endif
