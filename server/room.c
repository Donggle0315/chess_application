#include "room.h"

void* room_main(void* args){
    thread_arg *ta = (thread_arg*)args;
    pool_room *pr = ta->pr;
    int main_p1fd = ta->p1fd;
    int roomidx = ta->roomidx;


    pthread_detach(pthread_self());
    GAME_INFORMATION* gi=init_room();
    
    struct addrinfo *listp, *p;
    struct addrinfo hints;
    int listenfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // address suitable for server
    hints.ai_flags |=  AI_ADDRCONFIG; // returns valid addresses
    //hints.ai_flags |= AI_NUMERICSERV; // only accept number ip address

    getaddrinfo(NULL, NULL, &hints, &listp);

    for(p=listp; p != NULL; p=p->ai_next){
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
        close(listenfd);
    }

    

    if(listen(listenfd, 100) < 0){
        close(listenfd);
        pthread_exit((void*)-1);
    }
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // select
    char address[128];
    int count = 0;
    short port;

    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;

    switch (p->ai_family) {
        case AF_INET:
            sin = (struct sockaddr_in *) p;
            inet_ntop(AF_INET, &sin->sin_addr, address, 128);
            port = ntohs(sin->sin_port);      
            break;
        case AF_INET6:
            // TODO
            break;
    }

    // send address to p1fd
    // 그러면 p1에서 connect
    char buf[MAX_LEN];
    sprintf(buf, "ROM\n%s:%d\n", address, port); // address 보내야함
    write(main_p1fd, buf, MAX_LEN);

    sem_wait(&pr->mutex);
    strncpy(pr->room[roomidx].address, address, 128);
    pr->room[roomidx].port = port;
    sem_post(&pr->mutex);


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
    while(1){
        
        ready_set = read_set;
        select(maxfd+1, &ready_set, NULL, NULL, NULL);

        if(isFinish(b)) {//게임이 종료되면 결과를 전송
            char finish_buf[MAX_LEN];
            char* argv_buf[ARGUMENT_NUM];
            argv_buf[0]="FIN";
            if(b->player_turn==BLACK){//white승리
                argv_buf[1]="1";
            }
            else{//black승리
                argv_buf[1]="2";
            }
            makeString(argv_buf,finish_buf);
            write(p1fd,finish_buf,MAX_LEN);
            write(p2fd,finish_buf,MAX_LEN);
            break;
        }
        
        sendInfoToClient(gi,b,p1fd,p2fd);//게임 정보를 클라이언트에게 보냄
        //printBoard(b);

        int sr,sc,fr,fc;
        //턴 확인 후 말을 선택함
        if(b->player_turn==WHITE) printf("player : white\n");
        else printf("player : black\n");
        printf("select piece\ninput sr, sc: ");
        scanf("%d %d",&sr,&sc);
        
        //입력으로 -1 -1 이 들어오면 게임 리셋        
        if(sr==-1 && sc==-1) resetGame(b);
        
        //선택한 말이 이동할 수 있는 곳을 출력
        coordi moveable_pos[ROW*COL];
        int moveable_idx=0;
        getMoveablePosition(b,sr,sc,moveable_pos,&moveable_idx);
        if(!moveable_idx){//해당 말이 갈 곳이 없음
            printf("해당 말은 움직일 수 없음\n");
            continue;
        }
        else{
            do{
                printf("where is the piece going\n input fr, fc : ");
                scanf("%d %d",&fr,&fc);
            }while(!canMove(b,sr,sc,fr,fc));
            
            int deathCode=movePiece(b,sr,sc,fr,fc,true);
            if(deathCode){//잡은 말이 있을 경우
                addDeathPiece(b,deathCode);
            }
            else{
                printf("움직일 수 없음\n");
                continue;
            }
        }
        changeTurn(b,gi);
    }
    finishGame(b);
}

void exit_room(GAME_INFORMATION* gi,pool_room* pr){
    free(gi);
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
    char string[MAX_LEN];
    int idx=0;
    while(buf[idx][0]!=NULL){
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