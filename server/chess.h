/*
chess.c - prototypes adn definitions for chess
*/

#ifndef __CHESS_H__
#define __CHESS_H__

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

/* 게임 진행에 필요한 변수들을 저장하는 구조체 */
typedef struct coordi{
    int row;
    int col;
}coordi;




/* 게임 시작 및 초기화 & 게임 종료 */
/**
 * implement : 게임시작하고자 할 때, 게임에 필요한 변수들 초기화
 * input : x
 * output : chess_board Pointer
*/
chess_board* initBoard();

/**
 * implement : 지워도 될거 같음
 * input : x
 * output : x
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
 * implement : 말을 움직이는 함수로, 특수 규칙에 따라 추가적인 움직임 발생 가능
 * input : chess_board pointer, 시작 행(int), 시작 열(int), 도착 행(int), 도착 열(int), 캐슬링 옵션(bool)
 * output : 죽은 말의 코드(int) : 죽은 말이 없을 경우 BLANK 리턴
*/
int movePiece(chess_board*,int,int,int,int,bool);

/**
 * implement : 죽은 말을 리스트에 추가하는 함수
 * input : chess_board pointer, 죽은 말의 코드(int)
 * output : x
*/
void addDeathPiece(chess_board*,int);

/* 게임 진행 여부를 확인하는 함수들 */

/**
 * implement : 해당 말의 소속을 얻는 함수
 * input : 해당 말의 정보를 담은 정수
 * output : 해당 말의 소속을 담은 정수(BLANK,BLACK,WHITE) 
*/
int getPieceColor(int);

/**
 * implement : 움직이고 난 뒤, 프로모션 발생해야하는지 확인
 * input : chess_board pointer, 도착 행(int), 도착 열(int)
 * output : x
*/
void afterMove(chess_board*,int,int);

/**
 * implement : 게임이 끝나는지를 확인하는 함수
 * input : chess_board pointer
 * output : 게임이 끝나면 true, 게임이 끝나지 않으면 false 리턴
*/
bool isFinish(chess_board*);

/**
 * implement : 현재 게임 정보를 복사하여 저장하는 함수
 * input : 복사할 chess_board pointer
 * output : 복사된 chess_board pointer
*/
chess_board* copyBoard(chess_board*);

/**
 * implement : 현재 게임 정보를 복구하는 함수
 * input : 저장할 chess_board pointer, 복사될 chess_board pointer
 * output : x
*/
void recover_board(chess_board*,chess_board*);

/**
 * implement : 본인의 왕이 체크 당했는지를 확인하는 함수
 * input : chess_board pointer
 * output : 왕이 체크당한다면 true를, 체크당하지 않았다면 false
*/
bool isCheck(chess_board*);

/**
 * implement : 입력으로 들어온 좌표에 있는 말이 움직일 수 있는 좌표를 구하는 함수
 * input : chess_board_pointer, 말의 행(int), 말의 열(int), 좌표를 담을 coordi 리스트 주소, 좌표 리스트의 인덱스 주소
 * output : x
*/
void getMoveablePosition(chess_board*,int,int,coordi*,int*);



/**
 * implement : 플레이어 순서를 바꿔줌, 턴 수 1씩 증가
 * input : chess_board pointer, game_infomation pointer
 * output : x
*/
void changeTurn(chess_board*);


/* special rules */

/**
 * implement : 
 * input :
 * output :
*/
void promotion();

/**
 * implement : 캐슬링 할 수 있는 좌표를 구하는 함수
 * input : chess_board pointer, 행(int), 열(int), 캐슬링가능한 좌표를 담을 리스트 포인터, 리스트 개수를 나타내는 idx
 * output : x
*/
void castling(chess_board*,int,int,coordi*,int*);

/**
 * implement : 캐슬링 시에 왕이 해당 위치로 이동할 수 있는지 확인하는 함수
 * input : chess_board pointer, 도착 행(int), 도착 열(int)
 * output : 왕이 이동할 수 있다면 true, 이동할 수 없으면 false
*/
bool forCastling(chess_board*,int,int);

/**
 * implement : 앙파상이 가능한지를 확인해 주는 함수
 * input : chess_board pointer, 앙파상 위치를 알려주는 좌표 pointer, 마지막 이동 열, 마지막 이동 행
 * output : x
*/
void enPassant(chess_board*, coordi*,int,int);

#endif