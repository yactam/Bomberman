#ifndef __GAME_H__
#define __GAME_H__
#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "requests.h"

#define MAX_WIDTH 256
#define MAX_HEIGHT 256
#define EXPLOSION_DELAY 3
#define NB_LINES_CHAT 5
#define MAX_MESS_LENGTH 255

typedef enum {
    MODE4,
    TEAMS
} game_mode_t;

typedef struct {
    size_t height;
    size_t width;
    cases_t cells[MAX_HEIGHT][MAX_WIDTH];
} GameBoard;

typedef struct {
    int x;
    int y;
} player_pos_t;

typedef struct {
    bool bomb_dropped;
    time_t explosion_time;
    int bomb_x;
    int bomb_y;
} BombInfo;

typedef struct {
    uint8_t paleyr_id;
    uint16_t num;
    action_t action;
} PlayerAction;

typedef struct {
    char data[MAX_MESS_LENGTH + 5];
    size_t length;
    bool is_sent;
} Message;

int init_board(char *filename, GameBoard *board);
void debug_board(GameBoard board);
void draw_board(GameBoard board);
void draw_tchat(Message mess);
void init_game(uint8_t, uint8_t);
void clear_bombs_explosions(GameBoard *board);

#endif


