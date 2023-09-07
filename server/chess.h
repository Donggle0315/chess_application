/*
chess.c - prototypes adn definitions for chess
*/

#ifndef __CHESS_H__
#define __CHESS_H__

#include "structures_constants.h"
#include "server.h"
#include <stdbool.h>

/* macro functions */

#define BLANK       0
#define BLACK       20
#define B_ROOK      21
#define B_KNIGHT    22
#define B_BISHOP    23
#define B_QUEEN     24
#define B_KING      25
#define B_PAWN      26
#define WHITE       30
#define W_ROOK      31
#define W_KNIGHT    32
#define W_BISHOP    33
#define W_QUEEN     34
#define W_KING      35
#define W_PAWN      36
#define PLAY_TIME   3000





/* 게임 시작 및 초기화 & 게임 종료 */
/**
 * implement : allocate chess board and initialize game states
 * input : x
 * output : ChessBoard Pointer
*/
ChessBoard* initBoard();

/**
 * implement : free ongoing game and re-start game
 * input : ChessBoard pointer to finish
 * output : ChessBoard poitner to play
*/
ChessBoard* resetGame(ChessBoard*);

/**
 * implement : de-allocate chess board to finish the game
 * input : ChessBoard pointer to de-allocate
 * output : x
*/
void finishGame(ChessBoard*);

/* 말의 이동을 확인하고 이동시키는 함수들 */
/**
 * implement : check selected pawn can move
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool handlePawn(ChessBoard*, int, int, int, int);

/**
 * implement : check selected rook can move
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool handleRook(ChessBoard*,int,int,int,int);

/**
 * implement : check selected knight can move
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool handleKnight(ChessBoard*,int,int,int,int);

/**
 * implement : check selected bishop can move
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool handleBishop(ChessBoard*,int,int,int,int);

/**
 * implement : check selected queen can move
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool handleQueen(ChessBoard*,int,int,int,int);

/**
 * implement : check selected king can move
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool handleKing(ChessBoard*,int,int,int,int);

/**
 * implement : check if selected piece can move to selected pos
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : true if it can move, else false
*/
bool canMove(ChessBoard*,int,int,int,int);

/**
 * implement : move piece from start pos to finish pos
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int), 캐슬링 옵션(bool)
 * output : if there is a death piece, then return death piece's code, else return BLANK
*/
int movePiece(ChessBoard*,int,int,int,int,bool);

/**
 * implement : addd death piece's code to the death's list
 * input : chess_board pointer, death piece's code(int)
 * output : x
*/
void addDeathPiece(ChessBoard*,int);

/* Functions which is checking chess game status */

/**
 * implement : get selected piece's color
 * input : piece's code
 * output : piece's color(BLANK,BLACK,WHITE) 
*/
int getPieceColor(int);

/**
 * implement : after move piece, check if there is a promotion, then do promotion
 * input : chess_board pointer, 도착 행(int), 도착 열(int)
 * output : x
*/
void afterMoveCheckPromotion(ChessBoard*,int,int);

/**
 * implement : check if the game finish
 * input : chess_board pointer
 * output : if game finish, then return true, else return false
*/
bool isFinish(ChessBoard*);

/**
 * implement : copy chess_board structure and return copied board pointer
 * input : target chess_board pointer
 * output : copied chess_board pointer
*/
ChessBoard* copyBoard(ChessBoard*);

/**
 * implement : restore chess_board structure
 * input : restored chess_board pointer, copied chess_board pointer
 * output : x
*/
void recoverBoard(ChessBoard*,ChessBoard*);

/**
 * implement : check if our king is checked
 * input : chess_board pointer
 * output : if checked return true, else return false
*/
bool isCheck(ChessBoard*);

/**
 * implement : get movealbe position of selected piece and insert to the list 
 * input : chess_board_pointer, 말의 행(int), 말의 열(int), Coordi list storing moveable positions, Coordi list's idx pointer
 * output : x
*/
void getMoveablePosition(ChessBoard*,int,int,Coordi*,int*);

/**
 * implement : change turn of the player, and increase turn count
 * input : chess_board pointer
 * output : x
*/
void changeTurn(ChessBoard*);


/* special rules */

/**
 * implement : not implemented
 * input : x
 * output : x
*/
void promotion();

/**
 * implement : get positions to do castling
 * input : chess_board pointer, 행(int), 열(int), Coordi list storing castling positions, Coordi list's idx pointer
 * output : x
*/
void getCanCastlingPosition(ChessBoard*,int,int,Coordi*,int*);

/**
 * implement : check if the king can do castling without check
 * input : chess_board pointer, 도착 행(int), 도착 열(int)
 * output : if king can move return true, else return false
*/
bool canCastlingWithoutCheck(ChessBoard*,int,int);

/**
 * implement : check if the piece can do en_passant
 * input : chess_board pointer, Coordi pointer storing en_passant position, last moved row, last moved col
 * output : x
*/
void canEnPassant(ChessBoard*, Coordi*,int,int);

/**
 * implement : check if there is coordinate in the list
 * input : finish_row, finish_col, list containing the coordinates, size of list
 * output : if in the list return true, else return false
*/
bool isInMoveablePosition(int,int,Coordi*,int);

#endif
