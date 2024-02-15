

#include "unity.h"
#include "fff.h"

#include "chess_game.h"


DEFINE_FFF_GLOBALS

ChessBoard *board;
void setUp(){
    board = chess_create_board();
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            set_piece(board, i, j, BLANK);
        }
    }
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_ROOK);
}

void tearDown(){
    chess_destroy_board(&board);
}

void test_rook_cannot_move_diag() {
    TEST_ASSERT_FALSE(can_move(board, 3, 6, 2, 5));
    TEST_ASSERT_FALSE(can_move(board, 4, 1, 0, 3));
}

void test_rook_move_col() {
    // Able to move if col is same
    TEST_ASSERT_TRUE(can_move(board, 3, 6, 3, 3));
    TEST_ASSERT_TRUE(can_move(board, 4, 1, 4, 6));
}

void test_rook_move_row() {

}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_rook_move_col);
    RUN_TEST(test_rook_cannot_move_diag);

    UNITY_END();
    return 0;
}
