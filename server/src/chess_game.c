#include <stdlib.h>

#include "chess_game.h"
#include "wlog.h"

ChessBoard *chess_create_board() {
    ChessBoard *chess_board = malloc(sizeof(*chess_board));
    DEBUG("Create ChessBoard: %p", chess_board);
    return chess_board;
}

void chess_destroy_board(ChessBoard **board) {
    DEBUG("Destroy ChessBoard: %p", board);
    free(*board);
    *board = NULL;
}

void chess_initialize_board(ChessBoard *board) {
    set_piece(board, 0, 0, BLACK_ROOK);
    set_piece(board, 0, 1, BLACK_KNIGHT);
    set_piece(board, 0, 2, BLACK_BISHOP);
    set_piece(board, 0, 3, BLACK_QUEEN);
    set_piece(board, 0, 4, BLACK_KING);
    set_piece(board, 0, 5, BLACK_BISHOP);
    set_piece(board, 0, 6, BLACK_KNIGHT);
    set_piece(board, 0, 7, BLACK_ROOK);
    for (int i = 0; i < 8; i++) {
        set_piece(board, 1, i, BLACK_PAWN);
    }

    set_piece(board, 7, 0, WHITE_ROOK);
    set_piece(board, 7, 1, WHITE_KNIGHT);
    set_piece(board, 7, 2, WHITE_BISHOP);
    set_piece(board, 7, 3, WHITE_QUEEN);
    set_piece(board, 7, 4, WHITE_KING);
    set_piece(board, 7, 5, WHITE_BISHOP);
    set_piece(board, 7, 6, WHITE_KNIGHT);
    set_piece(board, 7, 7, WHITE_ROOK);
    for (int i = 0; i < 8; i++) {
        set_piece(board, 6, i, WHITE_PAWN);
    }

    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            set_piece(board, i, j, BLANK);
        }
    }
}

void set_piece(ChessBoard *board, int row, int col, PIECE piece) {
    DEBUG("set (%d, %d) : %d", row, col, piece);
    board->board[row][col] = piece;
}

PIECE get_piece(ChessBoard *board, int row, int col) {
    DEBUG("get (%d, %d) : %d", row, col, piece);
    return board->board[row][col];
}

bool can_move(ChessBoard *board, int sr, int sc, int tr, int tc) {
    if (sr == tr || sc == tc){
        return true;
    }
    return false;
}
