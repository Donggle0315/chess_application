#include <mysql/mysql.h>
#include <stdbool.h>

#include "network.h"
#include "server.h"





int main(){
    MYSQL *mysql;
    mysql = initMysql();

    ChessNet *chess_net = chessNetCreate();

    chessNetBindListen(chess_net, "12345");

    while(true){
        chessNetHandleInput(chess_net, args);
    }

    chessNetDestroy(chess_net);
    return 0;
}
