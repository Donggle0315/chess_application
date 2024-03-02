#ifndef CHESS_GAME
#define CHESS_GAME

#include <stdbool.h>

typedef enum {
    BLANK,
    BLACK_PAWN,
    BLACK_ROOK,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_QUEEN,
    BLACK_KING,
    WHITE_PAWN,
    WHITE_ROOK,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_QUEEN,
    WHITE_KING
} PIECE;

typedef enum {
    BLACK,
    WHITE,
    NONE
} COLOR;

typedef struct {
    PIECE board[8][8];
    bool has_moved[8][8];
} ChessBoard;

ChessBoard *chess_create_board();
void chess_destroy_board(ChessBoard **board);
void chess_initialize_board(ChessBoard *board);

void set_piece(ChessBoard *board, int row, int col, PIECE piece);
PIECE get_piece(ChessBoard *board, int row, int col);
COLOR get_color(ChessBoard *board, int row, int col);

bool can_move(ChessBoard *board, int start_row, int start_col, int target_row, int target_col);
bool move(ChessBoard *board, int start_row, int start_col, int target_row, int target_col);



#endif
