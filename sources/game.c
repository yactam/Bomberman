#include "game.h"
#include "debug.h"

WINDOW *game_window;
WINDOW *chat_window;

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
                    dprintf(2, "_");
                    break;
                case IWALL:
                    dprintf(2, "#");
                    break;
                case DWALL:
                    dprintf(2, "+");
                    break;
                case BOMB:
                    dprintf(2, "B");
                    break;
                case EXPLOSE:
                    dprintf(2, "X");
                    break;
                case PLAYER1:
                    dprintf(2, "1");
                    break;
                case PLAYER2:
                    dprintf(2, "2");
                    break;
                case PLAYER3:
                    dprintf(2, "3");
                    break;
                case PLAYER4:
                    dprintf(2, "4");
                    break;
                default:
                    dprintf(2, "?");
                    break;
            }
        }
        dprintf(2, "\n");
    }
    #endif
}

void init_game(uint8_t height, uint8_t width) {
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    timeout(0);

    game_window = newwin(height + 2, width + 2, 0, 0);
    box(game_window, 0, 0);

    chat_window = newwin(NB_LINES_CHAT, width + 2, height + 2, 0);
    box(chat_window, 0, 0);
    scrollok(chat_window, TRUE);
    refresh();
}

void draw_board(GameBoard board) {
    wclear(game_window);

    size_t x, y;

    for (x = 0; x < board.height; ++x) {
        for (y = 0; y < board.width; ++y) {
            char symbol;
            switch (board.cells[x][y]) {
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
            mvwprintw(game_window, x + 1, y + 1, "%c", symbol);
        }
    }

    box(game_window, 0, 0);
    

    wrefresh(game_window);
    
}

void draw_tchat(Message message) {
    wclear(chat_window);

    int max_x, max_y;
    getmaxyx(chat_window, max_y, max_x);

    size_t x = 0;
    size_t y = 1;

    for(size_t i = 0; i < message.len; ++i) {
        if (x >= max_x - 2) {
            x = 0;
            y++;
            if (y >= max_y - 1) { 
                wscrl(chat_window, 1);
                y = max_y - 2;
            }
        }
        mvwaddch(chat_window, y, x + 1, message.data[i]);
        x++;
    }

    box(chat_window, 0, 0);
    wrefresh(chat_window);
}

void clear_bombs_explosions(GameBoard *board) {
    for(size_t i = 0; i < board->height; ++i) {
        for (size_t j = 0; j < board->width; ++j) {
            if(board->cells[i][j] == EXPLOSE) {
                board->cells[i][j] = EMPTY;
            }
        }
        
    }
}