#ifndef __GAME_H__
#define __GAME_H__

#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "requests.h"

/**
 * @file game.h
 * @brief Game-related data structures and functions for managing the game state.
 */

#define MAX_WIDTH 256
#define MAX_HEIGHT 256
#define NB_LINES_CHAT 5
#define MAX_MESS_LENGTH 255

/**
 * @brief Global variable representing the delay before a bomb explodes.
 */
extern int EXPLOSION_DELAY;

/**
 * @brief Enumeration of game modes.
 */
typedef enum {
    MODE4, /**< Four-player mode */
    TEAMS  /**< Team-based mode */
} game_mode_t;

/**
 * @brief Structure representing the game board.
 */
typedef struct {
    size_t height; /**< Height of the game board */
    size_t width;  /**< Width of the game board */
    cases_t cells[MAX_HEIGHT][MAX_WIDTH]; /**< Cells of the game board */
} GameBoard;

/**
 * @brief Structure representing the position of a player.
 */
typedef struct {
    int x; /**< X-coordinate of the player */
    int y; /**< Y-coordinate of the player */
} player_pos_t;

/**
 * @brief Structure representing bomb information.
 */
typedef struct {
    bool bomb_dropped;     /**< Flag indicating if a bomb has been dropped */
    time_t explosion_time; /**< Time when the bomb will explode */
    int bomb_x;            /**< X-coordinate of the bomb */
    int bomb_y;            /**< Y-coordinate of the bomb */
} BombInfo;

/**
 * @brief Structure representing a player's action.
 */
typedef struct {
    uint8_t player_id; /**< ID of the player */
    uint16_t num;      /**< Sequence number of the action */
    action_t action;   /**< Action performed by the player */
} PlayerAction;

/**
 * @brief Structure representing a chat message.
 */
typedef struct {
    char data[MAX_MESS_LENGTH + 5]; /**< Message data */
    size_t length;                  /**< Length of the message */
    bool is_sent;                   /**< Flag indicating if the message has been sent */
} Message;

/**
 * @brief Initializes the game board from a file.
 * 
 * @param filename The name of the file containing the board data.
 * @param board The game board to initialize.
 * @return int 0 on success, -1 on failure.
 */
int init_board(char *filename, GameBoard *board);

/**
 * @brief Prints the game board to the debug output.
 * 
 * @param board The game board to print.
 */
void debug_board(GameBoard board);

/**
 * @brief Draws the game board on the screen using ncurses.
 * 
 * @param board The game board to draw.
 */
void draw_board(GameBoard board);

/**
 * @brief Draws the chat messages on the screen using ncurses.
 * 
 * @param mess The chat message to draw.
 */
void draw_tchat(Message mess);

/**
 * @brief Initializes the game state.
 * 
 * @param height The height of the game board.
 * @param width The width of the game board.
 */
void init_game(uint8_t height, uint8_t width);

/**
 * @brief Clears the bombs and explosions from the game board.
 * 
 * @param board The game board to clear.
 */
void clear_bombs_explosions(GameBoard *board);

#endif /* __GAME_H__ */