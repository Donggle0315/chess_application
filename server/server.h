/*
server.c - prototypes adn definitions for Chess Aplication
*/

#ifndef __SERVER_H__
#define __SERVER_H__

/* Include header files */
#include "structures_constants.h"

/**
 * implement : 
 * input : void
 * output : mysql 구조체 포인터
*/
MYSQL* initMysql();

/**
 * implement : client pool 초기화
 * input : PoolClient pointer, listenfd
 * output : int 성공(TRUE)/실패(FALSE)
*/
int initClientPool(PoolClient*, int);

/**
 * implement : room pool 초기화
 * input : PoolRoom pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int initRoomPool(PoolRoom*);

/**
 * implement : 서버에 접속한 클라이언트를 client_pool에 추가
 * input : client_pool pointer, int client_fd
 * output : void
*/
void addClientToPool(PoolClient*, int);

/**
 * implement : 들어온 buffer을 '\0'을 기준으로 나눠서 각 부분의 시작을 가리키는 포인터 배열을 만든다
 * input : strchr로 '\0'를 찾고, 이 부분에 널 문자를 넣어서 분리한다.
 * output : void
*/
void parseline(char*, char**);

/**
 * implement : 클라이언트의 요청에 따라 맞는 함수들을 호출 : LOG,REG,CRE,FET,ENT,EXT
 * input : client_pool pointer,room_pool pointer, mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int handleClient(PoolClient*, PoolRoom*, MYSQL*, char[], int, SendInfo*);

/**
 * implement : 클라이언트의 로그인
 * 클라이언트로부터 아이디/비밀번호를 받아와서 확인 -> 아이디와 비번 일치하면 유저한테 성공했다고 보내고 true 리턴 / 실패하면 유저에게 실패했다고 보내고 false 리턴
 * input : mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int userLogin(MYSQL*, PoolClient*, char**, int);

/**
 * implement : 클라이언트의 회원가입 : 클라이언트로부터 아이디/비번 받아옴 -> sql에서 중복 확인 -> 중복이 없다면 sql에 업데이트하고 클라이언트에 성공 여부 알려고 true 리턴
 * input : mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int userRegister(MYSQL*, char**);

/**
 * implement : 클라이언트의 방 만들기 및 방 설정 : addRoomToPool() 호출 -> 스레드 생성해서 room_main() 실행
 * input : room_pool pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int createRoom(PoolRoom*, char**, int, SendInfo*);

/**
 * implement : 서버에 만들어진 방을 room_pool에 추가 : room_option을 초기화 
 * input : void 
 * output : int 성공(TRUE)/실패(FALSE)
*/
int addRoomToPool(PoolRoom *, char**);

/**
 * implement : 전체 방 정보를 클라이언트로부터 받아서, room_pool에서 찾아서 클라이언트에게 전송
 * input : void
 * output : int 성공(TRUE)/실패(FALSE)
*/
int fetchInformation(PoolRoom*, SendInfo*);

/**
 * implement : 방에 들어감
 * input :
 * output :
*/
int enterRoom(PoolRoom*, int, int, int);

/**
 * implement : 클라이언트가 서버 연결을 끊는다 : client_pool에서 해당 클라이언트 정보를 삭제함, Pool 업데이트
 * input : PoolClient pointer, client_fd
 * output : int 성공(TRUE)/실패(FALSE)
*/
int exit_client(PoolClient*, int);

int openClientfd();

void deleteClientFromRoom(PoolRoom*, SendInfo*,int);


#endif
