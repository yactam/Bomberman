#include "game.h"

int init_board(char *filename, GameBoard *board) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", filename);
        return -1;
    }

    if (fscanf(file, "%zu %zu", &(board->height), &(board->width)) != 2) {
        fprintf(stderr, "Erreur lors de la lecture des dimensions du tableau\n");
        fclose(file);
        return -1;
    }

    if (board->height > MAX_HEIGHT || board->width > MAX_WIDTH) {
        fprintf(stderr, "Dimensions du tableau trop grandes\n");
        fclose(file);
        return -1;
    }

    for (size_t i = 0; i < board->height; i++) {
        for (size_t j = 0; j < board->width; j++) {
            int cell_value;
            if (fscanf(file, "%d", &cell_value) != 1) {
                fprintf(stderr, "Erreur lors de la lecture des données du tableau\n");
                fclose(file);
                return -1;
            }
            board->cells[i][j] = (cases_t) cell_value;
        }
    }

    fclose(file);
    return 0;
}

void debug_board(GameBoard board) {
    #ifndef NDEBUG
    for (size_t i = 0; i < board.height; ++i) {
        for (size_t j = 0; j < board.width; ++j) {
            switch (board.cells[i][j]) {
                case EMPTY:
                    printf("_");
                    break;
                case IWALL:
                    printf("#");
                    break;
                case DWALL:
                    printf("+");
                    break;
                case BOMB:
                    printf("B");
                    break;
                case EXPLOSE:
                    printf("X");
                    break;
                case PLAYER1:
                    printf("1");
                    break;
                case PLAYER2:
                    printf("2");
                    break;
                case PLAYER3:
                    printf("3");
                    break;
                case PLAYER4:
                    printf("4");
                    break;
                default:
                    printf("?");
                    break;
            }
        }
        printf("\n");
    }
    #endif
}

void init_game() {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(100);
}

void draw_board(GameBoard board) {
    clear();

    for (size_t i = 0; i < board.height; ++i) {
        for (size_t j = 0; j < board.width; ++j) {
            char symbol;
            switch (board.cells[i][j]) {
                case EMPTY:
                    symbol = ' ';
                    break;
                case IWALL:
                    symbol = '#';
                    break;
                case DWALL:
                    symbol = '+';
                    break;
                case BOMB:
                    symbol = 'B';
                    break;
                case EXPLOSE:
                    symbol = 'X';
                    break;
                case PLAYER1:
                    symbol = '1';
                    break;
                case PLAYER2:
                    symbol = '2';
                    break;
                case PLAYER3:
                    symbol = '3';
                    break;
                case PLAYER4:
                    symbol = '4';
                    break;
                default:
                    symbol = '?';
                    break;
            }
            mvprintw(i, j, "%c", symbol);
        }
    }

    refresh();
}