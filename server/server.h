/*
server.c - prototypes adn definitions for Chess Aplication
*/

#ifndef __SERVER_H__
#define __SERVER_H__

/* Include header files */
#include <stdbool.h>
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
#include <sys/select.h>
#include "mysql_login_info.h"
#include "utility.h"

/* Define macro functions */
#define TMP 1024
#define PORT "12345"
#define MAX_CLIENT 100
#define MAX_ROOM 100
#define MAX_EVENTS 100
#define MAX_LEN 2048

#define TRUE 1
#define FALSE 0

#define MAX_PLAYER_NUM  2
#define PLAYER1 11
#define PLAYER2 12
#define SPECTATOR 13

#define ROW         8
#define COL         8
#define PEICE_CNT   16

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

typedef struct GAME_INFORMATION{
    int player[MAX_PLAYER_NUM];
    int turn;//몇번째 플레이어가 플레이할 차례인지
}GAME_INFORMATION;

typedef struct chess_board{
    //기본 정보들
    int board[ROW][COL];
    int board_copy[ROW][COL];
    int white_death[PEICE_CNT];
    int w_death_idx;
    int black_death[PEICE_CNT];
    int b_death_idx;
    //unsinged char** board_history;
    bool black_check;
    bool white_check;
    int player_turn;
    //special rule 변수
    int promotion_r;
    int promotion_c;
    bool castling_check[6];
    bool castling_flag;
    bool last_move[5];
    bool en_passant_flag;

    //게임 시간
    int black_time;
    int white_time;
}chess_board;

typedef struct ROOM_OPTION{
    int room_id;
    char name[50];
    int max_user_count;
    int cur_user_count;
    int time;
    int player_fd[2];
    int player_idx[2];
    GAME_INFORMATION *gi;
    chess_board *b;
} room_option;

typedef struct POOL_ROOM{
    room_option room[MAX_ROOM];
} pool_room;

typedef struct SEND_INFO{
    char send_string[MAX_LEN];
    int send_fds[4];
    int size;
} send_info;
/* Prototypes of Functions */


/**
 * implement : 
 * input : void
 * output : mysql 구조체 포인터
*/
MYSQL* init_mysql();

/**
 * implement : client pool 초기화
 * input : pool_client pointer, listenfd
 * output : int 성공(TRUE)/실패(FALSE)
*/
int init_client_pool(pool_client*, int);

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
int handle_client(pool_client*, pool_room*, MYSQL*, char[], int, send_info*);

/**
 * implement : 클라이언트의 로그인
 * 클라이언트로부터 아이디/비밀번호를 받아와서 확인 -> 아이디와 비번 일치하면 유저한테 성공했다고 보내고 true 리턴 / 실패하면 유저에게 실패했다고 보내고 false 리턴
 * input : mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int user_login(MYSQL*, pool_client*, char**, int);

/**
 * implement : 클라이언트의 회원가입 : 클라이언트로부터 아이디/비번 받아옴 -> sql에서 중복 확인 -> 중복이 없다면 sql에 업데이트하고 클라이언트에 성공 여부 알려고 true 리턴
 * input : mysql pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int user_register(MYSQL*, char**);

/**
 * implement : 클라이언트의 방 만들기 및 방 설정 : add_room_to_pool() 호출 -> 스레드 생성해서 room_main() 실행
 * input : room_pool pointer
 * output : int 성공(TRUE)/실패(FALSE)
*/
int create_room(pool_room*, char**, int, send_info*);

/**
 * implement : 서버에 만들어진 방을 room_pool에 추가 : room_option을 초기화 
 * input : void 
 * output : int 성공(TRUE)/실패(FALSE)
*/
int add_room_to_pool(pool_room *, char**);

/**
 * implement : 전체 방 정보를 클라이언트로부터 받아서, room_pool에서 찾아서 클라이언트에게 전송
 * input : void
 * output : int 성공(TRUE)/실패(FALSE)
*/
int fetch_information(pool_room*, send_info*);

/**
 * implement : 방에 들어감
 * input :
 * output :
*/
int enter_room(pool_room*, int, int);

/**
 * implement : 클라이언트가 서버 연결을 끊는다 : client_pool에서 해당 클라이언트 정보를 삭제함, Pool 업데이트
 * input : pool_client pointer, client_fd
 * output : int 성공(TRUE)/실패(FALSE)
*/
int exit_client(pool_client*, int);

int open_clientfd();

#endif