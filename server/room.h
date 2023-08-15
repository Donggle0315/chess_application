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
 * implement : 
 * input : 
 * output :
*/
int room_main(pool_room*, char**, int, send_info*);

/**
 * implement : 
 * input : 
 * output :
*/
void start_game(room_option*, send_info*);

/**
 * implement : 
 * input : 
 * output :
*/
void handle_SEL(room_option*,send_info*,char**);

/**
 * implement : 
 * input : 
 * output :
*/
void handle_MOV(room_option*, send_info*,char**);

/**
 * implement : 
 * input : 
 * output :
*/
GAME_INFORMATION* init_room();

/**
 * implement : 
 * input : 
 * output :
*/
void increaseTurnCnt(GAME_INFORMATION*);

/**
 * implement : 
 * input : 
 * output :
*/
void exit_room(GAME_INFORMATION*,pool_room*);

/**
 * implement : 
 * input : 
 * output :
*/
void sendInfoToClient(room_option*,send_info*);

/**
 * implement : 
 * input : 
 * output :
*/
void sendMoveableToClient(room_option*, send_info*,coordi*, int);

/**
 * implement : 
 * input : 
 * output :
*/
void sendIsMoveToClient(room_option*, send_info*,bool,bool);

/**
 * implement : 
 * input : 
 * output :
*/
int add_player(GAME_INFORMATION*,int,fd_set);

/**
 * implement : 
 * input : 
 * output :
*/
void change_player_role();

/**
 * implement : 
 * input : 
 * output :
*/
void change_room_rule();

#endif