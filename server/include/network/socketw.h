#ifndef CHESS_NET
#define CHESS_NET

#include <sys/poll.h>
#include <sys/socket.h>

#define MSG_LEN 2048
#define MAX_CLIENT 256
#define SOCKETW_QUEUE_SIZE 256


typedef struct {
    int client_num;
    int msg_len;
    char *msg;
} SocketWMsg;


typedef struct {
    int front;
    int back;
    SocketWMsg *queue[SOCKETW_QUEUE_SIZE];
} SocketWQueue;


typedef struct {
    int (*client_handler)(int clientfd, char *msg, void *args);
    int (*connection_closed_handler)(int clientfd, void *args);
    int (*accept_handler)(int clientfd, void *args);

    struct pollfd fds[MAX_CLIENT];
    int maxfds;
    int listenfd;

    SocketWQueue *queue;

    void *client_info[FD_SETSIZE]; // stores void * of struct representing client information

} SocketW;

// socketw create, destroy
SocketW *socketw_create();
void socketw_destroy(SocketW **socketw);

// socketw server
int socketw_open_server(SocketW *socketw, const char *port);

// msg create, destroy
SocketWMsg *socketw_create_msg(int client_num, char *str, size_t msg_len);
void socketw_destroy_msg(SocketWMsg **msg);

// msg queue related
size_t socketw_queue_len(SocketW *socketw);
SocketWMsg *socketw_queue_front(SocketW *socketw);
SocketWMsg *socketw_queue_back(SocketW *socketw);
int socketw_enqueue(SocketW *socketw, SocketWMsg *msg);
SocketWMsg *socketw_dequeue(SocketW *socketw);

// set handlers
void socketw_set_client_handler(SocketW *socketw, int (*handler)(int, char*, void*));
void socketw_set_connection_close_handler(SocketW *socketw);
void socketw_set_accept_handler(SocketW *socketw);




int socketw_bind_listen(SocketW *socketw, const char *port);
int socketw_add_to_connection_pool(SocketW *socketw, int connfd);
int chessNetHandleInput(SocketW *socketw, void *args);
int chessNetSendMsgs(SocketW *socketw);
void chessNetSetClientHandler(SocketW *socketw, int (*handler)(int, char*, void*));
void chessNetSetConnectionCloseHandler(SocketW *socketw, int (*handler)(int, void*));
void chessNetSetAcceptHandler(SocketW *socketw, int (*handler)(int, void*));








#endif
