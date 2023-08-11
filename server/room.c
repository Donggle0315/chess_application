#include "room.h"

int room_main(pool_room* pr, char **arguments, int clientfd, send_info *si){
    int room_id = atoi(arguments[1]);
    arguments = &arguments[2];

    room_option *room = &pr->room[room_id];
    // PLY
    if(!strcmp(arguments[0], "PLY")){
        start_game(room, si);
    }
    // TUR

    // SEL

    // MOV
    
    

    int nready;
    int maxfd = listenfd;
    int p1fd, p2fd;
    int player_count = 0;
    int count = 0;
    while(1){
        printf("\ninside room: %d\n", count++);

        ready_set = read_set;

        nready = select(maxfd+1, &ready_set, NULL, NULL, NULL);

        // handle events
        if(FD_ISSET(listenfd, &ready_set)){
            int connfd = accept(listenfd, (SA*)clientaddr, &clientlen);
            bool temp = add_player(gi, connfd, read_set);
            if(temp == true){
                player_count++;
            }
            
            fprintf(stdout, "added player in fd: %d\n", connfd);
            continue;
        }

        if(player_count == 2){
            break;
        }
    }

    start_game(gi,read_set,maxfd,p1fd,p2fd);//체스 게임 시작

    exit_room(gi,pr);
    pthread_exit(0);
}

int start_game(room_option *room, send_info *si){
    room->gi = init_room();
    room->b = initBoard();
    
    si->send_fds[(si->size)++] = room->player_fd[0];
    si->send_fds[(si->size)++] = room->player_fd[1];

    sendInfoToClient(room->gi, room->b);
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

void start_game(GAME_INFORMATION* gi, fd_set read_set, int maxfd, int p1fd, int p2fd){
    chess_board* b=initBoard();

    fd_set ready_set;
    int nready;
    int len;
    char buf[MAX_LEN];
    int flag=2;
    while(1){
        if(isFinish(b)) {//게임이 종료되면 결과를 전송하고 무한 루프 탈출
            break;
        }
        sendInfoToClient(gi,b,p1fd,p2fd);//게임 정보를 클라이언트에게 보냄 "TUR"
        //read from client 
        int sr,sc,fr,fc;
        while(1){//올바른 SEL 문장을 읽어옴
            int now_player=getNowPlayer(gi,p1fd,p2fd);
            //현재 턴인 플레이어로부터 SEL 읽어오기
            while(flag!=0){//flag가 0이면 이미 좌표를 받은 상태임
                ready_set=read_set;
                nready=select(maxfd+1,&ready_set,NULL,NULL,NULL);
                if(nready<0){
                    fprintf(stderr,"Error in select\n");
                    return -1;
                }
                if(FD_ISSET(now_player,&ready_set)){//현재 순서에 맞는 플레이어가 입력함
                    len=readall(now_player,buf,MAX_LEN);
                    //SEL\n#\n이 올바른지 확인 -> 올바르면 break, 틀리면 continue;
                    if(checkCMD(buf,gi->turn,SEL)){
                        break;
                    }
                }
            }
            
            getCoordinate(len,buf,&sr,&sc,&fr,&fc);//버퍼에서 좌표를 추출
            if(sr==-1&&sc==-1) {//게임 리셋
                resetGame(b);
                break;
            }

            //자신의 말이 선택되었다면, 해당 말이 갈 수 있는 좌표를 구해 클라이언트에게 전달
            coordi moveable_pos[ROW*COL];
            int moveable_idx=0;
            getMoveablePosition(b,sr,sc,moveable_pos,&moveable_idx);
            sendMoveableToClient(gi,moveable_pos,moveable_idx,p1fd,p2fd);

            //클라이언트로부터 좌표를 불러와 움직일 수 있다면, 위치로 이동시켜줌
            while(1){
                ready_set=read_set;
                nready=select(maxfd+1,&ready_set,NULL,NULL,NULL);
                if(nready<0){
                    fprintf(stderr,"Error in select\n");
                    return -1;
                }
                if(FD_ISSET(now_player,&ready_set)){//현재 순서에 맞는 플레이어가 입력함
                    len=readall(now_player,buf,MAX_LEN);
                    //MOV\n#\n이 올바른지 확인
                    //SEL 이면 다시 위로 올라가야하고, 잘못된 좌표라면 continue, 올바르면 break
                    if(checkCMD(buf,gi->turn,SEL)){
                        flag=0;
                        // getCoordinate(len,buf,&sr,&sc,&fr,&fc);
                        // if(!canMove(b,sr,sc,fr,fc)){
                        //     continue;
                        // }
                        break;
                    }
                    if(checkCMD(buf,gi->turn,MOV)){
                        flag=1;
                        getCoordinate(len,buf,&sr,&sc,&fr,&fc);
                        if(!canMove(b,sr,sc,fr,fc)){//해당 좌표로 이동할 수 없으면, false를 보내고 다시 입력 받도록함
                            sendIsMoveToClient(gi,false,p1fd,p2fd);
                            continue;
                        }
                        break;//올바른 좌표 정보가 들어왔다면 무한루프 탈출
                    }
                }                
            }

            if(flag==0){//SEL로 돌아가야 함
                continue;
            }

            //해당 위치로 말을 움직임
            int deathCode=movePiece(b,sr,sc,fr,fc,true);
            sendIsMoveToClient(gi,true,p1fd,p2fd);//해당 위치로 움직였음을 클라이언트에게 보냄
            if(deathCode){//죽을 말이 있을 경우 추가
                addDeathPiece(b,deathCode);
            }
            break;
        }
        //플레이어 턴 변경
        changeTurn(b);
        increaseTurnCnt(gi);
    }
    //게임 종료
    sendFinishToClient(gi,b,p1fd,p2fd);
    finishGame(b);
}

void increaseTurnCnt(GAME_INFORMATION* gi){
    (gi->turn)++;
}

void exit_room(GAME_INFORMATION* gi,pool_room* pr){
    free(gi);
}

int getNowPlayer(GAME_INFORMATION* gi, int p1fd, int p2fd){
    if((gi->turn)%2==0) return p2fd;
    return p1fd;
}

int getString(char* string,char** buf){
    char* ptr=strtok(string,"\n");
    int idx=0;
    while(ptr!=NULL){
        strcpy(buf[idx++],ptr);
        ptr=strtok(NULL,"\n");
    }

    return idx;
}

void makeString(char** buf, char* string){
    int idx=0;
    while(buf[idx][0]!=0){
        strcat(string,buf[idx]);
        strcat(string,"\n");
        idx++;
    }
}

void sendInfoToClient(GAME_INFORMATION* gi, chess_board* b){
    char buf[MAX_LEN];
    fprintf(buf,"TUR\n%d\n",gi->turn);
    //보드 위 정보를 저장
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            char piece[3];
            fprintf(piece,"%d",b->board[i][j]);
            strcat(buf,piece);
        }
    }
    strcat(buf,"\n");
}

void sendMoveableToClient(GAME_INFORMATION* gi, coordi* moveable_pos,int idx,int p1fd,int p2fd){
    char buf[MAX_LEN];
    fprintf(buf,"SEL\n%d\n",gi->turn);
    //좌표 정보 저장
    for(int i=0;i<idx;i++){
        char pos[6];
        fprintf(pos,"%d%d",moveable_pos[i].row,moveable_pos[i].col);
        strcat(buf,pos);
    }
    strcat(buf,"\n");

    writeall(p1fd,buf,MAX_LEN);
    writeall(p2fd,buf,MAX_LEN);
}

void sendIsMoveToClient(GAME_INFORMATION*gi, bool move,int p1fd,int p2fd){
    char buf[MAX_LEN];
    fprintf(buf,"MOV\n%d\n",gi->turn);
    if(move){//이동 성공
        strcat(buf,"1\n");
    }
    else{//이동 실패
        strcat(buf,"0\n");
    }

    writeall(p1fd,buf,MAX_LEN);
    writeall(p2fd,buf,MAX_LEN);
}

void sendFinishToClient(GAME_INFORMATION* gi,chess_board* b,int p1fd,int p2fd){
    char buf[MAX_LEN];
    fprintf(buf,"FIN\n%d\n",gi->turn);
    if(b->player_turn==BLACK){//white승리
        strcat(buf,"1\n");
    }
    else{//black승리
        strcat(buf,"2\n");
    }

    writeall(p1fd,buf,MAX_LEN);
    writeall(p2fd,buf,MAX_LEN);
}

void getCoordinate(int len, char* buf, int* sr, int*sc,int* fr,int* fc){
    int idx=0;
    int newLineCnt=2;
    // 좌표가 있는 곳(개행문자 2번 뒤)까지 접근
    while(newLineCnt!=0){
        if(buf[idx++]=='\n'){
            newLineCnt--;
        }
    }
    //좌표 정보를 저장
    if('0'<=buf[idx] && buf[idx]<='9'){
        *sr=(buf[idx]-'0');
        *sc=(buf[idx+1]-'0');
        if('0'<=buf[idx+2] && buf[idx+2]<='9'){
            *fr=(buf[idx+2]-'0');
            *fc=(buf[idx+3]-'0');
        }
        else{
            *fr=-1;
            *fc=-1;
        }
    }
}

bool checkCMD(char* buf,int turn, int cmd){
    //cmd가 맞는지 확인
    bool cmdFlag=false;
    switch(cmd){
        case SEL:
            if(!strncmp(buf,"SEL",3)){
                cmdFlag=true;
            }
            break;
        case MOV:
            if(!strncmp(buf,"MOV",3)){
                cmdFlag=true;
            }
            break;
    }

    //턴수가 맞는지 확인
    int tur=0;
    for(int i=4;buf[i]!='\n';i++){
        tur*=10;
        tur+=(buf[i]-'0');
    }
    //결과 반환
    if(tur==turn && cmdFlag) return true;
    return false;
}