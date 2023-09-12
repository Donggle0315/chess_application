#include "room.h"
#include "chess.h"

int roomMain(PoolClient* pc,PoolRoom* pr, char **arguments, int clientidx, SendInfo *si){
    int room_id = atoi(arguments[1]);
    arguments = &arguments[2];
    int clientfd = pc->clientfd[clientidx];
    RoomOption *room = &pr->room[room_id];

    /* compare recieved message's argument and execute matched functions */
    // PLY: init game, send (board info, turn)
    if(!strcmp(arguments[0], "PLY")){
        startGame(pr,room, si);
    }
    // SEL: check if right player(turn), check if right piece(black/white), send moveable
    else if(!strcmp(arguments[0], "SEL")){
        handleSEL(room,si,arguments);
    }
    // MOV: check player turn, check right piece, check moveable
    // if moveable -> SUC, send 
    else if(!strcmp(arguments[0], "MOV")){
        handleMOV(pr,room,si,arguments);
    }
    //RES: reset the game
    else if(!strcmp(arguments[0],"RES")){
        resetGame(room->b);
        room->gi->turn=1;
    }
    else if(!strcmp(arguments[0],"EXT")){
        finishGame(room->b);
        exitRoom(room->gi, pr);
    }
    else if(!strcmp(arguments[0], "INF")){
	sendGameInfoToClient(room,si,pc,clientidx);
    }
    else if(!strcmp(arguments[0], "CHK")){
	sendTimeOutToClient(pr,room,si,0);
    }
    return 1;
}

void startGame(PoolRoom* pr,RoomOption *room, SendInfo *si){
    room->gi = initRoom();
    room->gi->room_id = room->room_id;
	
    room->b = initBoard();
    room->b->white_time = room->time;
    room->b->black_time = room->time;
    gettimeofday(&room->gi->prev_time,NULL);
    si->send_fds[(si->size)++] = room->player_fd[0];
    si->send_fds[(si->size)++] = room->player_fd[1];
    sendInfoToClient(room, si);
    wrappedWriteAll(si);
    sendTimeOutToClient(pr,room,si,0);
}

void handleSEL(RoomOption *room, SendInfo *si, char** arguments){
    //check if right player_turn
    int turn = atoi(arguments[1]);
    if(room->gi->turn != turn) {
        sendMoveableToClient(room,si,NULL,0);
        return;
    }
    //get moveable poses
    int row = arguments[2][0]-'0';
    int col = arguments[2][1]-'0';
    Coordinate movealbe_pos[ROW*COL];
    int moveable_idx = 0;
    getMoveablePosition(room->b,row,col,movealbe_pos,&moveable_idx);
    
    //send message to client
    if(turn%2) si->send_fds[(si->size)++] = room->player_fd[0];
    else si->send_fds[(si->size)++] = room->player_fd[1];
    sendMoveableToClient(room,si,movealbe_pos,moveable_idx);
}

void handleMOV(PoolRoom* pr,RoomOption *room, SendInfo *si, char** arguments){
    //check if right player_turn
    int turn = atoi(arguments[1]);
    if(room->gi->turn != turn) {
        sendIsMoveToClient(room,si,false,false);
        return;
    }
    //if the piece can move, then move and change turn
    int start_row = arguments[2][0]-'0';
    int start_col = arguments[2][1]-'0';
    int finish_row = arguments[2][2]-'0';
    int finish_col = arguments[2][3]-'0';
    int deathCode;
    bool moveable_flag = true;
    Coordinate movealbe_pos[ROW*COL];
    int moveable_idx = 0;
    getMoveablePosition(room->b,start_row,start_col,movealbe_pos,&moveable_idx);
    if(isInMoveablePosition(finish_row,finish_col,movealbe_pos,moveable_idx)){
        deathCode = movePiece(room->b,start_row,start_col,finish_row,finish_col,true);
        if(deathCode) addDeathPiece(room->b,deathCode);
        increaseTurnCnt(room);
		changeTurn(room->b);
    }
    else{
        moveable_flag = false;
    }

    //check if the game finish
    bool finish = isFinish(room->b);
    //send message to client
    si->size = 0;
    if(turn%2) si->send_fds[(si->size)++] = room->player_fd[0];
    else si->send_fds[(si->size)++] = room->player_fd[1];

    sendIsMoveToClient(room,si,moveable_flag,finish);
    wrappedWriteAll(si);
    si->send_fds[(si->size)++] = room->player_fd[0];
    si->send_fds[(si->size)++] = room->player_fd[1];
    if(finish){
	int winner=(room->b->player_turn == BLACK) ? WHITE : BLACK;
	sendTimeOutToClient(pr,room,si,winner);
    }
    else{
	sendInfoToClient(room, si);
    }
}

GameInformation* initRoom(){
    GameInformation* gi=(GameInformation*)malloc(sizeof(GameInformation));
    gi->turn=1;

    return gi;
}

int addPlayer(GameInformation* gi,int connfd,fd_set read_set){

}

void changePlayerRole(){

} //보류

void changeRoomRule(){

} //보류


void increaseTurnCnt(RoomOption* room){
    (room->gi->turn)++;
    struct timeval now;
    gettimeofday(&now,NULL);

    double time_diff = (now.tv_sec - room->gi->prev_time.tv_sec);

    if(room->b->player_turn == WHITE){
	room->b->white_time -= (int)time_diff;
    }
    else{
	room->b->black_time -= (int)time_diff;
    }
    room->gi->prev_time = now;		
}

void exitRoom(GameInformation* gi,PoolRoom* pr){
    pr->room[gi->room_id].room_id = -1;
    free(gi);
}

void sendInfoToClient(RoomOption *room, SendInfo *si){
    sprintf(si->send_string,"ROO\nTUR\n%d\n",room->gi->turn);
    //store board information
    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0; col_idx < COL; col_idx++){
            char piece[3];
            sprintf(piece,"%02d",room->b->board[row_idx][col_idx]);
            strcat(si->send_string,piece);
        }
    }
    strcat(si->send_string,"\n");
}

void sendMoveableToClient(RoomOption *room,SendInfo* si, Coordinate* moveable_pos,int moveable_idx){
    sprintf(si->send_string,"ROO\nSEL\n%d\n",room->gi->turn);
    //좌표 정보 저장
    for(int idx = 0; idx < moveable_idx; idx++){
        char pos[6];
        sprintf(pos,"%d%d",moveable_pos[idx].row,moveable_pos[idx].col);
        strcat(si->send_string,pos);
    }
    strcat(si->send_string,"\n");
}

void sendIsMoveToClient(RoomOption *room, SendInfo *si, bool move, bool finish){
    sprintf(si->send_string,"ROO\nMOV\n%d\n",room->gi->turn);
    if(move){//이동 성공
        strcat(si->send_string,"SUC\n");
    }
    else{//이동 실패
        strcat(si->send_string,"FAL\n");
    }
}

void sendGameInfoToClient(RoomOption* room, SendInfo* si,PoolClient* pc, int clientidx){
    if(room->cur_user_count != 2){
	sprintf(si->send_string,"ROO\nINF\n1\n%s\n%s\n",pc->client_info[room->player_idx[0]].user_id,"NULL");
    }
    else{
	sprintf(si->send_string,"ROO\nINF\n1\n%s\n%s\n",pc->client_info[room->player_idx[0]].user_id,pc->client_info[room->player_idx[1]].user_id);
    }
    si->send_fds[(si->size)++] = room->player_fd[0];

    wrappedWriteAll(si);
    if(room->cur_user_count != 2) return;
    sprintf(si->send_string,"ROO\nINF\n0\n%s\n%s\n",pc->client_info[room->player_idx[0]].user_id,pc->client_info[room->player_idx[1]].user_id);
    si->send_fds[(si->size)++] = room->player_fd[1];
    wrappedWriteAll(si);
}

void sendTimeOutToClient(PoolRoom* pr,RoomOption* room,SendInfo* si,int winner){
    struct timeval now;
    bool isFinish = false;
    gettimeofday(&now,NULL);
    double time_diff = (now.tv_sec - room->gi->prev_time.tv_sec);
    if(room->b->player_turn == WHITE){
	room->b->white_time -= (int)time_diff;
    }
    else{
	room->b->black_time -= (int)time_diff;
    }
    room->gi->prev_time = now;
    if(room->b->white_time <= 0 || winner == BLACK) {
	sprintf(si->send_string,"ROO\nFIN\nP2\n");
	isFinish = true;
    }
    else if(room->b->black_time <= 0 || winner == WHITE){
	sprintf(si->send_string,"ROO\nFIN\nP1\n");
	isFinish = true;
    }
    else if(winner == 0){
	sprintf(si->send_string,"ROO\nRES\n%d\n%d\n",room->b->white_time, room->b->black_time);		
    }
    si->send_fds[(si->size)++] = room->player_fd[0];
    si->send_fds[(si->size)++] = room->player_fd[1];
    wrappedWriteAll(si);
    if(isFinish){
	finishGame(room->b);
	exitRoom(room->gi, pr);
    }
}
