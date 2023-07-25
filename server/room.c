#include "room.h"

void* room_main(void* args){
    pthread_detach(pthread_self());
    GAME_INFORMATION* gi=init_room();
    

    start_game();//체스 게임 시작

    pthread_exit(0);
}

GAME_INFORMATION* init_room(){

}

int add_player(GAME_INFORMATION* gi){

}

void change_player_role(){

} //보류

void change_room_rule(){

} //보류

void start_game(){
    chess_board* b=initBoard();

    while(1){
        if(isFinish(b)) break;
        
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
        changeTurn(b);
    }
    finishGame(b);
}

void exit_room(pool_room* pr){

}
