#include "chess.h"

chess_board* initBoard(){
    chess_board* b=(chess_board*)malloc(sizeof(chess_board));
    /* 보드 생성 */
    for(int i=2;i<ROW-2;i++){
        for(int j=0;j<COL;j++){
            b->board[i][j]=BLANK;
        }
    }
    for(int i=0;i<COL;i++){
        b->board[1][i]=B_PAWN;
        b->board[6][i]=W_PAWN;
    }
    b->board[0][0]=b->board[0][7]=B_ROOK;
    b->board[0][1]=b->board[0][6]=B_KNIGHT;
    b->board[0][2]=b->board[0][5]=B_BISHOP;
    b->board[0][3]=B_QUEEN;
    b->board[0][4]=B_KING;

    b->board[7][0]=b->board[7][7]=W_ROOK;
    b->board[7][1]=b->board[7][6]=W_KNIGHT;
    b->board[7][2]=b->board[7][5]=W_BISHOP;
    b->board[7][3]=W_QUEEN;
    b->board[7][4]=W_KING;
    
    /* 죽을 말을 저장하는 배열 */
    for(int i=0;i<PEICE_CNT;i++){
        b->white_death[i]=b->black_death[i]=BLANK;
    }
    /* 플레이어 순서 */
    b->player_turn=WHITE;

    /* special rule variables */
    b->castling_flag=false;
    b->en_passant_flag=false;
    for(int i=0;i<4;i++){
        b->last_move[i]=false;
    }

    /* 시간 설정 */
    b->black_time=PLAY_TIME;
    b->white_time=PLAY_TIME;

    return b;
}

void gameStart(){
    //player_turn=WHITE인데 init으로 이동
}

chess_board* resetGame(chess_board* b){
    finishGame(b);
    chess_board* b=initBoard();
    return b;
}

void undoGame(){

}

void finishGame(chess_board* b){
    //need : 이긴 사람을 확인하는 알고리즘


    free(b);
}

/* 말의 이동을 확인하고 이동시키는 함수들 */

bool handlePawn(chess_board* b, int sr, int sc, int fr, int fc){
    int moveRow=fr-sr;
    int moveCol=fc-sc;

    if(b->player_turn==WHITE){ //흰색 폰이 움직이는 경우
        if(moveRow==-1 && abs(moveCol)==1 && getPieceColor(b->board[fr][fc])==BLACK) return true;//대각선 사냥
        if(b->board[fr][fc] != BLANK) return false;//이미 말 있음
        if(sr==6 && moveCol==0 && moveRow==-2 && b->board[sr-1][sc]==BLANK) return true;//처음 움직일때 두칸 이동
        if(moveCol==0 && moveRow==-1) return true;//한칸 앞으로 이동
    }

    if(b->player_turn==BLACK){ //흑색 폰이 움직이는 경우
        if(moveRow==-1 && abs(moveCol)==1 && getPieceColor(b->board[fr][fc])==WHITE) return true;//대각선 사냥
        if(b->board[fr][fc] != BLANK) return false;//이미 말 있음
        if(sr==1 && moveCol==0 && moveRow==2 && b->board[sr+1][sc]==BLANK) return true;//처음 움직일 때 두칸 이동
        if(moveCol==0 && moveRow==1) return true;//한칸 앞으로 이동
    }

    return false;
}

bool handleRook(chess_board* b, int sr, int sc, int fr, int fc){
    int moveRow=fr-sr;
    int moveCol=fc-sc;

    //이동 중 말이 존재할 경우
    if(abs(moveRow)!=0 && moveCol==0){//수직으로 움직임
        if(sr<fr){
            for(int i=sr+1;i<fr;i++){
                if(b->board[i][sc] != BLANK) return false;
            }
        }
        else{
            for(int i=sr-1;i>fr;i--){
                if(b->board[i][sc] != BLANK) return false;
            }
        }
    } 
    else if(abs(moveCol)!=0 && moveRow==0){//수평으로 움직임
        if(sc<fc){
            for(int i=sc+1;i<fc;i++){
                if(b->board[sr][i] != BLANK) return false;
            }
        }
        else{
            for(int i=sc-1;i>fc;i--){
                if(b->board[sr][i] != BLANK) return false;
            }
        }
    }
    else{//수평과 수직을 섞어서 이동한 경우
        return false;
    }
    return true;
}

bool handleKnight(chess_board* b, int sr, int sc, int fr, int fc){
    int moveRow=fr-sr;
    int moveCol=fc-sc;

    if(abs(moveRow)==2 && abs(moveCol)==1 ) return true;
    if(abs(moveRow)==1 && abs(moveCol)==2 ) return true;
    return false;
}

bool handleBishop(chess_board* b, int sr, int sc, int fr, int fc){
    int moveRow=fr-sr;
    int moveCol=fc-sc;

    if(moveRow==0 || moveCol==0 || abs(moveRow)!=abs(moveCol)) return false;

    if(sr<fr && sc<fc){//우하향
        for(int i=sr+1,j=sc+1; i<fr; i++,j++){
            if(b->board[i][j]!=BLANK) return false;
        }
    }
    else if(sr<fr && sc>fc){//좌하향
        for(int i=sr+1,j=sc-1; i<fr; i++,j--){
            if(b->board[i][j]!=BLANK) return false;
        }
    }
    else if(sr>fr && sc<fc){//우상향
        for(int i=sr-1,j=sc+1; i>fr; i--,j++){
            if(b->board[i][j]!=BLANK) return false;
        }
    }
    else if(sr>fr && sc>fc){//좌상향
        for(int i=sr-1,j=sc-1; i>fr; i--,j--){
            if(b->board[i][j]!=BLANK) return false;
        }
    }
    return true;
}

bool handleQueen(chess_board* b, int sr, int sc, int fr, int fc){
    return handleBishop(b,sr,sc,fr,fc) || handleRook(b,sr,sc,fr,fc);
}

bool handleKing(chess_board* b, int sr, int sc, int fr, int fc){
    int moveRow=fr-sr;
    int moveCol=fc-sc;

    if(abs(moveRow)==1 && moveCol==0) return true;
    if(moveRow==0 && abs(moveCol)==1) return true;
    if(abs(moveRow)==1 && abs(moveCol)==1) return true;
    return false;
}

bool canMove(chess_board* b, int sr, int sc, int fr, int fc){
    //보드 밖으로 움직이거나, 자신이 말이 아닌 것을 움직이려거나, 자신의 말로 움직이려거나, 빈칸을 움직이려할 때
    if(sr<0 || sr>=ROW || sc<0 || sc>=COL || fr<0 || fr>=ROW || fc<0 || fc>=COL) return false;
    if(getPieceColor(b->board[sr][sc]) != b->player_turn) return false;
    if(getPieceColor(b->board[fr][fc]) == b-> player_turn) return false;
    if(getPieceColor(b->board[sr][sc]) == BLANK) return false;

    switch((b->board[sr][sc])%10){
        case 1 ://ROOK
            return handleRook(b,sr,sc,fr,fc);
        case 2 ://KNIGHT
            return handleKnight(b,sr,sc,fr,fc);
        case 3 ://BISHOP
            return handleBishop(b,sr,sc,fr,fc);
        case 4 ://QUEEN
            return handleQueen(b,sr,sc,fr,fc);
        case 5 ://KING
            return handleKing(b,sr,sc,fr,fc);
    }

    return false;
}

void movePiece(){

}

/* 게임 진행 여부를 확인하는 함수들 */

int getPieceColor(int piece){
    if(BLACK < piece && piece <= B_PAWN) return BLACK;
    if(WHITE < piece && piece <= W_PAWN) return WHITE;
    return BLANK;
}

void afterMove(){

}

void isFinish(){

}

void isCheck(){

}

void getMoveablePosition(){

}

void changeTurn(){

}

/* special rules */
void promotion(){

}

void castling(){

}

void forCastling(){

}

void enPassant(){

}
