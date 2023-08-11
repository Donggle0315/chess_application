/*
room.c - prototypes adn definitions for room
*/

#ifndef __ROOM_H__
#define __ROOM_H__

#define MAX_PLAYER_NUM  2
#define ARGUMENT_NUM 5
#define TUR 1
#define SEL 2
#define MOV 3
#define ERR 4

#include "server.h"
#include "chess.h"





/**
 * implement : 방을 관리하는 메인함수(스레드에서 실행) : 
 * input : pool_room pointer
 * output :
*/
int room_main(pool_room*, char**, int, send_info*);

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
void start_game(GAME_INFORMATION*,fd_set,int,int,int);

/**
 * implement : GAME_INFORMATION에 저장된 턴수를 증가시키는 함수
 * input : GAME_INFORMATION pointer
 * output : void
*/
void increaseTurnCnt(GAME_INFORMATION*);

/**
 * implement : 게임이 종료된 후, 해당 방을 제거하기 위함 : room_pool에서 해당 방 제거
 * input : pool_room pointer
 * output : void
*/
void exit_room(GAME_INFORMATION*, pool_room*);

/**
 * implement : 현재 플레이해야하는 클라이언트의 fd를 알려주는 함수
 * input : GAME_INFORMATION pointer, p1fd, p2fd
 * output : 현재 턴인 플레이어 fd
*/
int getNowPlayer(GAME_INFORMATION*, int, int);

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
 * implement : TUR의 정보를 보내주는 함수(턴 수, 보드 정보)
 * input : 게임정보, 보드 정보, p1fd, p2fd
 * output : x
*/
void sendInfoToClient(GAME_INFORMATION*,chess_board*,int,int);

/**
 * implement : SEL의 정보를 보내주는 함수(턴 수, 이동 가능한 좌표)
 * input : 게임정보, 좌표 정보, p1fd, p2fd
 * output : x
*/
void sendMoveableToClient(GAME_INFORMATION*,coordi*,int,int,int);

/**
 * implement : MOV의 정보를 보내주는 함수(턴 수, 이동 여부) -> 이동 성공시 1, 실패시 0을 버퍼에 저장
 * input : 게임정보, 이동 성공 여부(bool), p1fd, p2fd
 * output : x
*/
void sendIsMoveToClient(GAME_INFORMATION*,bool,int,int);

/**
 * implement : FIN의 정보를 보내주는 함수(이긴 사람 여부)
 * input : 보드 정보, p1fd, p2fd
 * output : x
*/
void sendFinishToClient(GAME_INFORMATION*,chess_board*,int,int);

/**
 * implement : 버퍼에 저장된 좌표 정보를 얻어오는 함수 / sr,sc만 버퍼에 있다면 fr,fc는 -1로 초기화
 * input : len, buf, sr, sc, fr, fc
 * output : x
*/
void getCoordinate(int,char*,int*,int*,int*,int*);

/**
 * implement : 버퍼에 저장된 함수가 원하는 명령어, 턴수에 맞는지 확인하는 함수
 * input : buf, turn(int), cmd(int)
 * output : 다 충족하면 true, 아니면 false
*/
bool checkCMD(char*,int,int);
#endif