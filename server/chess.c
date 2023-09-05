#include "chess.h"
#include "room.h"

ChessBoard* initBoard(){
    /* dynamic allocate chess board */
    ChessBoard* chess_board = (ChessBoard*)malloc(sizeof(ChessBoard));
    
    /* fill with chess pieces */
    for(int row = 2; row < ROW-2; row++){
        for(int col = 0; col < COL; col++){
            chess_board->board[row][col] = BLANK;
        }
    }
    for(int col = 0; col < COL; col++){
        b->board[1][col] = B_PAWN;
        b->board[6][col] = W_PAWN;
    }
    chess_board->board[0][0] = chess_board->board[0][7] = B_ROOK;
    chess_board->board[0][1] = chess_board->board[0][6] = B_KNIGHT;
    chess_board->board[0][2] = chess_board->board[0][5] = B_BISHOP;
    chess_board->board[0][3] = B_QUEEN;
    chess_board->board[0][4] = B_KING;

    chess_board->board[7][0] = chess_board->board[7][7] = W_ROOK;
    chess_board->board[7][1] = chess_board->board[7][6] = W_KNIGHT;
    chess_board->board[7][2] = chess_board->board[7][5] = W_BISHOP;
    chess_board->board[7][3] = W_QUEEN;
    chess_board->board[7][4] = W_KING;
    
    /* init array which store death pieces */
    for(int death_idx = 0; death_idx < PEICE_CNT; death_idx++){
        chess_board->white_death[death_idx] = chess_board->black_death[death_idx] = BLANK;
    }

    /* init player turn */
    chess_board->player_turn = WHITE;

    /* special rule variables */
    chess_board->castling_flag = false;
    chess_board->en_passant_flag = false;
    for(int last_move_idx = 0; last_move_idx < 4; last_move_idx++){
        chess_board->last_move[last_move_idx] = false;
    }

    return chess_board;
}

ChessBoard* resetGame(ChessBoard* chess_board){
    /* finish chess game */
    finishGame(chess_board);

    /* re-initialize chess board */
    ChessBoard* new_chess_board = initBoard();

    return new_chess_board;
}

void finishGame(ChessBoard* chess_board){
    free(chess_board);
}

/* 말의 이동을 확인하고 이동시키는 함수들 */

bool handlePawn(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    int move_row = finish_row-start_row;
    int move_col = finish_col-start_col;

    if(chess_board->player_turn == WHITE){ //if white pawn will move
        if(move_row == -1 && abs(move_col) == 1 && getPieceColor(chess_board->board[finish_row][finish_col]) == BLACK) return true;//attack digonally
        if(chess_board->board[finish_row][finish_col] != BLANK) return false;//already in position
        if(start_row == 6 && move_col == 0 && move_row == -2 && chess_board->board[start_row-1][start_col] == BLANK) return true;//can move forward two space when first moving
        if(move_col == 0 && move_row == -1) return true;//move forward a space
    }

    if(chess_board->player_turn == BLACK){ //if black pawn will move
        if(move_row == 1 && abs(move_col) == 1 && getPieceColor(chess_board->board[finish_row][finish_col]) == WHITE) return true;//attack digonally
        if(chess_board->board[finish_row][finish_col] != BLANK) return false;//already in position
        if(start_row == 1 && move_col == 0 && move_row == 2 && chess_board->board[start_row+1][start_col] == BLANK) return true;//can move forward two space when first moving
        if(move_col == 0 && move_row == 1) return true;//move forward a space
    }

    /* selected pawn cannot move anywhere */
    return false;
}

bool handleRook(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    int move_row = finish_row-start_row;
    int move_col = finish_col-start_col;

    /* check if there is any piece on the route */
    if(abs(move_row) != 0 && move_col == 0){//move vertically
        if(start_row < finish_row){
            for(int row_idx = start_row+1; row_idx < finish_row; row_idx++){
                if(chess_board->board[row_idx][start_col] != BLANK) return false;
            }
        }
        else{
            for(int row_idx = start_row-1; row_idx > finish_row; row_idx--){
                if(chess_board->board[row_idx][start_col] != BLANK) return false;
            }
        }
    } 
    else if(abs(move_col) != 0 && move_row == 0){//move horizonally
        if(start_col < finish_col){
            for(int col_idx = start_col+1; col_idx < finish_col; col_idx++){
                if(chess_board->board[start_row][col_idx] != BLANK) return false;
            }
        }
        else{
            for(int col_idx = start_col-1; col_idx > finish_col; col_idx--){
                if(chess_board->board[start_row][col_idx] != BLANK) return false;
            }
        }
    }
   
    return true;
}

bool handleKnight(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    int move_row = finish_row-start_row;
    int move_col = finish_col-start_col;

    if(abs(move_row) == 2 && abs(move_col) == 1 ) return true;
    if(abs(move_row) == 1 && abs(move_col) == 2 ) return true;
    return false;
}

bool handleBishop(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    int move_row=finish_row-start_row;
    int move_col=finish_col-start_col;

    /* don't move diagonally */
    if(move_row == 0 || move_col == 0 || abs(move_row) != abs(move_col)) return false;

    if(start_row < finish_row && start_col < finish_col){//right down
        for(int row_idx = start_row+1,col_idx = start_col+1; row_idx < finish_row; row_idx++,col_idx++){
            if(chess_board->board[row_idx][col_idx] != BLANK) return false;
        }
    }
    else if(start_row < finish_row && start_col > finish_col){//left down
        for(int row_idx = start_row+1,col_idx = start_col-1; row_idx < finish_row; row_idx++,col_idx--){
            if(chess_board->board[row_idx][col_idx] != BLANK) return false;
        }
    }
    else if(start_row > finish_row && start_col < finish_col){//right up
        for(int row_idx = start_row-1,col_idx = start_col+1; row_idx > finish_row; row_idx--,col_idx++){
            if(chess_board->board[row_idx][col_idx] != BLANK) return false;
        }
    }
    else if(start_row > finish_row && start_col > finish_col){//left up
        for(int row_idx = start_row-1,col_idx = start_col-1; row_idx > finish_row; row_idx--,col_idx--){
            if(chess_board->board[row_idx][col_idx] != BLANK) return false;
        }
    }

    return true;
}

bool handleQueen(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    /* check move diagonally by handleBishop and move horizonally and vertically by handleRook */
    return handleBishop(chess_board,start_row,start_col,finish_row,finish_col) || handleRook(chess_board,start_row,start_col,finish_row,finish_col);
}

bool handleKing(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    int move_row = finish_row-start_row;
    int move_col = finish_col-start_col;

    if(abs(move_row) == 1 && move_col == 0) return true;//move one piece vertically
    if(move_row == 0 && abs(move_col) == 1) return true;//move one piece horizonally
    if(abs(move_row) == 1 && abs(move_col) == 1) return true;//move one piece diagonally

    return false;
}

bool canMove(ChessBoard* chess_board, int start_row, int start_col, int finish_row, int finish_col){
    /* check selected pos is out of board */
    if(start_row < 0 || start_row >= ROW || start_col < 0 || start_col >= COL || finish_row < 0 || finish_row >= ROW || finish_col < 0 || finish_col >= COL) return false;
    /* check selected piece is legal */
    if(getPieceColor(chess_board->board[start_row][start_col]) != chess_board->player_turn) return false;
    if(getPieceColor(chess_board->board[finish_row][finish_col]) == chess_board-> player_turn) return false;
    if(getPieceColor(chess_board->board[start_row][start_col]) == BLANK) return false;
    /* check if selected piece can move */
    switch((chess_board->board[start_row][start_col]) % 10){
        case 1 ://ROOK
            return handleRook(chess_board,start_row,start_col,finish_row,finish_col);
        case 2 ://KNIGHT
            return handleKnight(chess_board,start_row,start_col,finish_row,finish_col);
        case 3 ://BISHOP
            return handleBishop(chess_board,start_row,start_col,finish_row,finish_col);
        case 4 ://QUEEN
            return handleQueen(chess_board,start_row,start_col,finish_row,finish_col);
        case 5 ://KING
            return handleKing(chess_board,start_row,start_col,finish_row,finish_col);
        case 6://PAWN
            return handlePawn(chess_board,start_row,start_col,finish_row,finish_col);
    }
    
    return false;
}

int movePiece(chess_board* b,int start_row,int start_col, int finish_row, int finish_col, bool option){
    int deathCode=BLANK;
    if(b->en_passant_flag){//앙파상인 경우
        if((b->board[start_row][start_col]%10)==6 && abs(start_col-finish_col)==1 && b->board[finish_row][finish_col]==BLANK){
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
        if(start_row==0 && start_col==4 && finish_row==0 && finish_col==1){
            b->board[0][2]=b->board[0][0];
            b->board[0][0]=BLANK;
        }
        else if(start_row==0 && start_col==4 && finish_row==0 && finish_col==6){
            b->board[0][5]=b->board[0][7];
            b->board[0][7]=BLANK;
        }
        else if(start_row==7 && start_col==4 && finish_row==7 && finish_col==1){
            b->board[7][2]=b->board[7][0];
            b->board[7][0]=BLANK;
        }
        else if(start_row==7 && start_col==4 && finish_row==7 && finish_col==6){
            b->board[7][5]=b->board[7][7];
            b->board[7][7]=BLANK;
        }
    }

    //말을 잡은 경우 deathCode에 해당 말 정보 등록
    if(getPieceColor(b->board[finish_row][finish_col])!=BLANK) deathCode=b->board[finish_row][finish_col];

    //말 이동
    b->board[finish_row][finish_col]=b->board[start_row][start_col];
    b->board[start_row][start_col]=BLANK;

    if(option){//캐슬링 확인
        b->last_move[0]=start_row;
        b->last_move[1]=start_col;
        b->last_move[2]=finish_row;
        b->last_move[3]=finish_col;
        b->last_move[4]=b->board[finish_row][finish_col];

        if(start_row==0 && start_col==0) b->castling_check[0]=true;//LEFT B_ROOK
        else if(start_row==0 && start_col==4) b->castling_check[1]=true;//B_KING
        else if(start_row==0 && start_col==7) b->castling_check[2]=true;//RIGHT B_ROOK
        else if(start_row==7 && start_col==0) b->castling_check[3]=true;//LEFT W_ROOK
        else if(start_row==7 && start_col==4) b->castling_check[4]=true;//W_KING
        else if(start_row==7 && start_col==7) b->castling_check[5]=true;//RIGHT B_ROOK
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

void afterMove(chess_board* b,int finish_row, int finish_col){
    if(b->board[finish_row][finish_col]%10==6){//도착한 말이 폰일 경우
        int check_row=(b->player_turn==BLACK) ? 7 : 0;
        if(finish_row==check_row){
            b->promotion_r=finish_row;
            b->promotion_c=finish_col;
            //drawPromotion(b->player_turn); //need : 추가 구현
        }
    }
}


void printB(chess_board* b){
	
}
bool isFinish(chess_board* b){
    bool flag=true;
    int current_player=(b->player_turn==WHITE)?WHITE:BLACK;
	//int our_king = (b->player_turn==BLACK)?B_KING:W_KING;
    for(int i=0;i<ROW;i++){
        for(int j=0;j<COL;j++){
            if(getPieceColor(b->board[i][j])==current_player){
                coordi tmp_pos[64];
                int tmp_idx=0;
                //changeTurn(b);
                getMoveablePosition(b,i,j,tmp_pos,&tmp_idx);
                for(int k=0;k<tmp_idx;k++){
                    chess_board* tmp_board=copyBoard(b);
                    int tmp=movePiece(b,i,j,tmp_pos[k].row,tmp_pos[k].col,false);
					//if(b->board[tmp_pos[k].row][tmp_pos[k].col] == our_king ) flag=true;
                    if(!isCheck(b)) flag=false;
                    recover_board(b,tmp_board);
                }
                //changeTurn(b);
                if(!flag) return false;
            }
        }
    }
    return true;s
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
    tmp->p2_time=b->p2_time;
    tmp->p1_time=b->p1_time;

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
    tmp->p2_time=b->p2_time;
    tmp->p1_time=b->p1_time;

}

bool isCheck(chess_board* b){
    int k_row, k_col;
    int color=b->player_turn;
	int enemy_color=(b->player_turn == WHITE) ? BLACK : WHITE;
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
			if(getPieceColor(b->board[i][j]) != enemy_color) continue;
            changeTurn(b);
            if(canMove(b,i,j,k_row,k_col)){
				changeTurn(b);
                return true;
            }
            changeTurn(b);
        }
    }
    return false;
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

void changeTurn(chess_board* b){
    b->player_turn=(b->player_turn==WHITE ? BLACK : WHITE);
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


bool isInMoveablePosition(int finish_row,int finish_col,coordi *movealbe_pos,int moveable_idx){
    for(int i=0;i<moveable_idx;i++){
        if(movealbe_pos[i].row==finish_row && movealbe_pos[i].col==finish_col) return true;
    }
    return false;
}