/*
room.c - prototypes adn definitions for room
*/

#ifndef __ROOM_H__
#define __ROOM_H__

#define TUR 1
#define SEL 2
#define MOV 3
#define ERR 4

#include "structures_constants.h"
#include "server.h"


/**
 * implement : 
 * input : 
 * output :
*/
int roomMain(PoolClient*, PoolRoom*, char**, int, SendInfo*);

/**
 * implement : 
 * input : 
 * output :
*/
void startGame(PoolRoom* ,RoomOption*, SendInfo*);

/**
 * implement : 
 * input : 
 * output :
*/
void handleSEL(RoomOption*,SendInfo*,char**);

/**
 * implement : 
 * input : 
 * output :
*/
void handleMOV(PoolRoom* ,RoomOption*, SendInfo*,char**);

/**
 * implement : 
 * input : 
 * output :
*/
GameInformation* initRoom();

/**
 * implement : 
 * input : 
 * output :
*/
void increaseTurnCnt(RoomOption*);

/**
 * implement : 
 * input : 
 * output :
*/
void exitRoom(GameInformation*,PoolRoom*);

/**
 * implement : 
 * input : 
 * output :
*/
void sendInfoToClient(RoomOption*,SendInfo*);

/**
 * implement : 
 * input : 
 * output :
*/
void sendMoveableToClient(RoomOption*, SendInfo*, Coordinate*, int);

/**
 * implement : 
 * input : 
 * output :
*/
void sendIsMoveToClient(RoomOption*, SendInfo*,bool,bool);

/**
 * implement : 
 * input : 
 * output :
*/
void sendGameInfoToClient(RoomOption*, SendInfo*, PoolClient*, int);

void sendTimeOutToClient(PoolRoom*, RoomOption*, SendInfo*, int);
/**
 * implement : 
 * input : 
 * output :
*/
int addPlayer(GameInformation*,int,fd_set);

/**
 * implement : 
 * input : 
 * output :
*/
void changePlayerRole();

/**
 * implement : 
 * input : 
 * output :
*/
void changeRoomRule();

#endif
