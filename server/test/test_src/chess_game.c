#include "chess_game.h"
#include "fff.h"
#include "unity.h"
#include "unity_internals.h"

DEFINE_FFF_GLOBALS

ChessBoard *board;
void setUp() {
    board = chess_create_board();
    chess_initialize_board(board);
}

void tearDown() {
    chess_destroy_board(&board);
}

void test_create_destroy_board() {
    ChessBoard *b = chess_create_board();
    TEST_ASSERT_NOT_NULL(b);
    chess_destroy_board(&b);
    TEST_ASSERT_NULL(b);
}

void test_get_set_piece() {
    ChessBoard *b = chess_create_board();
    set_piece(b, 0, 0, BLACK_KING);
    set_piece(b, 4, 5, WHITE_KING);
    TEST_ASSERT_EQUAL(BLACK_KING, get_piece(b, 0, 0));
    TEST_ASSERT_EQUAL(WHITE_KING, get_piece(b, 4, 5));
}

void test_initialize_board() {
    ChessBoard *b = chess_create_board();

    chess_initialize_board(b);

    TEST_ASSERT_EQUAL(get_piece(b, 0, 0), BLACK_ROOK);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 1), BLACK_KNIGHT);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 2), BLACK_BISHOP);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 3), BLACK_QUEEN);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 4), BLACK_KING);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 5), BLACK_BISHOP);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 6), BLACK_KNIGHT);
    TEST_ASSERT_EQUAL(get_piece(b, 0, 7), BLACK_ROOK);
    for(int i=0; i<8; i++){
        TEST_ASSERT_EQUAL(get_piece(b, 1, i), BLACK_PAWN);
    }

    TEST_ASSERT_EQUAL(get_piece(b, 7, 0), WHITE_ROOK);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 1), WHITE_KNIGHT);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 2), WHITE_BISHOP);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 3), WHITE_QUEEN);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 4), WHITE_KING);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 5), WHITE_BISHOP);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 6), WHITE_KNIGHT);
    TEST_ASSERT_EQUAL(get_piece(b, 7, 7), WHITE_ROOK);
    for(int i=0; i<8; i++){
        TEST_ASSERT_EQUAL(get_piece(b, 6, i), WHITE_PAWN);
    }

    for(int i=2; i<6; i++){
        for(int j=0; j<8; j++){
            TEST_ASSERT_EQUAL(get_piece(b, i, j), BLANK);
        }
    }
}


void test_can_move_rook() {
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            set_piece(board, i, j, BLANK);
        }
    }

    set_piece(board, 3, 6, BLACK_ROOK);
    set_piece(board, 4, 1, WHITE_ROOK);

    // Able to move if row or col is same
    TEST_ASSERT_TRUE(can_move(board, 3, 6, 3, 3));
    TEST_ASSERT_TRUE(can_move(board, 4, 1, 4, 6));

    // Unable to move if both row and col isn't same
    TEST_ASSERT_FALSE(can_move(board, 3, 6, 2, 7));
    TEST_ASSERT_FALSE(can_move(board, 4, 1, 2, 5));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_destroy_board);
    RUN_TEST(test_get_set_piece);
    RUN_TEST(test_initialize_board);

    UNITY_END();
}

/*
 * List Of Things to Implement (Test)
 *   --Create Board, Destroy Board
 *   --Initialize Board
 *   --Get Piece
 *   Can move Rook
 *   Can move Knight
 *   Can move Bishop
 *   Can move Queen
 *   Can move King
 *   Can move Pawn (Black)
 *   Can move Pawn (White)
 *   Check if can move pieces around
 *   Move pieces around
 *   Catch pices
 *   Check
 *   Checkmate
 */