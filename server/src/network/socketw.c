#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socketw.h"
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef DEBUG
#define PRINT_ERROR(err_msg) print_error(__FILE__, __LINE__, __func__, err_msg)
#else
#define PRINT_ERROR(err_msg)
#endif

void print_error(const char *file_path, const int line_num,
                 const char *function_name, const char *error) {
    fprintf(stderr, "file: %s\nline %d, function:%s : %s\n", file_path,
            line_num, function_name, error);
}

SocketW *socketw_create() {
    SocketW *socketw = malloc(sizeof(*socketw));
    if (socketw == NULL) {
        PRINT_ERROR("SocketW malloc has failed");
    }

    socketw->accept_handler = NULL;
    socketw->client_handler = NULL;
    socketw->connection_closed_handler = NULL;

    // init pollfd array
    for (int i = 0; i < MAX_CLIENT; i++) {
        socketw->fds[i].fd = -1;
        socketw->fds[i].events = POLLIN;
        socketw->fds[i].revents = 0;
    }

    // init queue
    SocketWQueue *queue = malloc(sizeof(*queue));
    queue->front = 0;
    queue->back = 0;
    socketw->queue = queue;

    // init fd related
    socketw->maxfds = 0;
    socketw->listenfd = -1;

    return socketw;
}

void socketw_destroy(SocketW **socketw) {
    free((*socketw)->queue);
    (*socketw)->queue = NULL;

    free(*socketw);
    *socketw = NULL;
}

SocketWMsg *socketw_create_msg(int client_num, char *str, size_t msg_len) {
    SocketWMsg *msg = malloc(sizeof(*msg));

    msg->client_num = client_num;
    msg->msg = malloc(msg_len * sizeof(msg->msg));
    strncpy(msg->msg, str, msg_len);
    msg->msg_len = msg_len;

    return msg;
}

void socketw_destroy_msg(SocketWMsg **msg) {
    free((*msg)->msg);
    (*msg)->msg = NULL;
    free(*msg);
    *msg = NULL;
}

size_t socketw_queue_len(SocketW *socketw) {
    int front = socketw->queue->front;
    int back = socketw->queue->back;
    if (front < back) {
        return back - front;
    } else {
        return front - back;
    }
}

SocketWMsg *socketw_queue_front(SocketW *socketw) {
    if (socketw_queue_len(socketw) == 0)
        return NULL;

    return socketw->queue->queue[socketw->queue->front];
}

SocketWMsg *socketw_queue_back(SocketW *socketw) {
    if (socketw_queue_len(socketw) == 0)
        return NULL;

    int back =
        (socketw->queue->back + SOCKETW_QUEUE_SIZE - 1) % SOCKETW_QUEUE_SIZE;
    return socketw->queue->queue[back];
}

int socketw_enqueue(SocketW *socketw, SocketWMsg *msg) {
    if (socketw_queue_len(socketw) == SOCKETW_QUEUE_SIZE - 1) {
        PRINT_ERROR("error: socket queue size exceeded");
        return -1;
    }
    SocketWQueue *queue = socketw->queue;

    int next = (queue->back + 1) % SOCKETW_QUEUE_SIZE;
    queue->queue[queue->back] = msg;
    queue->back = next;

    return 0;
}

SocketWMsg *socketw_dequeue(SocketW *socketw) {
    if (socketw_queue_len(socketw) == 0) {
        PRINT_ERROR("error: nothing to dequeue");
        return NULL;
    }

    SocketWMsg *msg = socketw->queue->queue[socketw->queue->front];
    socketw->queue->front = (socketw->queue->front + 1) % SOCKETW_QUEUE_SIZE;

    return msg;
}

struct addrinfo *_socketw_getaddrinfo(const char *port) {
    struct addrinfo *listp, hints;

    // set hints for server
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(NULL, port, &hints, &listp) < 0) {
        PRINT_ERROR("getaddrinfo has failed");
        return NULL;
    }

    return listp;
}

int _socketw_bind(struct addrinfo **listp) {
    struct addrinfo *p;
    const int opt = 1;
    int listenfd;

    for (p = *listp; p != NULL; p = p->ai_next) {
        // try opening socket
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        // error in opening socket
        if (listenfd < 0)
            continue;

        // man 7 socket
        // bind should allow reuse of local address
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
            0) {
            PRINT_ERROR("setsockopt has failed");
            return -1;
        }
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(listenfd);
    }

    freeaddrinfo(*listp);
    *listp = NULL;

    return listenfd;
}

int socketw_open_server(SocketW *socketw, const char *port) {
    // call getaddrinfo
    struct addrinfo *listp = _socketw_getaddrinfo(port);
    if (listp == NULL)
        return -1;

    // try bind
    int listenfd = _socketw_bind(&listp);
    if (listenfd < 0)
        return -1;

    // try listen
    if (listen(listenfd, 100) < 0) {
        close(listenfd);
        return -1;
    }

    socketw->listenfd = listenfd;

    return listenfd;
}

/*



int chessNetAddToConnectionPool(SocketW *chess_net, int connfd){
    // find appropriate place to put index
    int i;
    for(i=0; i<MAX_CLIENT; i++){
        if(chess_net->clientfd[i] == -1){
            break;
        }
    }

    if(i==MAX_CLIENT){
        PRINT_ERROR("client number reached MAX_CLIENT");
    }
    else{
        chess_net->clientfd[i] = connfd;
        FD_SET(connfd, &(chess_net->read_set));
    }

    if(connfd > chess_net->maxfd)
        chess_net->maxfd = connfd;

    if(i > chess_net->maxi)
        chess_net->maxi = i;

    return i;
}

int chessNetHandleInput(SocketW *chess_net, void *args){
    // socket hasn't opened yet
    if (chess_net->listenfd == -1){
        PRINT_ERROR("Socket is currently not listening yet.");
        return -1;
    }

    int nready;

    chess_net->ready_set = chess_net->read_set;
    if ((nready = select(chess_net->maxfd+1, &chess_net->ready_set, NULL, NULL,
NULL)) < 0){ PRINT_ERROR("Error has occured in Select."); return -1;
    }

    // call accept on new connection
    if (FD_ISSET(chess_net->listenfd, &chess_net->ready_set)){
        (nready)--;
        struct sockaddr_storage *clientaddr;
        socklen_t clientaddr_len = sizeof(struct sockaddr_storage);
        int connfd = accept(chess_net->listenfd, (struct sockaddr*)clientaddr,
&clientaddr_len); if(connfd < 0){ PRINT_ERROR("Error in accept");
        }

        int client_num = chessNetAddToConnectionPool(chess_net, connfd);

        // Calls callback function on accept
        chess_net->accept_handler(client_num, args);
    }

    for (int client_num=0; client_num<chess_net->maxi && nready>0;
client_num++){ int clientfd = chess_net->clientfd[client_num];
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
            chess_net->connection_closed_handler(client_num, args);
        }
        else{
            // handle_client
            chess_net->client_handler(client_num, buf, args);
        }
    }
    return 1;
}

int chessNetSendMsgs(SocketW *chess_net){
    SocketWMsg *cur_msg;
    while((cur_msg = chessNetDequeue(chess_net)) != NULL){
        int fd = chess_net->clientfd[cur_msg->client_num];
        int status =  writeall(fd, cur_msg->msg, MSG_LEN);
        if (status < 0){
            return status;
        }
        chessNetFreeMsg(cur_msg);
        cur_msg = NULL;
    }

    return 1;
}

void chessNetSetClientHandler(SocketW *chess_net, int (*handler)(int, char*,
void*)){ chess_net->client_handler = handler;
}

void chessNetSetConnectionCloseHandler(SocketW *chess_net, int (*handler)(int,
void*)){ chess_net->connection_closed_handler = handler;
}

void chessNetSetAcceptHandler(SocketW *chess_net, int (*handler)(int, void*)){
    chess_net->accept_handler = handler;
}*/
