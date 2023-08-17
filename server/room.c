#include "room.h"

int room_main(pool_room* pr, char **arguments, int clientfd, send_info *si){
    int room_id = atoi(arguments[1]);
    arguments = &arguments[2];
	
	printf("error?11\n");
    room_option *room = &pr->room[room_id];
    // PLY: init game, send (board info, turn)
    if(!strcmp(arguments[0], "PLY")){
        start_game(room, si);
    }
    // SEL: check if right player(turn), check if right piece(black/white), send moveable
    else if(!strcmp(arguments[0], "SEL")){
        handle_SEL(room,si,arguments);
    }
    // MOV: check player turn, check right piece, check moveable
    // if moveable -> SUC, send 
    else if(!strcmp(arguments[0], "MOV")){
        handle_MOV(room,si,arguments);
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
}

void start_game(room_option *room, send_info *si){
    room->gi = init_room();
	
	printf("error?2\n");
    room->b = initBoard();
    
	printf("error?3\n");
	
    si->send_fds[(si->size)++] = room->player_fd[0];
    si->send_fds[(si->size)++] = room->player_fd[1];
	
	printf("error?\n");
	
    sendInfoToClient(room, si);
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

void handle_MOV(room_option *room, send_info *si, char** arguments){
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
    if(canMove(room->b,sr,sc,fr,fc)){
        deathCode=movePiece(room->b,sr,sc,fr,fc,true);
        if(deathCode) addDeathPiece(room->b,deathCode);
        changeTurn(room->b);
        increaseTurnCnt(room->gi);
    }
    else{
        flag=false;
    }
  
	//check if the game finish
    bool finish = isFinish(room->b);
    //send message to client
    if(turn%2) si->send_fds[(si->size)++]=room->player_fd[0];
    else si->send_fds[(si->size)++]=room->player_fd[1];
    sendIsMoveToClient(room,si,flag,finish);
    for(int sendidx=0; sendidx< si->size; sendidx++){
        writeall(si->send_fds[sendidx],si->send_string,MAX_LEN);
    }
    si->send_fds[(si->size)++]=room->player_fd[0];
    si->send_fds[(si->size)++]=room->player_fd[1];
    sendInfoToClient(room, si);
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


void increaseTurnCnt(GAME_INFORMATION* gi){
    (gi->turn)++;
}

void exit_room(GAME_INFORMATION* gi,pool_room* pr){
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
    if(finish){//게임 끝
        strcat(si->send_string,"FIN\n");
    }
    else{//게임 안 끝나
        strcat(si->send_string,"RES\n");
    }
}