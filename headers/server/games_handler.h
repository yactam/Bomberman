#ifndef __GAMES_HANDLER_H__
#define __GAMES_HANDLER_H__

#include <pthread.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include "game.h"

/**
 * @file games_handler.h
 * @brief Handles game management and player interactions.
 */

#define MAX_GAMES 100
#define NB_PLAYERS 4
#define MULTICAST_ADDR "ff12::1"

/**
 * @enum game_status_t
 * @brief Represents the status of a game.
 */
typedef enum {
    WAITING_CONNECTIONS = 0, /**< Waiting for players to connect */
    READY_TO_START,          /**< Ready to start the game */
    ON_GOING,                /**< Game is ongoing */
    GAME_OVER                /**< Game is over */
} game_status_t;

/**
 * @struct Player
 * @brief Represents a player in a game.
 */
typedef struct {
    uint8_t player_id;          /**< ID of the player */
    uint8_t game_id;            /**< ID of the game the player is in */
    player_status_t player_status; /**< Status of the player */
} Player;

/**
 * @struct Game
 * @brief Represents a game and its state.
 */
typedef struct {
    int game_id;                        /**< ID of the game */
    game_mode_t game_mode;              /**< Mode of the game */
    game_status_t game_status;          /**< Status of the game */
    int nb_players;                     /**< Number of players in the game */
    int nb_ready_players;               /**< Number of players ready to play */
    Player players[NB_PLAYERS];         /**< Array of players in the game */
    pthread_t game_thread;              /**< Thread handling the game */
    pthread_mutex_t game_mtx;           /**< Mutex for game synchronization */
    pthread_cond_t game_cond;           /**< Condition variable for game synchronization */
    uint16_t port_udp;                  /**< UDP port for the game */
    uint16_t port_multicast;            /**< Multicast port for the game */
    unsigned char addr_mdiff[16];       /**< Multicast address for the game */
    GameBoard game_board;               /**< Game board state */
    int multicast_sock;                 /**< Multicast socket */
    struct sockaddr_in6 gradr;          /**< Multicast group address */
    int clients_tcp_sockets[NB_PLAYERS]; /**< Array of TCP sockets for clients */
    SSL *ssl[NB_PLAYERS];               /**< Array of SSL contexts for clients */
} Game;

/**
 * @struct ServerGames
 * @brief Manages all games on the server.
 */
typedef struct {
    int nb_games;                      /**< Number of games on the server */
    Game games[MAX_GAMES];             /**< Array of games on the server */
    pthread_mutex_t sgames_mtx;        /**< Mutex for server games synchronization */
} ServerGames;

static unsigned char multicast_addr[16] = MULTICAST_ADDR;
static uint16_t PORTUDP = 12345;
static uint16_t PORTMULTICAST = 54321;

/**
 * @brief Initializes the server games structure.
 * 
 * @param[out] serverGames Double pointer to the ServerGames structure to initialize.
 */
void init_serverGames(ServerGames** serverGames);

/**
 * @brief Frees the memory allocated for the server games structure.
 * 
 * @param[in, out] serverGames Double pointer to the ServerGames structure to free.
 */
void free_serverGames(ServerGames** serverGames);

/**
 * @brief Adds a client to a game.
 * 
 * @param[in, out] serverGames Pointer to the ServerGames structure.
 * @param[in] game_mode The game mode.
 * @param[out] port_udp Pointer to store the allocated UDP port.
 * @param[out] port_multicast Pointer to store the allocated multicast port.
 * @param[out] addr_mdiff Pointer to store the multicast address.
 * @param[in] client_tcp_socket TCP socket for the client.
 * @param[in] ssl SSL context for the client.
 * @return int 0 on success, -1 on failure.
 */
int add_client(ServerGames** serverGames, game_mode_t game_mode, uint16_t* port_udp, uint16_t* port_multicast, char* addr_mdiff, int client_tcp_socket, SSL* ssl);

/**
 * @brief Removes a client from a game.
 * 
 * @param[in, out] serverGames Pointer to the ServerGames structure.
 * @param[in] portudp UDP port of the client.
 * @param[in] id_player ID of the player to remove.
 * @return int 0 on success, -1 on failure.
 */
int remove_client(ServerGames** serverGames, uint16_t portudp, uint8_t id_player);

/**
 * @brief Generates a multicast address for a game.
 * 
 * @return char* Pointer to the multicast address.
 */
char *generate_multicast_addr();

/**
 * @brief Generates a UDP port for a game.
 * 
 * @return uint16_t The generated UDP port.
 */
uint16_t generate_udpPort();

/**
 * @brief Generates a multicast port for a game.
 * 
 * @return uint16_t The generated multicast port.
 */
uint16_t generate_multicastPort();

/**
 * @brief Sets the status of a player in a game.
 * 
 * @param[in, out] serverGames Pointer to the ServerGames structure.
 * @param[in] port_udp UDP port of the game.
 * @param[in] id_player ID of the player.
 * @param[in] player_status The new status of the player.
 */
void set_player_status(ServerGames** serverGames, uint16_t port_udp, uint8_t id_player, player_status_t player_status);

/**
 * @brief Supervisor handler for managing game threads.
 * 
 * @param arg Argument for the handler.
 * @return void* NULL.
 */
void* games_supervisor_handler(void* arg);

/**
 * @brief Compares two game boards.
 * 
 * @param board1 The first game board.
 * @param board2 The second game board.
 * @return size_t Number of differing cells.
 */
size_t compare_boards(GameBoard board1, GameBoard board2);

/**
 * @brief Initializes player positions on the game board.
 * 
 * @param board The game board.
 * @param player_positions Array to store player positions.
 */
void init_players_positions(GameBoard board, player_pos_t *player_positions);

/**
 * @brief Processes player actions in the game.
 * 
 * @param actions Array of player actions.
 * @param nb_actions Number of actions.
 * @param player_positions Array of player positions.
 * @param bomb_infos Array of bomb information.
 * @param game Pointer to the game.
 */
void process_players_actions(PlayerAction *actions, size_t nb_actions, player_pos_t *player_positions, BombInfo *bomb_infos, Game *game);

/**
 * @brief Handles bomb explosions in the game.
 * 
 * @param bomb_infos Array of bomb information.
 * @param bomb_pos Position of the bomb.
 * @param game Pointer to the game.
 * @param player_positions Array of player positions.
 */
void handle_explosion(BombInfo *bomb_infos, size_t bomb_pos, Game *game, player_pos_t *player_positions);

/**
 * @brief Checks if the game is over.
 * 
 * @param game Pointer to the game.
 * @return int 1 if the game is over, 0 otherwise.
 */
int check_game_over(Game *game);

#endif /* __GAMES_HANDLER_H__ */