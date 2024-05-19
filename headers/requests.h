#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * @file requests.h
 * @brief Defines structures and functions for handling client and server requests.
 */

#define MAX_DATA 255
#define MAX_BUFSIZE 4069
#define MAX_ACTIONS 4069

#define CODEREQ_LEN 13
#define ID_LEN 2
#define EQ_LEN 1
#define CNUM_LEN 13
#define ACTION_LEN 3

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
#define ID_MASK 0x3
#define EQ_MASK 0x1
#define ACTION_MASK 0x7

#define MAX_WIDTH 256
#define MAX_HEIGHT 256

/**
 * @enum action_t
 * @brief Represents possible actions in the game.
 */
typedef enum {
    GO_NORTH=0,
    GO_EAST=1,
    GO_SOUTH=2,
    GO_OUEST=3,
    DROP_BOMB=4,
    UNDO=5
} action_t;

/**
 * @enum player_status_t
 * @brief Represents the status of a player.
 */
typedef enum {
    DISCONNECTED = 0,
    CONNECTING,
    READY,
    PLAYING,
    DEAD
} player_status_t;

/**
 * @enum cases_t
 * @brief Represents the content of a cell in the game grid.
 */
typedef enum {
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

typedef uint16_t Header_t; /**< Type representing the header in requests. */
typedef uint16_t Message_t; /**< Type representing the message in requests. */

/**
 * @struct CReq_Join
 * @brief Represents a client join request.
 */
typedef struct {
    Header_t header;
} CReq_Join;

/**
 * @struct CReq_Play
 * @brief Represents a client play request.
 */
typedef struct {
    Header_t header;
    Message_t message;    
} CReq_Play;

/**
 * @struct Creq_Tchat
 * @brief Represents a client chat request.
 */
typedef struct {
    Header_t header;
    uint8_t len;
    char data[MAX_DATA];
} Creq_Tchat;

typedef uint8_t reqtype_t; /**< Type representing the request type. */

/**
 * @struct CReq
 * @brief Represents a client request.
 */
typedef struct {
    reqtype_t type; /**< The type of the request */
    union {
        CReq_Join join; /**< join request */
        CReq_Play play; /**< play request */
        Creq_Tchat tchat; /**< chat request */
    } req;  /**< Union representing different types of client requests */
} CReq;

/**
 * @struct SReq_Start
 * @brief Represents a server start request.
 */
typedef struct {
    Header_t header;
    uint16_t portudp;
    uint16_t portmdiff;
    unsigned char adrmdiff[16];
} SReq_Start;

/**
 * @struct SReq_Grid
 * @brief Represents a server grid request.
 */
typedef struct {
    Header_t header;
    uint16_t num;
    uint8_t hauteur;
    uint8_t largeur;
    cases_t cells[MAX_HEIGHT][MAX_WIDTH];
} SReq_Grid;

/**
 * @struct Coord
 * @brief Represents the coordinates of a cell.
 */
typedef struct {
    uint8_t row;
    uint8_t col;
} Coord;

/**
 * @struct Cell
 * @brief Represents a cell with coordinates and content.
 */
typedef struct {
    Coord coord;
    uint8_t content;
} Cell;

/**
 * @struct SReq_Cell
 * @brief Represents a server cell update request.
 */
typedef struct {
    Header_t header;
    uint16_t num;
    uint8_t nb;
    Cell cells[MAX_HEIGHT * MAX_WIDTH];
} SReq_Cell;

/**
 * @struct SReq_Tchat
 * @brief Represents a server chat request.
 */
typedef struct {
    Header_t header;
    uint8_t len;
    unsigned char data[MAX_DATA];
} SReq_Tchat;

/**
 * @struct SReq_End
 * @brief Represents a server end game request.
 */
typedef struct {
    Header_t header;
} SReq_End;

/**
 * @struct SReq
 * @brief Represents a server request.
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

/**
 * @struct Buf_t
 * @brief Represents a buffer for storing data.
 */
typedef struct {
    char content[MAX_BUFSIZE];
    size_t size;
} Buf_t;

/**
 * @struct UDP_Infos
 * @brief Represents information for a UDP connection.
 */
typedef struct {
    int sock_udp;
    struct sockaddr_in6 server_addr;
} UDP_Infos;

/**
 * @struct Client_Infos
 * @brief Represents information about a client.
 */
typedef struct {
    int client_tcp_sock;
    uint8_t client_id;
    time_t last_activity;
    uint16_t game_udp_port;
    player_status_t status;
} Client_Infos;

/**
 * @brief Initializes a buffer.
 * 
 * @param buffer The buffer to initialize.
 */
void initbuf(Buf_t *buffer);

/**
 * @brief Appends data to a buffer.
 * 
 * @param buffer The buffer to append to.
 * @param elt The data to append.
 * @param elt_size The size of the data.
 * @return int 0 on success, -1 on failure.
 */
int appendbuf(Buf_t *buffer, void *elt, size_t elt_size);

/**
 * @brief Copies data from a buffer.
 * 
 * @param buffer The buffer to copy from.
 * @param start The starting position in the buffer.
 * @param elt_size The size of the data to copy.
 * @param dst The destination to copy to.
 * @return int 0 on success, -1 on failure.
 */
int copyfrombuf(Buf_t *buffer, size_t *start, size_t elt_size, void *dst);

/**
 * @brief Gets the code request from a header.
 * 
 * @param header The header.
 * @return uint16_t The code request.
 */
uint16_t get_codereq(Header_t header);

/**
 * @brief Gets the ID from a header.
 * 
 * @param header The header.
 * @return uint16_t The ID.
 */
uint16_t get_id(Header_t header);

/**
 * @brief Gets the team ID from a header.
 * 
 * @param header The header.
 * @return uint16_t The team ID.
 */
uint16_t get_eq(Header_t header);

/**
 * @brief Gets the action from a message.
 * 
 * @param message The message.
 * @return uint8_t The action.
 */
uint8_t get_action(Message_t message);

/**
 * @brief Gets the number from a message.
 * 
 * @param message The message.
 * @return uint16_t The number.
 */
uint16_t get_num(Message_t message);

/**
 * @brief Debugs a client request.
 * 
 * @param client_rq The client request to debug.
 */
void debug_creq(CReq *client_rq);

/**
 * @brief Debugs a server request.
 * 
 * @param server_rq The server request to debug.
 */
void debug_sreq(SReq *server_rq);

#endif /* __REQUESTS_H__ */