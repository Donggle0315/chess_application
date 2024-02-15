#include <stdlib.h>

#include "wlog.h"
#include "chess_game.h"


ChessBoard *chess_create_board() {
    ChessBoard *chess_board = malloc(sizeof(*chess_board));
    return chess_board;
}
