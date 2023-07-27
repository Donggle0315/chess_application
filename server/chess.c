#include "chess.h"
#include "room.h"

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
    chess_board* re=initBoard();
    return re;
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
        case 6://PAWN
            return handlePawn(b,sr,sc,fr,fc);
    }
    return false;
}

int movePiece(chess_board* b,int sr,int sc, int fr, int fc, bool option){
    int deathCode=BLANK;
    if(b->en_passant_flag){//앙파상인 경우
        if((b->board[sr][sc]%10)==6 && abs(sc-fc)==1 && b->board[fr][fc]==0){
            if(b->player_turn==BLACK){
                b->board[b->last_move[2]+1][b->last_move[3]]=b->board[b->last_move[2]][b->last_move[3]];
                b->board[b->last_move[2]][b->last_move[3]]=BLANK;
            }
            else if(b->player_turn==WHITE){
                b->board[b->last_move[2]-1][b->last_move[3]]=b->board[b->last_move[2]][b->last_move[3]];
                b->board[b->last_move[2]][b->last_move[3]]=BLANK;
            }
        }
    }

    if(b->castling_flag){//캐슬링
        if(sr==0 && sc==4 && fr==0 && fc==1){
            b->board[0][2]=b->board[0][0];
            b->board[0][0]=BLANK;
        }
        else if(sr==0 && sc==4 && fr==0 && fc==6){
            b->board[0][5]=b->board[0][7];
            b->board[0][7]=BLANK;
        }
        else if(sr==7 && sc==4 && fr==7 && fc==1){
            b->board[7][2]=b->board[7][0];
            b->board[7][0]=BLANK;
        }
        else if(sr==7 && sc==4 && fr==7 && fc==6){
            b->board[7][5]=b->board[7][7];
            b->board[7][7]=BLANK;
        }
    }

    //말을 잡은 경우 deathCode에 해당 말 정보 등록
    if(getPieceColor(b->board[fr][fc])!=BLANK) deathCode=b->board[fr][fc];

    //말 이동
    b->board[fr][fc]=b->board[sr][sc];
    b->board[sr][sc]=BLANK;

    if(option){//캐슬링 확인
        b->last_move[0]=sr;
        b->last_move[1]=sc;
        b->last_move[2]=fr;
        b->last_move[3]=fc;
        b->last_move[4]=b->board[fr][fc];

        if(sr==0 && sc==0) b->castling_check[0]=true;//LEFT B_ROOK
        else if(sr==0 && sc==4) b->castling_check[1]=true;//B_KING
        else if(sr==0 && sc==7) b->castling_check[2]=true;//RIGHT B_ROOK
        else if(sr==7 && sc==0) b->castling_check[3]=true;//LEFT W_ROOK
        else if(sr==7 && sc==4) b->castling_check[4]=true;//W_KING
        else if(sr==7 && sc==7) b->castling_check[5]=true;//RIGHT B_ROOK
    }

    return deathCode;
}   

void addDeathPiece(chess_board*b, int deathCode){
    int enemy=(b->player_turn==WHITE)?BLACK:WHITE;
    if(enemy==WHITE){
        b->white_death[b->w_death_idx]=deathCode;
        (b->w_death_idx)++;
    }
    else if(enemy==BLACK){
        b->black_death[b->b_death_idx]=deathCode;
        (b->b_death_idx)++;
    }
}
/* 게임 진행 여부를 확인하는 함수들 */

int getPieceColor(int piece){
    if(BLACK < piece && piece <= B_PAWN) return BLACK;
    if(WHITE < piece && piece <= W_PAWN) return WHITE;
    return BLANK;
}

void afterMove(chess_board* b,int fr, int fc){
    if(b->board[fr][fc]%10==6){//도착한 말이 폰일 경우
        int check_row=(b->player_turn==BLACK) ? 7 : 0;
        if(fr==check_row){
            b->promotion_r=fr;
            b->promotion_c=fc;
            //drawPromotion(b->player_turn); //need : 추가 구현
        }
    }
}

bool isFinish(chess_board* b){
    return false;
    bool false_flag=false;
    int enemy_player=(b->player_turn==WHITE)?BLACK:WHITE;
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            if(getPieceColor(b->board[i][j])==enemy_player){
                coordi tmp_pos[64];
                int tmp_idx=0;
                changeTurn(b,NULL);
                getMoveablePosition(b,i,j,tmp_pos,&tmp_idx);
                for(int k=0;k<tmp_idx;k++){
                    chess_board* tmp_board=copyBoard(b);
                    int tmp=movePiece(b,i,j,tmp_pos[k].row,tmp_pos[k].col,false);
                    if(!isCheck(b)) false_flag=true;
                    recover_board(b,tmp_board);
                }
                changeTurn(b,NULL);
                if(false_flag) return false;
            }
        }
    }
    return true;
}

chess_board* copyBoard(chess_board* b){
    chess_board* tmp=(chess_board*)malloc(sizeof(chess_board));
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            tmp->board[i][j]=b->board[i][j];
        }
    }
    for(int i=0;i<PEICE_CNT;i++){
        tmp->white_death[i]=b->white_death[i];
        tmp->black_death[i]=b->black_death[i];
    }
    tmp->w_death_idx=b->w_death_idx;
    tmp->b_death_idx=b->b_death_idx;

    tmp->black_check=b->black_check;
    tmp->white_check=b->white_check;
    tmp->player_turn=b->player_turn;

    tmp->promotion_r=b->promotion_r;
    tmp->promotion_c=b->promotion_c;

    for(int i=0;i<6;i++){
        tmp->castling_check[i]=b->castling_check[i];
    }
    tmp->castling_flag=b->castling_flag;
    for(int i=0;i<5;i++){
        tmp->last_move[i]=b->last_move[i];
    }
    tmp->en_passant_flag=b->en_passant_flag;
    tmp->black_time=b->black_time;
    tmp->white_time=b->white_time;

    return tmp;
}

void recover_board(chess_board* tmp, chess_board*b){
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            tmp->board[i][j]=b->board[i][j];
        }
    }
    for(int i=0;i<PEICE_CNT;i++){
        tmp->white_death[i]=b->white_death[i];
        tmp->black_death[i]=b->black_death[i];
    }
    tmp->w_death_idx=b->w_death_idx;
    tmp->b_death_idx=b->b_death_idx;

    tmp->black_check=b->black_check;
    tmp->white_check=b->white_check;
    tmp->player_turn=b->player_turn;

    tmp->promotion_r=b->promotion_r;
    tmp->promotion_c=b->promotion_c;

    for(int i=0;i<6;i++){
        tmp->castling_check[i]=b->castling_check[i];
    }
    tmp->castling_flag=b->castling_flag;
    for(int i=0;i<5;i++){
        tmp->last_move[i]=b->last_move[i];
    }
    tmp->en_passant_flag=b->en_passant_flag;
    tmp->black_time=b->black_time;
    tmp->white_time=b->white_time;

}

bool isCheck(chess_board* b){
    int k_row, k_col, color;
    bool flag=false;
    color=(b->player_turn==WHITE)?WHITE:BLACK;

    //본인 왕의 위치를 찾음
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            if(b->board[i][j]%10==5 && getPieceColor(b->board[i][j])==color){//자신의 왕일 때
                k_row=i;
                k_col=j;
            }
        }
    }
    //상대방 말이 본인의 왕의 위치로 이동할 수 있으면 true
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            changeTurn(b,NULL);
            if(canMove(b,i,j,k_row,k_col)){
                flag=true;
            }
            changeTurn(b,NULL);
        }
    }
    return flag;
}

void getMoveablePosition(chess_board* b, int row, int col,coordi* can_pos,int* idx){
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            if(canMove(b,row,col,i,j)){
                chess_board* tmp_board=copyBoard(b);
                int tmp=movePiece(b,row,col,i,j,false);
                if(!isCheck(b)){
                    can_pos[*idx].row=i;
                    can_pos[*idx].col=j;
                    (*idx)++;
                }
                recover_board(b,tmp_board);
            }
        }
    }

    coordi tmp_en_passant;
    enPassant(b,&tmp_en_passant,row,col);
    if(b->en_passant_flag){
        can_pos[*idx].row=tmp_en_passant.row;
        can_pos[*idx].col=tmp_en_passant.col;
        (*idx)++;
    }


    int castling_idx=0;
    coordi tmp_castling[2];
    castling(b,row,col,tmp_castling,&castling_idx);
    if(b->castling_flag){
        for(int i=0;i<castling_idx;i++){
            can_pos[*idx].row=tmp_castling[i].row;
            can_pos[*idx].col=tmp_castling[i].col;
            (*idx)++;
        }
    }
}

void changeTurn(chess_board* b,GAME_INFORMATION* gi){
    b->player_turn=(b->player_turn==WHITE ? BLACK : WHITE);
    if(gi==NULL) return;
    (gi->turn)++;
}

/* special rules */
void promotion(){
    return;
}

void castling(chess_board* b,int row,int col,coordi* can_castling,int* idx){
    if(b->player_turn==WHITE){
        if(row==7 && col==4 && b->castling_check[4]==false){
            if(!b->castling_check[3] && forCastling(b,7,3) && forCastling(b,7,2)&& forCastling(b,7,1)){
                can_castling[*idx].row=7;
                can_castling[*idx].col=1;
                (*idx)++;
            }
            if(!b->castling_check[5] && forCastling(b,7,5) && forCastling(b,7,6)){
                can_castling[*idx].row=7;
                can_castling[*idx].col=6;
                (*idx)++;
            }
        }
    }
    else if(b->player_turn==BLACK){
        if(row==0 && col==4 && b->castling_check[1]==false){
            if(!b->castling_check[0] && forCastling(b,0,3) && forCastling(b,0,2)&& forCastling(b,0,1)){
                can_castling[*idx].row=0;
                can_castling[*idx].col=1;
                (*idx)++;
            }
            if(!b->castling_check[2] && forCastling(b,0,5) && forCastling(b,0,6)){
                can_castling[*idx].row=0;
                can_castling[*idx].col=6;
                (*idx)++;
            }
        }
    }

    if((*idx)==0) b->castling_flag=false;
    else b->castling_flag=true;
}

bool forCastling(chess_board* b,int r, int c){
    int krow,kcol,kid;//king에 대한 정보
    if(b->board[r][c]!=BLANK) return false;//해당 위치에 말이 있는 경우
    if(b->player_turn==WHITE){//W_KING location
        krow=7;
        kcol=4;
    }
    else{//B_KING location
        krow=0;
        kcol=4;
    }

    //해당 위치로 왕 이동
    b->board[r][c]=b->board[krow][kcol];
    kid=b->board[krow][kcol];
    b->board[krow][kcol]=BLANK;

    if(isCheck(b)){//체크가 된다면 원상복구
        b->board[r][c]=BLANK;
        b->board[krow][kcol]=kid;
        return false;
    }
    //원상복구
    b->board[r][c]=BLANK;
    b->board[krow][kcol]=kid;
    return true;
}

void enPassant(chess_board* b,coordi* tmp,int last_row,int last_col){
    if(b->player_turn==WHITE){
        if(b->last_move[0]==1&&b->last_move[2]==3&&b->last_move[4]==B_PAWN){//마지막으로 움직임이 적팀 폰이 앞으로 두칸
            if(last_row==3 && abs(last_col-(b->last_move[3]))==1){
                b->en_passant_flag=true;
                tmp->row=last_row-1;
                tmp->col=b->last_move[3];
                return;
            }
        }
    }
    else if(b->player_turn==BLACK){
        if(b->last_move[0]==6&&b->last_move[2]==4&&b->last_move[4]==W_PAWN){//마지막으로 움직임이 적팀 폰이 앞으로 두칸
            if(last_row==4 && abs(last_col-(b->last_move[3]))==1){
                b->en_passant_flag=true;
                tmp->row=last_row+1;
                tmp->col=b->last_move[3];
                return;
            }
        }
    }
    b->en_passant_flag=false;
}
