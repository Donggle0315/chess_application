#include "room.h"

int room_main(pool_client* pc,pool_room* pr, char **arguments, int clientidx, send_info *si){
    int room_id = atoi(arguments[1]);
    arguments = &arguments[2];
	int clientfd=pc->clientfd[clientidx];
	printf("error?11\n");
    room_option *room = &pr->room[room_id];
    // PLY: init game, send (board info, turn)
    if(!strcmp(arguments[0], "PLY")){
        start_game(pr,room, si);
    }
    // SEL: check if right player(turn), check if right piece(black/white), send moveable
    else if(!strcmp(arguments[0], "SEL")){
        handle_SEL(room,si,arguments);
    }
    // MOV: check player turn, check right piece, check moveable
    // if moveable -> SUC, send 
    else if(!strcmp(arguments[0], "MOV")){
        handle_MOV(pr,room,si,arguments);
    }
    //RES: reset the game
    else if(!strcmp(arguments[0],"RES")){
        resetGame(room->b);
        room->gi->turn=1;
    }
    else if(!strcmp(arguments[0],"EXT")){
        finishGame(room->b);
        exit_room(room->gi, pr);
    }
    else if(!strcmp(arguments[0], "INF")){
        sendGameInfoToClient(room,si,pc,clientidx);
    }
	else if(!strcmp(arguments[0], "CHK")){
		sendTimeOutToClient(pr,room,si,0);
	}
}

void start_game(pool_room* pr,room_option *room, send_info *si){
    room->gi = init_room();
	room->gi->room_id = room->room_id;
	
    room->b = initBoard();
    room->b->p1_time = room->time;
	room->b->p2_time = room->time;
	gettimeofday(&room->gi->prev_time,NULL);
    si->send_fds[(si->size)++] = room->player_fd[0];
    si->send_fds[(si->size)++] = room->player_fd[1];
    sendInfoToClient(room, si);
	for(int sendidx=0; sendidx< si->size; sendidx++){
        writeall(si->send_fds[sendidx],si->send_string,MAX_LEN);
    }
	si->size=0;
	sendTimeOutToClient(pr,room,si,0);
}

void handle_SEL(room_option *room, send_info *si, char** arguments){
    //check if right player_turn
    int turn = atoi(arguments[1]);
    if(room->gi->turn != turn) {
        sendMoveableToClient(room,si,NULL,0);
        return;
    }
    //get moveable poses
    int row = arguments[2][0]-'0';
    int col = arguments[2][1]-'0';
    coordi movealbe_pos[ROW*COL];
    int moveable_idx=0;
    getMoveablePosition(room->b,row,col,movealbe_pos,&moveable_idx);
    
    //send message to client
    if(turn%2) si->send_fds[(si->size)++]=room->player_fd[0];
    else si->send_fds[(si->size)++]=room->player_fd[1];
    sendMoveableToClient(room,si,movealbe_pos,moveable_idx);
}

void handle_MOV(pool_room* pr,room_option *room, send_info *si, char** arguments){
    //check if right player_turn
    int turn = atoi(arguments[1]);
    if(room->gi->turn != turn) {
		printf("in if\n");
        sendIsMoveToClient(room,si,false,false);
        return;
    }
    //if the piece can move, then move and change turn
    int sr=arguments[2][0]-'0';
    int sc=arguments[2][1]-'0';
    int fr=arguments[2][2]-'0';
    int fc=arguments[2][3]-'0';
    int deathCode;
    bool flag=true;
    //if(canMove(room->b,sr,sc,fr,fc)){
    coordi movealbe_pos[ROW*COL];
    int moveable_idx=0;
    getMoveablePosition(room->b,sr,sc,movealbe_pos,&moveable_idx);
    if(isInMoveablePosition(fr,fc,movealbe_pos,moveable_idx)){
        deathCode=movePiece(room->b,sr,sc,fr,fc,true);
        if(deathCode) addDeathPiece(room->b,deathCode);
        increaseTurnCnt(room);
		changeTurn(room->b);
        
    }
    else{
        flag=false;
    }
  
	//check if the game finish
    bool finish = isFinish(room->b);
	if(finish) printf("isFin is true\n");
    //send message to client
    si->size = 0;
	if(turn%2) si->send_fds[(si->size)++]=room->player_fd[0];
    else si->send_fds[(si->size)++]=room->player_fd[1];
    sendIsMoveToClient(room,si,flag,finish);
    for(int sendidx=0; sendidx< si->size; sendidx++){
        writeall(si->send_fds[sendidx],si->send_string,MAX_LEN);
    }
	si->size=0;
    si->send_fds[(si->size)++]=room->player_fd[0];
    si->send_fds[(si->size)++]=room->player_fd[1];
	if(finish){
		int winner=(room->b->player_turn == BLACK) ? WHITE:BLACK;
		sendTimeOutToClient(pr,room,si,winner);
	}
    else{
		sendInfoToClient(room, si);
	}
}

GAME_INFORMATION* init_room(){
    GAME_INFORMATION* gi=(GAME_INFORMATION*)malloc(sizeof(GAME_INFORMATION));
    gi->turn=1;

    return gi;
}

int add_player(GAME_INFORMATION* gi,int connfd,fd_set read_set){

}

void change_player_role(){

} //보류

void change_room_rule(){

} //보류


void increaseTurnCnt(room_option* room){
    (room->gi->turn)++;
	struct timeval now;
	gettimeofday(&now,NULL);
	
	double time_diff = (now.tv_sec - room->gi->prev_time.tv_sec);
	printf("time_diff %lf\n",time_diff);
	
	if(room->b->player_turn==WHITE){
		room->b->p1_time -= (int)time_diff;
	}
	else{
		room->b->p2_time -= (int)time_diff;
	}
	room->gi->prev_time=now;
	printf("white : %d\nblack : %d",room->b->p1_time, room->b->p2_time);
		
}

void exit_room(GAME_INFORMATION* gi,pool_room* pr){
    pr->room[gi->room_id].room_id=-1;
	free(gi);
}

void sendInfoToClient(room_option *room, send_info *si){
    sprintf(si->send_string,"ROO\nTUR\n%d\n",room->gi->turn);
    //보드 위 정보를 저장
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            char piece[3];
            sprintf(piece,"%02d",room->b->board[i][j]);
            strcat(si->send_string,piece);
        }
    }
    strcat(si->send_string,"\n");
}

void sendMoveableToClient(room_option *room,send_info* si, coordi* moveable_pos,int idx){
    sprintf(si->send_string,"ROO\nSEL\n%d\n",room->gi->turn);
    //좌표 정보 저장
    for(int i=0;i<idx;i++){
        char pos[6];
        sprintf(pos,"%d%d",moveable_pos[i].row,moveable_pos[i].col);
        strcat(si->send_string,pos);
    }
    strcat(si->send_string,"\n");
}

void sendIsMoveToClient(room_option *room, send_info *si, bool move, bool finish){
    sprintf(si->send_string,"ROO\nMOV\n%d\n",room->gi->turn);
    if(move){//이동 성공
        strcat(si->send_string,"SUC\n");
    }
    else{//이동 실패
        strcat(si->send_string,"FAL\n");
    }
}

void sendGameInfoToClient(room_option* room, send_info* si,pool_client* pc, int clientidx){
	if(room->cur_user_count!=2){
    	sprintf(si->send_string,"ROO\nINF\n1\n%s\n%s\n",pc->client_info[room->player_idx[0]].user_id,"NULL");
	}
	else{
		sprintf(si->send_string,"ROO\nINF\n1\n%s\n%s\n",pc->client_info[room->player_idx[0]].user_id,pc->client_info[room->player_idx[1]].user_id);
	}
	printf("%s\n",si->send_string);
    si->send_fds[(si->size)++]=room->player_fd[0];
    // int enemy_idx=room->player_idx[1];
    // strcat(si->send_string,pc->client_info[enemy_idx].user_id);
    for(int sendidx=0; sendidx< si->size; sendidx++){
        writeall(si->send_fds[sendidx],si->send_string,MAX_LEN);
    }
	si->size=0;
    if(room->cur_user_count!=2) return;
    sprintf(si->send_string,"ROO\nINF\n0\n%s\n%s\n",pc->client_info[room->player_idx[0]].user_id,pc->client_info[room->player_idx[1]].user_id);
    // enemy_idx=room->player_idx[0];
    // strcat(si->send_string,pc->client_info[enemy_idx].user_id);
    si->send_fds[(si->size)++]=room->player_fd[1];
    for(int sendidx=0; sendidx< si->size; sendidx++){
        writeall(si->send_fds[sendidx],si->send_string,MAX_LEN);
    }
	si->size=0;
}

void sendTimeOutToClient(pool_room* pr,room_option* room,send_info* si,int winner){
	struct timeval now;
	bool isFin=false;
	gettimeofday(&now,NULL);
	double time_diff = (now.tv_sec - room->gi->prev_time.tv_sec);
	if(room->b->player_turn==WHITE){
		room->b->p1_time -= (int)time_diff;
	}
	else{
		room->b->p2_time -= (int)time_diff;
	}
	room->gi->prev_time=now;
	if(room->b->p1_time <=0 || winner==BLACK) {
	    sprintf(si->send_string,"ROO\nFIN\nP2\n");
		isFin=true;
	}
	else if(room->b->p2_time<=0 || winner==WHITE){
		sprintf(si->send_string,"ROO\nFIN\nP1\n");
		isFin=true;
	}
	else if(winner ==0){
		sprintf(si->send_string,"ROO\nRES\n%d\n%d\n",room->b->p1_time, room->b->p2_time);		
	}
	si->size=0;
	si->send_fds[(si->size)++]=room->player_fd[0];
	si->send_fds[(si->size)++]=room->player_fd[1];
    for(int sendidx=0; sendidx< si->size; sendidx++){
        writeall(si->send_fds[sendidx],si->send_string,MAX_LEN);
    }
	si->size=0;
	if(isFin){
		printf("isFin\n");
		finishGame(room->b);
        exit_room(room->gi, pr);
	}
}