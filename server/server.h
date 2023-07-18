/*
server.c - prototypes adn definitions for Chess Aplication
*/

#ifndef __SERVER_H__
#define __SERVER_H__

/* Include header files */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>

/* Define macro functions */
#define TMP 1024
#define PORT 12345
#define MAX_CLIENT 100
#define MAX_ROOM 100
#define MAX_EVENTS 100
#define MAX_LEN 2048

#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PW "wasdup@1017"
#define MYSQL_DB "chess_db"
#define MYSQL_PORT 23456

#define TRUE 1
#define FALSE 0

#define PLAYER1 11
#define PLAYER2 12
#define SPECTATOR 13

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

typedef struct user_information{
    char user_id[MAX_LEN];
}user_information;

typedef struct POOL_CLIENT{
    int conn_count;
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    user_information client_info[FD_SETSIZE];
    int has_login[FD_SETSIZE];//로그인 되어있으면 FALSE, 없으면 TRUE
} pool_client;

typedef struct ROOM_OPTION{
    int room_id;
    int roomfd;
    char name[50];
    int max_user_count;
    int cur_user_count;
    int time;
    struct sockaddr_storage address;
} room_option;

typedef struct POOL_ROOM{
    room_option room[MAX_ROOM];
    sem_t mutex;
} pool_room;

/* Prototypes of Functions */


/**
 * implement : 
 * input : void
 * output : mysql 구조체 포인터
*/
MYSQL* init_mysql();

/**
 * implement : client pool 초기화
 * input : pool_client pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int init_client_pool(pool_client*);

/**
 * implement : room pool 초기화
 * input : pool_room pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int init_room_pool(pool_room*);

/**
 * implement : 서버에 접속한 클라이언트를 client_pool에 추가
 * input : client_pool pointer, int client_fd
 * output : void
*/
void add_client_to_pool(pool_client*, int);

/**
 * implement : 클라이언트의 요청에 따라 맞는 함수들을 호출 : LOG,REG,CRE,FET,ENT,EXT
 * input : client_pool pointer,room_pool pointer, mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int handle_client(pool_client*, pool_room*, MYSQL*);

/**
 * implement : 클라이언트의 로그인
 * 클라이언트로부터 아이디/비밀번호를 받아와서 확인 -> 아이디와 비번 일치하면 유저한테 성공했다고 보내고 true 리턴 / 실패하면 유저에게 실패했다고 보내고 false 리턴
 * input : mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int client_login(MYSQL*);

/**
 * implement : 클라이언트의 회원가입 : 클라이언트로부터 아이디/비번 받아옴 -> sql에서 중복 확인 -> 중복이 없다면 sql에 업데이트하고 클라이언트에 성공 여부 알려고 true 리턴
 * input : mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int client_register(MYSQL*);

/**
 * implement : 클라이언트의 방 만들기 및 방 설정 : add_room_to_pool() 호출 -> 스레드 생성해서 room_main() 실행
 * input : room_pool pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int create_room(pool_room*);

/**
 * implement : 서버에 만들어진 방을 room_pool에 추가 : room_option을 초기화 
 * input : void 
 * output : int 성공(TRUE)/실패(FALSE)
*/
int add_room_to_pool();

/**
 * implement : 전체 방 정보를 클라이언트로부터 받아서, room_pool에서 찾아서 클라이언트에게 전송
 * input : void
 * output : int 성공(TRUE)/실패(FALSE)
*/
int fetch_information();

/**
 * implement : 방에 들어감
 * input :
 * output :
*/
void enter_room(); //보류

/**
 * implement : 클라이언트가 서버 연결을 끊는다 : client_pool에서 해당 클라이언트 정보를 삭제함, Pool 업데이트
 * input : pool_client pointer, client_fd
 * output : int 성공(TRUE)/실패(FALSE)
*/
int exit_client(pool_client*, int);


#endif __SERVER_H__