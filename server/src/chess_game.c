#include <math.h>
#include <stdlib.h>

#include "chess_game.h"
#include "wlog.h"

bool can_move_rook(ChessBoard *board, int sr, int sc, int tr, int tc);
bool can_move_knight(int sr, int sc, int tr, int tc);

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

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board->has_moved[i][j] = false;
        }
    }
}

void set_piece(ChessBoard *board, int row, int col, PIECE piece) {
    DEBUG("set (%d, %d) : %d", row, col, piece);
    board->board[row][col] = piece;
}

PIECE get_piece(ChessBoard *board, int row, int col) {
    PIECE piece = board->board[row][col];
    DEBUG("get (%d, %d) : %d", row, col, piece);
    return piece;
}

bool has_moved(ChessBoard *board, int row, int col) {
    return board->has_moved[row][col];
}

void set_has_moved(ChessBoard *board, int row, int col) {
    board->has_moved[row][col] = true;
}

COLOR get_color(ChessBoard *board, int row, int col) {
    COLOR color = WHITE;
    switch (get_piece(board, row, col)) {
    case BLACK_ROOK:
    case BLACK_KNIGHT:
    case BLACK_BISHOP:
    case BLACK_QUEEN:
    case BLACK_KING:
    case BLACK_PAWN:
        color = BLACK;
        break;
    case WHITE_ROOK:
    case WHITE_KNIGHT:
    case WHITE_BISHOP:
    case WHITE_QUEEN:
    case WHITE_KING:
    case WHITE_PAWN:
        color = WHITE;
        break;
    case BLANK:
        color = NONE;
    }

    DEBUG("get (%d, %d) : %d", row, col, color);
    return color;
}

bool is_blocked_in_between(ChessBoard *board, int sr, int sc, int tr, int tc) {
    int dr = (sr < tr) ? 1 : -1;
    int dc = (sc < tc) ? 1 : -1;
    if (sr == tr) {
        dr = 0;
    } else if (sc == tc) {
        dc = 0;
    }

    sr += dr;
    sc += dc;
    while (sr != tr || sc != tc) {
        if (get_piece(board, sr, sc) != BLANK) {
            return true;
        }
        sr += dr;
        sc += dc;
    }

    return false;
}

bool can_move_common(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // Can't move Out of bounds
    if (sr < 0 || sr >= 8 || sc < 0 || sr >= 8 || tr < 0 || tr >= 8 || tc < 0 ||
        tc >= 8) {
        DEBUG("Out of bounds");
        return false;
    }

    // Can't move on same place
    if (sr == tr && sc == tc) {
        DEBUG("Same place");
        return false;
    }

    // Can't move if same color
    if (get_color(board, sr, sc) == get_color(board, tr, tc)) {
        DEBUG("Same color");
        return false;
    }

    PIECE target_piece = get_piece(board, tr, tc);
    if (target_piece == WHITE_KING || target_piece == BLACK_KING) {
        DEBUG("Can't catch King");
        return false;
    }

    return true;
}

bool can_move_rook(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // move diagonal
    if (sr != tr && sc != tc) {
        DEBUG("Move Diagonal");
        return false;
    }

    if (is_blocked_in_between(board, sr, sc, tr, tc)) {
        DEBUG("Something blocked Rook")
        return false;
    }

    return true;
}

bool can_move_knight(int sr, int sc, int tr, int tc) {
    if ((abs(sr - tr) == 2 && abs(sc - tc) == 1) ||
        (abs(sr - tr) == 1 && abs(sc - tc) == 2)) {
        return true;
    }

    return false;
}

bool can_move_bishop(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // not diagonal
    if (abs(sr - tr) != abs(sc - tc)) {
        DEBUG("Not diagonal for bishop")
        return false;
    }

    if (is_blocked_in_between(board, sr, sc, tr, tc)) {
        DEBUG("Something blocked Bishop")
        return false;
    }

    return true;
}

bool can_move_queen(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // not horizontal/vertical or diagonal
    if (sr != tr && sc != tc && abs(sr - tr) != abs(sc - tc)) {
        return false;
    }

    if (is_blocked_in_between(board, sr, sc, tr, tc)) {
        DEBUG("Something blocked Queen")
        return false;
    }

    return true;
}

bool can_move_king(ChessBoard *board, int sr, int sc, int tr, int tc) {
    if (abs(sr - tr) <= 1 && abs(sc - tc) <= 1) {
        return true;
    }
    return false;
}

bool can_move_pawn_black(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // normal move
    if ((sc == tc && sr + 1 == tr) ||
        (!has_moved(board, sr, sc) && sc == tc && sr + 2 == tr)) {
        // something blocked in the way
        if(get_piece(board, tr, tc) != BLANK) {
            return false;
        }

        DEBUG("Pawn normal can move");
        return true;
    }
    // if can catch, can move
    else if (abs(sc - tc) == 1 && sr + 1 == tr &&
             get_color(board, tr, tc) == WHITE) {
        DEBUG("Pawn catch can move");
        return true;
    }
    return false;
}

bool can_move_pawn_white(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // normal move
    if ((sc == tc && sr - 1 == tr) ||
        (!has_moved(board, sr, sc) && sc == tc && sr - 2 == tr)) {
        // something blocked in the way
        if(get_piece(board, tr, tc) != BLANK) {
            return false;
        }

        DEBUG("Pawn normal can move");
        return true;
    }
    // if can catch, can move
    else if (abs(sc - tc) == 1 && sr - 1 == tr &&
             get_color(board, tr, tc) == BLACK) {
        DEBUG("Pawn catch can move");
        return true;
    }

    return false;
}

bool can_move(ChessBoard *board, int sr, int sc, int tr, int tc) {
    // check common reasons for not being able to move
    if (!can_move_common(board, sr, sc, tr, tc)) {
        return false;
    }

    PIECE cur_piece = get_piece(board, sr, sc);
    DEBUG("Cur Piece: %d (%d, %d) -> (%d, %d)", cur_piece, sr, sc, tr, tc);

    switch (cur_piece) {
    case BLACK_ROOK:
    case WHITE_ROOK:
        return can_move_rook(board, sr, sc, tr, tc);
    case BLACK_KNIGHT:
    case WHITE_KNIGHT:
        return can_move_knight(sr, sc, tr, tc);
    case BLACK_BISHOP:
    case WHITE_BISHOP:
        return can_move_bishop(board, sr, sc, tr, tc);
    case BLACK_QUEEN:
    case WHITE_QUEEN:
        return can_move_queen(board, sr, sc, tr, tc);
    case BLACK_KING:
    case WHITE_KING:
        return can_move_king(board, sr, sc, tr, tc);
    case BLACK_PAWN:
        return can_move_pawn_black(board, sr, sc, tr, tc);
    case WHITE_PAWN:
        return can_move_pawn_white(board, sr, sc, tr, tc);
    case BLANK:
        return false;
    }

    return false;
}

bool move(ChessBoard *board, int sr, int sc, int tr, int tc) {
    if (!can_move(board, sr, sc, tr, tc)) {
        return false;
    }

    // Move the piece
    set_piece(board, tr, tc, get_piece(board, sr, sc));
    set_piece(board, sr, sc, BLANK);
    set_has_moved(board, sr, sc);
    set_has_moved(board, tr, tc);

    return true;
}
