#include <sys/select.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include "utility.h"


#define MSG_LEN 2048
#define MAX_CLIENT 256
#define CHESSNET_QUEUE_SIZE 256
#define PRINT_ERROR(err_msg) chessNetPrintError(__FILE__, __LINE__, __func__, err_msg)

typedef struct Msg {
    int fd;
    int msg_len;
    char *msg;
} Msg;

typedef struct ChessNetQueue {
    int head;
    int tail;
    Msg *queue[CHESSNET_QUEUE_SIZE];
} ChessNetQueue;

typedef struct ChessNet {
    int (*client_handler)(int clientfd, char *msg, void *args);
    int (*connection_closed_handler)(int clientfd, void *args);
    int (*accept_handler)(int clientfd, void *args);

    ChessNetQueue *network_queue;

    int maxi;
    int maxfd;

    int clientfd[FD_SETSIZE];      // maps clientfd to idx
    void *client_info[FD_SETSIZE]; // stores void * of struct representing client information
    fd_set read_set;
    fd_set ready_set;

    int listenfd;
} ChessNet;


void chessNetPrintError(const char* file_path, const int line_num, const char* function_name, const char *error){
    fprintf(stderr, "file: %s\nline %d, function:%s : %s\n", file_path, line_num, function_name, error);
}

// copies str
// allocates MSG_LEN size memory. msg_len is stored for future uses. (just in case)
Msg *chessNetCreateMsg(int fd, char *str, size_t msg_len){
    Msg *msg = malloc(sizeof(*msg));
    if(msg == NULL){
        PRINT_ERROR("Msg malloc has failed.");
        return NULL;
    }

    msg->fd = fd;
    msg->msg = malloc(MSG_LEN);
    
    strcpy(msg->msg, str);
    
    msg->msg_len = msg_len;

    return msg;
}

void chessNetFreeMsg(Msg *msg){
    free(msg->msg);
    free(msg);
}


int chessNetEnqueue(ChessNet *chess_net, Msg *msg){
    ChessNetQueue *cq = chess_net->network_queue;
    int next_tail = (cq->tail+1) % CHESSNET_QUEUE_SIZE;
    if(next_tail == cq->head){
        PRINT_ERROR("Enqueue Failed. Max size reached.");
        return -1; 
    }

    cq->queue[cq->tail] = msg;
    cq->tail = next_tail;

    return 0; // returns zero on success
}

// has to free Msg with chessNetFreeMsg
Msg *chessNetDequeue(ChessNet *chess_net){
    ChessNetQueue *cq = chess_net->network_queue;
    if(cq->head == cq->tail){
        PRINT_ERROR("Dequeue Failed. Nothing to dequeue.");
        return NULL;
    }

    int prev = cq->head;
    cq->head = (cq->head+1) % CHESSNET_QUEUE_SIZE;

    return cq->queue[prev];
}

ChessNet *chessNetCreate(){
    ChessNet *chess_net = malloc(sizeof(*chess_net));
    if(chess_net == NULL){
        PRINT_ERROR("ChessNet malloc has failed");
        return NULL;
    }

    chess_net->accept_handler = NULL;
    chess_net->client_handler = NULL;
    chess_net->connection_closed_handler = NULL;

    int maxi = 0;
    int maxfd = 0;

    FD_ZERO(&chess_net->read_set);
    chess_net->ready_set = chess_net->read_set;

    // create queue
    ChessNetQueue *cq = malloc(sizeof(*cq));
    if(cq == NULL){
        return NULL;
    }

    cq->head = 0;
    cq->tail = 0;
    
    chess_net->network_queue = cq;

    int listenfd = -1;

    return chess_net;
}

void chessNetDestroy(ChessNet *chess_net){
    free(chess_net->network_queue);
    free(chess_net);
}

// opens a server on port
int chessNetBindListen(ChessNet *chess_net, const char *port){
    struct addrinfo *listp, *p;
    struct addrinfo hints;
    int listenfd;
    int opt = 1;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // address suitable for server
    hints.ai_flags |=  AI_ADDRCONFIG; // returns valid addresses
    hints.ai_flags |= AI_NUMERICSERV; // only accept number ip address

    if(getaddrinfo(NULL, port, &hints, &listp) < 0){
        PRINT_ERROR("getaddrinfo has failed.");
        return -1;
    }

    for(p=listp; p != NULL; p=p->ai_next){
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
            PRINT_ERROR("setsockopt has failed.");
            return -1;
        }
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
        close(listenfd);
    }

    freeaddrinfo(listp);
    if(p == NULL) return -1;

    if(listen(listenfd, 100) < 0){
        close(listenfd);
        return -1;
    }

    chess_net->listenfd = listenfd;
    FD_SET(listenfd, &chess_net->read_set);

    return listenfd;
}

int chessNetHandleInput(ChessNet *chess_net, void* args){
    // socket hasn't opened yet
    if (chess_net->listenfd == -1){
        PRINT_ERROR("Socket is currently not listening yet.");
        return -1;
    }
    
    int nready;

    chess_net->ready_set = chess_net->read_set;
    if ((nready = select(chess_net->maxfd+1, &chess_net->ready_set, NULL, NULL, NULL)) < 0){
        PRINT_ERROR("Error has occured in Select.");
        return -1;
    }
    
    // call accept on new connection
    if (FD_ISSET(chess_net->listenfd, &chess_net->ready_set)){
        struct sockaddr_storage *clientaddr;
        socklen_t clientaddr_len = sizeof(struct sockaddr_storage);
        int connfd = accept(chess_net->listenfd, (struct sockaddr*)clientaddr, &clientaddr_len);
        
        // find appropriate place to put index
        int i;
        (nready)--;
        for(i=0; i<MAX_CLIENT; i++){
            if(chess_net->clientfd[i] == -1){
                break;
            }
        }

        chess_net->clientfd[i] = connfd;
        chess_net->has_login[i] = false;
        FD_SET(connfd, &(chess_net->read_set));

        if(connfd > chess_net->maxfd)
            chess_net->maxfd = connfd;
        if(i > chess_net->maxi)
            chess_net->maxi = i;
        (chess_net->connection_count)++;

        if(i==MAX_CLIENT)
            fprintf(stderr, "client number reached MAX_CLIENT");

        chess_net->accept_handler(connfd, args);
    }

    for (int i=0; i<chess_net->maxi && nready>0; i++){
        int clientfd = chess_net->clientfd[i];
        if(!FD_ISSET(clientfd, &chess_net->ready_set)){
            continue;
        }

        char buf[MSG_LEN];
        int read_status = readall(clientfd, buf, MSG_LEN);

        if (read_status < 0){
            PRINT_ERROR("Error in readall");
            continue;
        }
        else if (read_status == 0){
            // connection closed
            chess_net->connection_closed_handler(clientfd, args);
        }
        else{
            // handle_client
            chess_net->client_handler(clientfd, buf, args);
        }
    }
    return 1;
}

int chessNetSendMsgs(ChessNet *chess_net){
    Msg *cur_msg;
    while((cur_msg = chessNetDequeue(chess_net)) != NULL){
        int status =  writeall(cur_msg->fd, cur_msg->msg, MSG_LEN);
        if (status < 0){
            return status;
        }
        chessNetFreeMsg(cur_msg);
        cur_msg = NULL;
    }

    return 1;
}

void chessNetSetClientHandler(ChessNet *chess_net, int (*handler)(int, char*, void*)){
    chess_net->client_handler = handler;
}

void chessNetSetConnectionCloseHandler(ChessNet *chess_net, int (*handler)(int, void*)){
    chess_net->connection_closed_handler = handler;
}

void chessNetSetAcceptHandler(ChessNet *chess_net, int (*handler)(int, void*)){
    chess_net->accept_handler = handler;
}
