#ifndef __STRUCTURES_CONSTANTS__
#define __STRUCTURES_CONSTANTS__

/* Include header files */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <sys/select.h>
#include "mysql_login_info.h"


/* Defines */
#define PORT "12345"
#define MAX_CLIENT 100
#define MAX_ROOM 100
#define MAX_EVENTS 100
#define MAX_LEN 2048

#define MAX_PLAYER_NUM  2
#define PLAYER1 11
#define PLAYER2 12
#define SPECTATOR 13

#define ROW         8
#define COL         8
#define PIECE_CNT   16



/* Struct Definitions */

typedef struct sockaddr SA;

// Structure that stores informations about a user
typedef struct{
    char user_id[MAX_LEN];
    char username[MAX_LEN];
}UserInformation;

// Structure for managing file descriptions for clients. Used as arguments of select()
typedef struct {
    int connection_count; 
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;

    // below is information about the user in a certain index.
    int clientfd[FD_SETSIZE];
    UserInformation client_info[FD_SETSIZE];
    bool has_login[FD_SETSIZE]; 
} PoolClient;

typedef struct {
    int player[MAX_PLAYER_NUM];
    int turn;//몇번째 플레이어가 플레이할 차례인지
}GameInformation;

typedef struct {
    //기본 정보들
    int board[ROW][COL];
    int board_copy[ROW][COL];
    int white_death[PIECE_CNT];
    int w_death_idx;
    int black_death[PIECE_CNT];
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

    // Time left for each player (sec)
    int black_time;
    int white_time;
}ChessBoard;

typedef struct {
    int room_id;
    char name[50];
    int max_user_count;
    int cur_user_count;
    int time;
    int player_fd[2];
    int player_idx[2];
    GameInformation *gi;
    ChessBoard *b;
} RoomOption;

typedef struct {
    RoomOption room[MAX_ROOM];
} PoolRoom;


typedef struct {
    MYSQL *mysql;
    PoolRoom *pool_room;
    PoolClient *pool_client;
} MainArg;

typedef struct {
    char send_string[MAX_LEN];
    int send_fds[4];
    int size;
} SendInfo;

typedef struct {
    int row;
    int col;
}Coordinate;


#endif
