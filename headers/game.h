#ifndef __GAME_H__
#define __GAME_H__
#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "requests.h"

#define MAX_WIDTH 256
#define MAX_HEIGHT 256

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
    uint8_t paleyr_id;
    action_t action;
} PlayerAction;

int init_board(char *filename, GameBoard *board);
void debug_board(GameBoard board);
void draw_board(GameBoard board);
void init_game();

#endif


