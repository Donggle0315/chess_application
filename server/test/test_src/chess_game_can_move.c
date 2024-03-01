

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
}

void tearDown(){
    chess_destroy_board(&board);
}


void test_rook_move_col() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_ROOK);
    TEST_ASSERT_TRUE(can_move(board, 3, 6, 3, 3));
    TEST_ASSERT_TRUE(can_move(board, 4, 1, 4, 6));
}

void test_rook_move_row() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_ROOK);
    TEST_ASSERT_TRUE(can_move(board, 3, 6, 7, 6));
    TEST_ASSERT_TRUE(can_move(board, 4, 1, 2, 1));
}


void test_rook_cannot_move_diag() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_ROOK);
    TEST_ASSERT_FALSE(can_move(board, 3, 6, 2, 5));
    TEST_ASSERT_FALSE(can_move(board, 4, 1, 0, 3));
}

void test_rook_cannot_move_blocked() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 3, 2, BLACK_PAWN);
    TEST_ASSERT_FALSE(can_move(board, 3, 6, 3, 0));
}

void test_knight_move() {
    set_piece(board, 3, 3, WHITE_KNIGHT);
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 5, 4));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 5, 2));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 1, 4));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 1, 2));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 2, 1));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 4, 1));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 2, 5));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 4, 5));

    set_piece(board, 3, 3, BLACK_KNIGHT);
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 5, 4));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 5, 2));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 1, 4));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 1, 2));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 2, 1));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 4, 1));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 2, 5));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 4, 5));
}

void test_knight_cannot_move() {
    set_piece(board, 3, 3, BLACK_KNIGHT);
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 4, 4));
}

void test_bishop_can_move_diagonal() {
    set_piece(board, 3, 3, BLACK_BISHOP);
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 5, 5));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 1, 1));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 5, 1));
}


void test_bishop_cannot_move_not_diagonal() {
    set_piece(board, 3, 3, WHITE_BISHOP);
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 2, 5));
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 4, 6));
}

void test_bishop_cannot_move_blocked() {
    set_piece(board, 3, 3, WHITE_BISHOP);
    set_piece(board, 2, 4, WHITE_PAWN);
    set_piece(board, 5, 5, BLACK_PAWN);
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 1, 5));
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 6, 6));
}

void test_queen_move_col() {
    set_piece(board, 3, 3, BLACK_QUEEN);
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 3, 6));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 3, 1));
}

void test_queen_move_row() {
    set_piece(board, 3, 3, WHITE_QUEEN);
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 1, 3));
    TEST_ASSERT_TRUE(can_move(board, 3, 3, 7, 3));
}

void test_queen_cannot_move() {
    set_piece(board, 3, 3, WHITE_QUEEN);
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 1, 4));
    TEST_ASSERT_FALSE(can_move(board, 3, 3, 5, 4));
}

void test_blank_cannot_move() {
    set_piece(board, 0, 0, BLANK);
    TEST_ASSERT_FALSE(can_move(board, 0, 0, 1, 0));
}

void test_all_cannot_move_oob() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_KNIGHT);
    TEST_ASSERT_FALSE(can_move(board, 3, 6, 0, 9));
    TEST_ASSERT_FALSE(can_move(board, 4, 1, -1, 6));
}

void test_all_cannot_move_to_itself() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_ROOK);
    TEST_ASSERT_FALSE(can_move(board, 3, 6, 3, 6));
    TEST_ASSERT_FALSE(can_move(board, 4, 1, 4, 1));
}

void test_all_cannot_move_same_color() {
    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 3, 2, BLACK_PAWN);

    set_piece(board, 4, 1, WHITE_ROOK);
    set_piece(board, 4, 7, WHITE_PAWN);

    TEST_ASSERT_FALSE(can_move(board, 3, 6, 3, 2));
    TEST_ASSERT_FALSE(can_move(board, 4, 1, 4, 7));
}


int main(void) {
    UNITY_BEGIN();

    // rook
    RUN_TEST(test_rook_move_col);
    RUN_TEST(test_rook_move_row);
    RUN_TEST(test_rook_cannot_move_diag);
    RUN_TEST(test_rook_cannot_move_blocked);

    // knight
    RUN_TEST(test_knight_move);
    RUN_TEST(test_knight_cannot_move);

    // bishop
    RUN_TEST(test_bishop_can_move_diagonal);
    RUN_TEST(test_bishop_cannot_move_not_diagonal);
    RUN_TEST(test_bishop_cannot_move_blocked);

    // queen
    RUN_TEST(test_queen_move_col);
    RUN_TEST(test_queen_move_row);
    RUN_TEST(test_queen_cannot_move);

    // blank
    RUN_TEST(test_blank_cannot_move);

    // all
    RUN_TEST(test_all_cannot_move_to_itself);
    RUN_TEST(test_all_cannot_move_oob);
    RUN_TEST(test_all_cannot_move_same_color);

    return UNITY_END();
}
