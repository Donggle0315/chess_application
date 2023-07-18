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
#include <sys/epoll.h>
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

/* Simplifies calls to bind(), connect(), and accept() */
typedef struct sockaddr SA;

typedef struct POOL_CLIENT{

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
 * implement : 서버 초기화
 * input :
 * output :
*/
void init_program(void);

/**
 * implement : DB와 연결
 * input :
 * output :
*/
void connect_DB(void);

/**
 * implement : client pool 초기화, 서버에 접속한 모든 클라이언트 정보 저장
 * input :
 * output :
*/
void init_client_pool(void);

/**
 * implement : room pool 초기화, 서버에 만들어진 모든 방 정보 저장
 * input :
 * output :
*/
void init_room_pool(void);

/**
 * implement : 방을 만드는 클라이언트 확인
 * input :
 * output :
*/
void room_maker_epoll(void);

/**
 * implement : 서버에 접속한 클라이언트를 client_pool에 추가
 * input :
 * output :
*/
void add_client_to_pool();

/**
 * implement : 클라이언트의 요청에 따라 맞는 함수들을 호출
 * input :
 * output :
*/
void handle_client();

/**
 * implement : 클라이언트의 로그인
 * input :
 * output :
*/
void client_login();

/**
 * implement : 클라이언트의 회원가입
 * input :
 * output :
*/
void client_register();

/**
 * implement : 클라이언트의 방 만들기 및 방 설정
 * input :
 * output :
*/
void create_room();

/**
 * implement : 서버에 만들어진 방을 room_pool에 추가
 * input :
 * output :
*/
void add_room_to_pool();

/**
 * implement : 요청된 방 정보를 room_pool에서 가지고 옴
 * input :
 * output :
*/
void fetch_information();

/**
 * implement : 방에 들어감
 * input :
 * output :
*/
void enter_room();

/**
 * implement : 클라이언트가 서버 연결을 끊는다
 * input :
 * output :
*/
void exit_clien();

/**
 * implement : 방을 관리하는 메인함수
 * input :
 * output :
*/
void room_main();

/**
 * implement : 방 정보 초기화
 * input :
 * output :
*/
void init_room();

/**
 * implement : 방에 들어오는 클라이언트와의 통신을 위한 함수
 * input :
 * output :
*/
void room_enter_epoll();

/**
 * implement : accept 된 player를 player_pool에 추가, 역할 부여(p1, p2)
 * input :
 * output :
*/
void add_player();

/**
 * implement : 플레이어의 역할을 변경
 * input :
 * output :
*/
void change_player_role();

/**
 * implement : 방 설정을 변경
 * input :
 * output :
*/
void change_room_rule();

/**
 * implement : 게임을 시작하는 함수
 * input :
 * output :
*/
void start_game();

/**
 * implement : 게임이 종료된 후, 해당 방을 제거하기 위함
 * input :
 * output :
*/
void exit_room();



#endif __SERVER_H__