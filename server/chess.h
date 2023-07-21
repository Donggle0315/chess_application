/*
chess.c - prototypes adn definitions for chess
*/

#ifndef __CHESS_H__
#define __CHESS_H__

//#include "server.h"
#include <stdbool.h> //bool 자료형 사용
#include <stdio.h>
#include <stdlib.h>

/* macro functions */
#define ROW         8
#define COL         8
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
#define PEICE_CNT   16
#define PLAY_TIME   3000

/* 게임 진행에 필요한 변수들을 저장하는 구조체 */
typedef struct coordi{
    int row;
    int col;
}coordi;

typedef struct chess_board{
    //기본 정보들
    int board[ROW][COL];
    int** board_copy;
    int white_death[PEICE_CNT];
    int black_death[PEICE_CNT];
    //unsinged char** board_history;
    bool black_check;
    bool white_check;
    int player_turn;
    //special rule 변수
    int promotion_r;
    int promotion_c;
    coordi castling_check;
    bool castling_flag;
    bool last_move[4];
    bool en_passant_flag;
    //게임 시간
    int black_time;
    int white_time;
}chess_board;


/* 게임 시작 및 초기화 & 게임 종료 */
/**
 * implement : 게임시작하고자 할 때, 게임에 필요한 변수들 초기화
 * input : x
 * output : chess_board Pointer
*/
chess_board* initBoard();

/**
 * implement : 
 * input :
 * output :
*/
void gameStart();

/**
 * implement : 진행중인 게임을 초기화(free)하고 새로운 게임을 만듦(init)
 * input : 중단하고자하는 chess_board pointer
 * output : 새롭게 만들어진 chess_board poitner
*/
chess_board* resetGame(chess_board*);

/**
 * implement : 
 * input :
 * output :
*/
void undoGame();

/**
 * implement : 게임이 끝났을 때 보드판을 초기화(Free) 및 승패 여부를 알려줌
 * input : 초기화 하고자 하는 chess_board pointer
 * output : x
*/
void finishGame(chess_board*);

/* 말의 이동을 확인하고 이동시키는 함수들 */
/**
 * implement : 폰이 이동 가능한지 확인하는 함수
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool handlePawn(chess_board*,int,int,int,int);

/**
 * implement : 룩이 이동 가능한지 확인하는 함수
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool handleRook(chess_board*,int,int,int,int);

/**
 * implement : 나이트가 이동 가능한지 확인하는 함수
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool handleKnight(chess_board*,int,int,int,int);

/**
 * implement : 비숍이 이동 가능한지 확인하는 함수
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool handleBishop(chess_board*,int,int,int,int);

/**
 * implement : 퀸이 이동 가능한지 확인하는 함수 
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool handleQueen(chess_board*,int,int,int,int);

/**
 * implement : 킹이 이동 가능한지 확인하는 함수
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool handleKing(chess_board*,int,int,int,int);

/**
 * implement : 선택된 말이 해당 지점으로 움직일 수 있는지를 판단하는 함수
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int)
 * output : 움직일 수 있으면 true, 움직일 수 없으면 false
*/
bool canMove(chess_board*,int,int,int,int);

/**
 * implement : 
 * input :
 * output :
*/
void movePiece();

/* 게임 진행 여부를 확인하는 함수들 */

/**
 * implement : 해당 말의 소속을 얻는 함수
 * input : 해당 말의 정보를 담은 정수
 * output : 해당 말의 소속을 담은 정수(BLANK,BLACK,WHITE) 
*/
int getPieceColor(int);

/**
 * implement : 
 * input :
 * output :
*/
void afterMove();

/**
 * implement : 
 * input :
 * output :
*/
void isFinish();

/**
 * implement : 
 * input :
 * output :
*/
void isCheck();

/**
 * implement : 
 * input :
 * output :
*/
void getMoveablePosition();

/**
 * implement : 
 * input :
 * output :
*/
void changeTurn();


/* special rules */

/**
 * implement : 
 * input :
 * output :
*/
void promotion();

/**
 * implement : 
 * input :
 * output :
*/
void castling();

/**
 * implement : 
 * input :
 * output :
*/
void forCastling();

/**
 * implement : 
 * input :
 * output :
*/
void enPassant();

#endif __CHESS_H__