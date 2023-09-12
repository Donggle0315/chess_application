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
        chess_board->board[1][col] = B_PAWN;
        chess_board->board[6][col] = W_PAWN;
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
    for(int death_idx = 0; death_idx < PIECE_CNT; death_idx++){
        chess_board->white_death[death_idx] = chess_board->black_death[death_idx] = BLANK;
    }

    /* init player turn */
    chess_board->player_turn = WHITE;

    /* special rule variables */
    chess_board->castling_flag = false;
    chess_board->en_passant_flag = false;
    for(int last_move_idx = 0; last_move_idx < 4; last_move_idx++){
        chess_board->last_move[last_move_idx] = 0;
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

/* Functions which checkes piece's moving and makes the piece move */

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

int movePiece(ChessBoard* chess_board,int start_row,int start_col, int finish_row, int finish_col, bool option){
    /* store death piece's code */
    int death_code = BLANK;

    /* if en_passant is occured */
    if(chess_board->en_passant_flag){
        if((chess_board->board[start_row][start_col] % 10) == 6 && abs(start_col-finish_col) == 1 && chess_board->board[finish_row][finish_col] == BLANK){
            if(chess_board->player_turn == BLACK){
                chess_board->board[chess_board->last_move[2]+1][chess_board->last_move[3]] = chess_board->board[chess_board->last_move[2]][chess_board->last_move[3]];
                chess_board->board[chess_board->last_move[2]][chess_board->last_move[3]] = BLANK;
            }
            else if(chess_board->player_turn == WHITE){
                chess_board->board[chess_board->last_move[2]-1][chess_board->last_move[3]] = chess_board->board[chess_board->last_move[2]][chess_board->last_move[3]];
                chess_board->board[chess_board->last_move[2]][chess_board->last_move[3]] = BLANK;
            }
        }
    }

    /* if castling is occured */
    if(chess_board->castling_flag){
        if(start_row == 0 && start_col == 4 && finish_row == 0 && finish_col == 1){
            chess_board->board[0][2] = chess_board->board[0][0];
            chess_board->board[0][0] = BLANK;
        }
        else if(start_row == 0 && start_col == 4 && finish_row == 0 && finish_col == 6){
            chess_board->board[0][5] = chess_board->board[0][7];
            chess_board->board[0][7] = BLANK;
        }
        else if(start_row == 7 && start_col == 4 && finish_row == 7 && finish_col == 1){
            chess_board->board[7][2] = chess_board->board[7][0];
            chess_board->board[7][0] = BLANK;
        }
        else if(start_row == 7 && start_col == 4 && finish_row == 7 && finish_col == 6){
            chess_board->board[7][5] = chess_board->board[7][7];
            chess_board->board[7][7] = BLANK;
        }
    }

    /* if a piece kills enemy piece, store the dead piece into deathcode variable */
    if(getPieceColor(chess_board->board[finish_row][finish_col]) != BLANK) {
        death_code = chess_board->board[finish_row][finish_col];
    }

    /* move selected piece */
    chess_board->board[finish_row][finish_col] = chess_board->board[start_row][start_col];
    chess_board->board[start_row][start_col] = BLANK;

    /* if option is true, then the board state must be changed so that store changed states */
    if(option){
        chess_board->last_move[0] = start_row;
        chess_board->last_move[1] = start_col;
        chess_board->last_move[2] = finish_row;
        chess_board->last_move[3] = finish_col;
        chess_board->last_move[4] = chess_board->board[finish_row][finish_col];

        if(start_row == 0 && start_col == 0) chess_board->castling_check[0] = true;//LEFT B_ROOK
        else if(start_row == 0 && start_col == 4) chess_board->castling_check[1] = true;//B_KING
        else if(start_row == 0 && start_col == 7) chess_board->castling_check[2] = true;//RIGHT B_ROOK
        else if(start_row == 7 && start_col == 0) chess_board->castling_check[3] = true;//LEFT W_ROOK
        else if(start_row == 7 && start_col == 4) chess_board->castling_check[4] = true;//W_KING
        else if(start_row == 7 && start_col == 7) chess_board->castling_check[5] = true;//RIGHT B_ROOK
    }

    return death_code;
}   

void addDeathPiece(ChessBoard *chess_board, int death_code){
    int enemy_player = (chess_board->player_turn == WHITE) ? BLACK : WHITE;
    if(enemy_player == WHITE){
        chess_board->white_death[chess_board->w_death_idx] = death_code;
        (chess_board->w_death_idx)++;
    }
    else if(enemy_player == BLACK){
        chess_board->black_death[chess_board->b_death_idx] = death_code;
        (chess_board->b_death_idx)++;
    }
}

/* Functions which is checking chess game status */

int getPieceColor(int piece_code){
    if(BLACK < piece_code && piece_code <= B_PAWN) return BLACK;
    if(WHITE < piece_code && piece_code <= W_PAWN) return WHITE;
    return BLANK;
}

void afterMoveCheckPromotion(ChessBoard* chess_board, int finish_row, int finish_col){
    if(chess_board->board[finish_row][finish_col] % 10 == 6){//Pawn 
        int check_row = (chess_board->player_turn==BLACK) ? 7 : 0;//pawn arrived at opposite end place
        if(finish_row == check_row){
            chess_board->promotion_r = finish_row;
            chess_board->promotion_c = finish_col;
            //drawPromotion(chess_board->player_turn); //need : 추가 구현
        }
    }
}

bool isFinish(ChessBoard* chess_board){
    bool finish_flag = true;//if flag is true, then the game is finished
    int current_player = (chess_board->player_turn == WHITE) ? WHITE : BLACK;

    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0; col_idx < COL; col_idx++){
            if(getPieceColor(chess_board->board[row_idx][col_idx]) == current_player){
                /* store movealbe positions of selected piece */
                Coordinate moveable_pos[64];
                int moveable_pos_idx = 0;
                getMoveablePosition(chess_board,row_idx,col_idx,moveable_pos,&moveable_pos_idx);//get moveable position of selected piece
                for(int pos_idx = 0; pos_idx < moveable_pos_idx; pos_idx++){
                    ChessBoard* copy_board = copyBoard(chess_board);//store original board
                    int tmp_death_code = movePiece(chess_board,row_idx,col_idx,moveable_pos[pos_idx].row,moveable_pos[pos_idx].col,false);
                    if(!isCheck(chess_board)) finish_flag = false;//this game is not finished
                    recoverBoard(chess_board,copy_board);//restore original board
                }
                if(!finish_flag) return false;//player can move a piece
            }
        }
    }

    return true;
}

ChessBoard* copyBoard(ChessBoard* chess_board){
    ChessBoard* copy_board = (ChessBoard*)malloc(sizeof(ChessBoard));
    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0; col_idx < COL; col_idx++){
            copy_board->board[row_idx][col_idx] = chess_board->board[row_idx][col_idx];
        }
    }
    for(int idx = 0; idx < PIECE_CNT; idx++){
        copy_board->white_death[idx] = chess_board->white_death[idx];
        copy_board->black_death[idx] = chess_board->black_death[idx];
    }
    copy_board->w_death_idx = chess_board->w_death_idx;
    copy_board->b_death_idx = chess_board->b_death_idx;

    copy_board->black_check = chess_board->black_check;
    copy_board->white_check = chess_board->white_check;
    copy_board->player_turn = chess_board->player_turn;

    copy_board->promotion_r = chess_board->promotion_r;
    copy_board->promotion_c = chess_board->promotion_c;

    for(int idx = 0; idx < 6; idx++){
        copy_board->castling_check[idx] = chess_board->castling_check[idx];
    }
    copy_board->castling_flag = chess_board->castling_flag;
    for(int idx = 0; idx < 5; idx++){
        copy_board->last_move[idx] = chess_board->last_move[idx];
    }
    copy_board->en_passant_flag = chess_board->en_passant_flag;
    copy_board->black_time = chess_board->black_time;
    copy_board->white_time = chess_board->white_time;

    return copy_board;
}

void recoverBoard(ChessBoard* original_board, ChessBoard* chess_board){
    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0; col_idx < COL; col_idx++){
            original_board->board[row_idx][col_idx] = chess_board->board[row_idx][col_idx];
        }
    }
    for(int idx = 0; idx < PIECE_CNT; idx++){
        original_board->white_death[idx] = chess_board->white_death[idx];
        original_board->black_death[idx] = chess_board->black_death[idx];
    }
    original_board->b_death_idx = chess_board->b_death_idx;
    original_board->w_death_idx = chess_board->w_death_idx;

    original_board->black_check = chess_board->black_check;
    original_board->white_check = chess_board->white_check;
    original_board->player_turn = chess_board->player_turn;

    original_board->promotion_r = chess_board->promotion_r;
    original_board->promotion_c = chess_board->promotion_c;

    for(int idx = 0; idx < 6; idx++){
        original_board->castling_check[idx] = chess_board->castling_check[idx];
    }
    original_board->castling_flag = chess_board->castling_flag;
    for(int idx = 0; idx < 5; idx++){
        original_board->last_move[idx] = chess_board->last_move[idx];
    }
    original_board->en_passant_flag = chess_board->en_passant_flag;
    original_board->black_time = chess_board->black_time;
    original_board->white_time = chess_board->white_time;

}

bool isCheck(ChessBoard* chess_board){
    int king_row, king_col;
    int current_player = chess_board->player_turn;
	int enemy_player = (chess_board->player_turn == WHITE) ? BLACK : WHITE;

    /* find current player's king's position */
    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0; col_idx < COL; col_idx++){
            if(chess_board->board[row_idx][col_idx] % 10 == 5 && getPieceColor(chess_board->board[row_idx][col_idx]) == current_player){//자신의 왕일 때
                king_row = row_idx;
                king_col = col_idx;
            }
        }
    }

    /* if any enemy piece can move to current_player's king, then return ture */
    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0;col_idx < COL; col_idx++){
			if(getPieceColor(chess_board->board[row_idx][col_idx]) != enemy_player) continue;
            /* check if enemy's piece can move to king */
            changeTurn(chess_board);
            if(canMove(chess_board,row_idx,col_idx,king_row,king_col)){
				changeTurn(chess_board);
                return true;
            }
            changeTurn(chess_board);
        }
    }

    /* check king doesn't occur */
    return false;
}

void getMoveablePosition(ChessBoard* chess_board, int row, int col, Coordinate* can_pos_list, int* can_pos_idx){
    for(int row_idx = 0; row_idx < ROW; row_idx++){
        for(int col_idx = 0; col_idx < COL; col_idx++){
            /* check if selected piece can move [row_idx][col_idx] and it deosn't make its king be checked */
            if(canMove(chess_board,row,col,row_idx,col_idx)){
                ChessBoard* copy_board = copyBoard(chess_board);
                int tmp_deate_code = movePiece(chess_board,row,col,row_idx,col_idx,false);
                if(!isCheck(chess_board)){
                    /* add moveable position */
                    can_pos_list[*can_pos_idx].row = row_idx;
                    can_pos_list[(*can_pos_idx)++].col = col_idx;
                }
                recoverBoard(chess_board,copy_board);
            }
        }
    }

    /* check if selected piece can do en_passant */
    Coordinate en_passant_pos;
    canEnPassant(chess_board,&en_passant_pos,row,col);
    if(chess_board->en_passant_flag){
        can_pos_list[*can_pos_idx].row = en_passant_pos.row;
        can_pos_list[(*can_pos_idx)++].col = en_passant_pos.col;
    }

    /* check if selected piece can do castling */
    int castling_pos_idx = 0;
    Coordinate castling_pos_list[2];
    getCanCastlingPosition(chess_board,row,col,castling_pos_list,&castling_pos_idx);
    if(chess_board->castling_flag){
        for(int idx = 0; idx < castling_pos_idx; idx++){
            can_pos_list[*can_pos_idx].row = castling_pos_list[idx].row;
            can_pos_list[(*can_pos_idx)++].col = castling_pos_list[idx].col;
        }
    }
}

void changeTurn(ChessBoard* chess_board){
    chess_board->player_turn = (chess_board->player_turn == WHITE ? BLACK : WHITE);
}

/* special rules */
void promotion(){
    return;
}

void getCanCastlingPosition(ChessBoard* chess_board,int row,int col,Coordinate* can_castling_list,int* can_castling_idx){
    /* if castling condition is satisfied, then insert satisfied piece's position to the can_castling_list */
    if(chess_board->player_turn == WHITE){
        if(row == 7 && col ==4  && chess_board->castling_check[4] == false){
            if(!chess_board->castling_check[3] && canCastlingWithoutCheck(chess_board,7,3) && canCastlingWithoutCheck(chess_board,7,2) && canCastlingWithoutCheck(chess_board,7,1)){
                can_castling_list[*can_castling_idx].row = 7;
                can_castling_list[(*can_castling_idx)++].col = 1;
            }
            if(!chess_board->castling_check[5] && canCastlingWithoutCheck(chess_board,7,5) && canCastlingWithoutCheck(chess_board,7,6)){
                can_castling_list[*can_castling_idx].row = 7;
                can_castling_list[(*can_castling_idx)++].col = 6;
            }
        }
    }
    else if(chess_board->player_turn == BLACK){
        if(row == 0 && col == 4 && chess_board->castling_check[1] == false){
            if(!chess_board->castling_check[0] && canCastlingWithoutCheck(chess_board,0,3) && canCastlingWithoutCheck(chess_board,0,2) && canCastlingWithoutCheck(chess_board,0,1)){
                can_castling_list[*can_castling_idx].row = 0;
                can_castling_list[(*can_castling_idx)++].col = 1;
            }
            if(!chess_board->castling_check[2] && canCastlingWithoutCheck(chess_board,0,5) && canCastlingWithoutCheck(chess_board,0,6)){
                can_castling_list[*can_castling_idx].row = 0;
                can_castling_list[(*can_castling_idx)++].col = 6;
            }
        }
    }

    /* update castling_flag in chess_board structure */
    if((*can_castling_idx) == 0) chess_board->castling_flag = false;
    else chess_board->castling_flag = true;
}

bool canCastlingWithoutCheck(ChessBoard* chess_board,int row, int col){
    bool castling_flag = true;
    int king_id, king_row, king_col = 4;//king's information

    /* if [row][col] is already any other piece, cannot do castling */
    if(chess_board->board[row][col] != BLANK) return false;

    /* find king's position */
    if(chess_board->player_turn == WHITE) king_row = 7;
    else king_row = 0;

    /* move king to the [row][col] */
    chess_board->board[row][col] = chess_board->board[king_row][king_col];
    chess_board->board[king_row][king_col] = BLANK;
    king_id = chess_board->board[king_row][king_col];

    /* if moved position makes check, then cannot do castling */
    if(isCheck(chess_board)) castling_flag = false;
    
    /* restore the position */
    chess_board->board[row][col] = BLANK;
    chess_board->board[king_row][king_col] = king_id;

    return castling_flag;
}

void canEnPassant(ChessBoard* chess_board,Coordinate* en_passant_pos,int last_row,int last_col){
    /* check if WHITE's piece satisfy en_passant condition */
    if(chess_board->player_turn == WHITE){
        if(chess_board->last_move[0] == 1 && chess_board->last_move[2] == 3 \
        && chess_board->last_move[4] == B_PAWN && last_row == 3 \
        && abs(last_col - (chess_board->last_move[3])) == 1){
            chess_board->en_passant_flag = true;
            en_passant_pos->row = last_row-1;
            en_passant_pos->col = chess_board->last_move[3];
        }
    }
    /* check if BLACK's piece satisfy en_passant condition */
    else if(chess_board->player_turn == BLACK){
        if(chess_board->last_move[0] == 6 && chess_board->last_move[2] == 4  \
        && chess_board->last_move[4] == W_PAWN && last_row == 4 \
        && abs(last_col - (chess_board->last_move[3])) == 1){
            chess_board->en_passant_flag = true;
            en_passant_pos->row = last_row+1;
            en_passant_pos->col = chess_board->last_move[3];
        }
    }
    else{
        chess_board->en_passant_flag=false;
    }
}

bool isInMoveablePosition(int finish_row,int finish_col,Coordinate *movealbe_pos,int moveable_idx){
    for(int idx = 0; idx < moveable_idx; idx++){
        if(movealbe_pos[idx].row == finish_row && movealbe_pos[idx].col == finish_col) return true;
    }
    return false;
}
