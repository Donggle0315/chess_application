/*
room.c - prototypes adn definitions for room
*/

#ifndef __ROOM_H__
#define __ROOM_H__

#include "server.h"
#include "chess.h"

#define MAX_PLAYER_NUM  2
#define ARGUMENT_NUM 5

typedef struct GAME_INFORMATION{
    int player[MAX_PLAYER_NUM];
    int turn;//몇번째 플레이어가 플레이할 차례인지
}GAME_INFORMATION;

/**
 * implement : 방을 관리하는 메인함수(스레드에서 실행) : 
 * input : pool_room pointer
 * output :
*/
void* room_main(void*);

/**
 * implement : 방 정보 초기화 : 체스와 관련된 코드를 실행, GAME_INFORMATION 초기화
 * input : x
 * output : GAME_INFORMATION pointer
*/
GAME_INFORMATION* init_room();

/**
 * implement : accept 된 player를 GAME_INFORMATION에 추가, 역할 부여(p1, p2, 관전)
 * input : GAME_INFORMATION*
 * output :int 성공(TRUE)/실패(FALSE)
*/
int add_player(GAME_INFORMATION*,int,fd_set);

/**
 * implement : 플레이어의 역할을 변경
 * input :
 * output :
*/
void change_player_role(); //보류

/**
 * implement : 방 설정을 변경
 * input :
 * output :
*/
void change_room_rule(); //보류

/**
 * implement : 게임을 시작하는 함수
 * input :
 * output :
*/
void start_game(fd_set,int,int,int);

/**
 * implement : 게임이 종료된 후, 해당 방을 제거하기 위함 : room_pool에서 해당 방 제거
 * input : pool_room pointer
 * output : void
*/
void exit_room(GAME_INFORMATION*, pool_room*);

/**
 * implement : 인자로 들어온 문자열을 '\n'을 기준으로 나눠 버퍼에 저장하는 함수
 * input : 나눌 문자열(char*), 문자열을 저장할 버퍼(char*[])
 * output : 버퍼에 저장된 문자열 개수(idx)
*/
int getString(char*,char**);

/**
 * implement : 인자로 들어온 문자열을 '\n'을 기준으로 구분하여 하나의 문자열을 만드는 함수
 * input : 나눌 문자열 배열(char*[]), 문자열을 저장할 버퍼(char*)
 * output : x
*/
void makeString(char**,char*);

/**
 * implement : 정수를 문자열로 바꾸는 함수
 * input : 정수, 문자열
 * output : x
*/
void convertIntToString(int,char*);

#endif