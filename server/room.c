#include "room.h"

void* room_main(void* args){
    thread_arg *ta = (thread_arg*)args;
    pool_room *pr = ta->pr;
    int main_p1fd = ta->p1fd;
    int roomidx = ta->roomidx;
    free(ta);

    pthread_detach(pthread_self());
    GAME_INFORMATION* gi=init_room();
    
    struct addrinfo *listp, *p;
    struct addrinfo hints;
    int listenfd;
    int opt=1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // address suitable for server
    hints.ai_flags |=  AI_ADDRCONFIG; // returns valid addresses
    hints.ai_flags |= AI_NUMERICSERV; // only accept number ip address

    char port_string[6];
    unsigned short port;
    for(port=49152; port < 65535; port++){
        snprintf(port_string, sizeof(port_string), "%d", port);
        if(getaddrinfo(NULL, port_string, &hints, &listp)==0){
            break;
        }
        printf("can't connect to port: %d\n", port);
    }
    
    
    for(p=listp; p != NULL; p=p->ai_next){
        printf("try socket\n");
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        printf("try bind2\n");
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
        close(listenfd);
    }

    char address[1024];
    
    int en;
    if((en = getnameinfo(p->ai_addr, p->ai_addrlen, address, sizeof(address), NULL, 0, NI_NUMERICHOST)) != 0){
        printf("error in getnameinfo\n");
        printf("%s\n", gai_strerror(en));
    }
    
    if(listen(listenfd, 100) < 0){
        close(listenfd);
        pthread_exit((void*)-1);
    }
    


    // send address to p1fd
    // 그러면 p1에서 connect
    char buf[MAX_LEN];
    memset(buf, 0, MAX_LEN);
    sprintf(buf, "ENT\n%s:%d\n", address, port); // address 보내야함
    writeall(main_p1fd, buf, MAX_LEN);

    int temp;
    sem_getvalue(&pr->mutex, &temp);
    printf("%d\n", temp);
    
    sem_wait(&pr->mutex);
    printf("asdasd\n");
    strncpy(pr->room[roomidx].address, address, 128);
    pr->room[roomidx].port = port;
    sem_post(&pr->mutex);

    printf("yay\n");

    freeaddrinfo(listp);
    if(!p) pthread_exit((void*)-1);

    struct sockaddress_storage *clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_storage);

    fd_set read_set;
    fd_set ready_set;
    FD_ZERO(&read_set);
    FD_SET(listenfd, &read_set);

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
        if(isFinish(b)) {//게임이 종료되면 결과를 전송
            break;
        }
        sendInfoToClient(gi,b,p1fd,p2fd);//게임 정보를 클라이언트에게 보냄 "TUR"
        //printBoard(b);

        //read from client 
        int sr,sc,fr,fc;
        while(1){
            int now_player=getNowPlayer(gi,p1fd,p2fd);
            //현재 턴인 플레이어로부터 SEL 읽어오기
            while(flag!=0){//flag가 0이면 이미 좌표를 받은 상태임
                ready_set=read_set;
                nready=select(maxfd+1,&ready_set,NULL,NULL,NULL);
                if(nready<0){
                    fprintf(stderr,"Error in select\n");
                    return -1;
                }
                if(FD_ISSET(now_player,&ready_set)){
                    len=readall(now_player,buf,MAX_LEN);
                    //SEL\n#\n이 올바른지 확인 -> 올바르면 break, 틀리면 continue;
                    if(checkCMD(buf,gi->turn,SEL)){
                        break;
                    }
                }
            }
            
            getCoordinate(len,buf,&sr,&sc,&fr,&fc);
            if(sr==-1&&sc==-1) {
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
                if(FD_ISSET(now_player,&ready_set)){
                    len=readall(now_player,buf,MAX_LEN);
                    //MOV\n#\n이 올바른지 확인
                    //SEL 이면 다시 위로 올라가야하고, 잘못된 좌표라면 continue, 올바르면 break
                    if(checkCMD(buf,gi->turn,SEL)){
                        flag=0;
                        getCoordinate(len,buf,&sr,&sc,&fr,&fc);
                        if(!canMove(b,sr,sc,fr,fc)){
                            continue;
                        }
                        break;
                    }
                    if(checkCMD(buf,gi->turn,MOV)){
                        flag=1;
                    }
                    else{
                        continue;
                    }
                }                
            }

            if(flag==0){//SEL로 돌아가야 함
                continue;
            }

            //해당 위치로 말을 움직일 수 있음
            int deathCode=movePiece(b,sr,sc,fr,fc,true);
            sendIsMoveToClient(gi,true,p1fd,p2fd);//해당 위치로 움직였음을 클라이언트에게 보냄
            if(deathCode){
                addDeathPiece(b,deathCode);
            }
            break;
        }
        
        changeTurn(b);
    }
    sendFinishToClient(gi,b,p1fd,p2fd);
    finishGame(b);
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

void convertIntToString(int num, char*string){
    char tmp[MAX_LEN];
    tmp[MAX_LEN-1]='\0';
    int idx=MAX_LEN-2;
    
    while(num!=0){
        tmp[idx--]=(num%10)+'0';
        num/=10;
    }
    strcpy(string,tmp+idx+1);
}

void sendInfoToClient(GAME_INFORMATION* gi, chess_board* b, int p1fd, int p2fd){
    char buf[MAX_LEN];
    char tmp[MAX_LEN];
    strcat(buf,"TUR\n");
    convertIntToString(gi->turn,tmp);
    strcat(buf,tmp);
    strcat(buf,"\n");

    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            char piece[3];
            convertIntToString(b->board[i][j],piece);
            strcat(buf,piece);
        }
    }
    strcat(buf,"\n");

    write(p1fd,buf,MAX_LEN);
    write(p2fd,buf,MAX_LEN);
}

void sendMoveableToClient(GAME_INFORMATION* gi, coordi* moveable_pos,int idx,int p1fd,int p2fd){
    char buf[MAX_LEN];
    char tmp[MAX_LEN];
    strcat(buf,"SEL\n");
    convertIntToString(gi->turn,tmp);
    strcat(buf,tmp);
    strcat(buf,"\n");

    for(int i=0;i<idx;i++){
        char pos[6];
        char tmp_pos[2];
        convertIntToString(moveable_pos[i].row,pos);
        convertIntToString(moveable_pos[i].col,tmp_pos);
        strcat(pos,tmp_pos);
        strcat(buf,pos);
    }
    strcat(buf,"\n");

    write(p1fd,buf,MAX_LEN);
    write(p2fd,buf,MAX_LEN);
}

void sendIsMoveToClient(GAME_INFORMATION*gi, bool move,int p1fd,int p2fd){
    char buf[MAX_LEN];
    char tmp[MAX_LEN];
    strcat(buf,"MOV\n");
    convertIntToString(gi->turn,tmp);
    strcat(buf,tmp);
    strcat(buf,"\n");

    if(move){
        strcat(buf,"1\n");
    }
    else{
        strcat(buf,"0\n");
    }

    write(p1fd,buf,MAX_LEN);
    write(p2fd,buf,MAX_LEN);
}

void sendFinishToClient(GAME_INFORMATION* gi,chess_board* b,int p1fd,int p2fd){
    char buf[MAX_LEN];
    char tmp[MAX_LEN];
    strcat(buf,"MOV\n");
    convertIntToString(gi->turn,tmp);
    strcat(buf,tmp);
    strcat(buf,"\n");

    if(b->player_turn==BLACK){//white승리
        strcat(buf,"1\n");
    }
    else{//black승리
        strcat(buf,"2\n");
    }

    write(p1fd,buf,MAX_LEN);
    write(p2fd,buf,MAX_LEN);    
}

void getCoordinate(int len, char* buf, int* sr, int*sc,int* fr,int* fc){
    for(int i=0;i<len;i++){
        if('0'<=buf[i] && buf[i]<='9'){
            *sr=(buf[i]-'0');
            *sc=(buf[i+1]-'0');
            if('0'<=buf[i+2] && buf[i+2]<='9'){
                *fr=(buf[i+2]-'0');
                *fc=(buf[i+3]-'0');
            }
            break;
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