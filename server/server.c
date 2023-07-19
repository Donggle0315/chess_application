// include of user h
#include "server.h"

/* INITIALIZATION AND TERMINATION */
MYSQL *init_mysql(){
    MYSQL *mysql;
    if(mysql_library_init(0, NULL, NULL)){
        fprintf(stderr, "mysql_library_init failed\n");
        exit(1);
    }

    mysql = mysql_init(NULL);
    if(!mysql){
        fprintf(stderr, "mysql_init failed\n");
        exit(1);
    }

    if(!mysql_real_connect(mysql, MYSQL_HOST, MYSQL_USER, MYSQL_PW, MYSQL_DB, MYSQL_PORT, NULL, 0)){
        fprintf(stderr, "mysql_real_connect failed\n");
        exit(1);
    }

    return mysql;
}

int init_client_pool(pool_client *pc, int listenfd){
    pc->conn_count = 0;
    pc->maxfd = listenfd;

    FD_ZERO(&pc->read_set);
    FD_SET(listenfd, &pc->read_set);

    for(int i=0; i<MAX_CLIENT; i++){
        pc->clientfd[i] = -1;
        pc->has_login[i] = FALSE;
    }

    return TRUE;
}

int init_room_pool(pool_room *pr){
    sem_init(&pr->mutex, 0, 1);
}


void terminate_program(MYSQL *mysql){
    mysql_close(mysql);
    mysql_library_end(); 
}

/* INITIALIZATION AND TERMINATION */

int main(){

    // initialize MYSQL struct
    MYSQL *mysql;
    mysql = init_mysql();

    // open_clientfd like function
    struct addrinfo *listp, *p;
    struct addrinfo hints;
    int listenfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // address suitable for server
    hints.ai_flags |=  AI_ADDRCONFIG; // returns valid addresses
    hints.ai_flags |= AI_NUMERICSERV; // only accept number ip address

    getaddrinfo(NULL, itoa(PORT), &hints, &listp);

    for(p=listp; p != NULL; p->ai_next){
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
        close(listenfd);
    }

    freeaddrinfo(listp);
    if(!p) return -1;

    if(listen(listenfd, 20) < 0){
        close(listenfd);
        return -1;
    }



    // accept
    
    pool_client pc;
    pool_room pr;

    init_client_pool(&pc, listenfd);
    init_room_pool(&pr);

    struct sockaddress_storage *clientaddr;
    int clientlen = sizeof(struct sockaddr_storage);
    while(1){
        pc.ready_set = pc.read_set;
        pc.nready = select(pc.maxfd+1, &pc.ready_set, NULL, NULL, NULL);
        // if listenfd
        if(pc.nready < 0){
            // error
            printf("Error in select\n");
            return;
        } 

        // handle events
        for(int i=0; i<pc.nready; i++){
            if(FD_ISSET(listenfd, &pc.ready_set)){
                int connfd = accept(listenfd, (SA*)clientaddr, clientlen);
                add_client_to_pool(&pc, connfd);
                fprintf(stdout, "added client in fd: %d", connfd);
                continue;
            }
            else{
                handle_clients(&pc, &pr, &mysql);
            }
        }

    }


    terminate_program(mysql);
    return 0;
}

void add_client_to_pool(pool_client *pc, int fd){

    // find appropriate place to put index
    int i;
    (pc->nready)--;
    for(i=0; i<MAX_CLIENT; i++){
        if(pc->clientfd[i] == -1){
            break;
        }
    }

    pc->clientfd[i] = fd;
    pc->has_login[i] = FALSE;
    FD_SET(fd, &pc->read_set);

    if(fd > pc->maxfd)
        pc->maxfd = fd;
    if(i > pc->maxi)
        pc->maxi = i;
    (pc->conn_count)++;

    if(i==MAX_CLIENT)
        fprintf(stderr, "client number reached MAX_CLIENT");
}

int handle_clients(pool_client *pc, pool_room *pr, MYSQL *mysql){
    char data[MAX_LEN];
    int clientfd = event->data.fd;
    int len = read(clientfd, data, MAX_LEN); // change this later to prevent short-counts

    if(!strcmp(data, "LOG")){
        user_login();
    }
    else if(!strcmp(data, "REG")){
        user_register();
    }
    else if(!strcmp(data, "CRE")){
        create_room();
    }
    else if(!strcmp(data, "FET")){
        fetch_information();
    }
    else if(!strcmp(data, "ENT")){
        enter_room();
    }
    else if(!strcmp(data, "EXT")){
        exit_client();
    }
}

// login and register -> use mysql db
void login(){

}

void user_register(){

}


void create_room(){
    pthread_t tid;
    // make room and add it to pool
    add_room_to_pool();
    if(pthread_create(&tid, NULL, room_main, NULL) < 0){
        fprintf(stderr, "pthread_create failed\n");
    }
}

void fetch_information(){
    // fetch information from room_pool
}

void enter_room(){
    // fetch selected room from room pool
    // cur_user_count < max_user_count -> connect to room

}

void exit_client(){

}




void init_room(){
    if(pthread_detach(pthread_self()) < 0){
        fprintf(stderr, "pthread_detach failed\n");
    }
}

void* room_main(void* arg){
    // argument is room_option pointer
    room_option *option = arg;
    init_room();
    
    // open_listenfd related stuff

    struct epoll_event event[MAX_EVENTS];
    int epollfd;
    epollfd = epoll_create(1);
    epoll_ctl(); // add listening descriptor

    int event_count;
    while(1){
        event_count = epoll_wait();
        // if listenfd
        if(event_count < 0){
            // error
            printf("Error in epoll_wait\n");
            return;
        } 

        // handle events
        for(int i=0; i<event_count; i++){
            if(event[i].data.fd == listenfd){
                add_player();
                continue;
            }
            else{
                handle_player(event);
            }
        }

    }
    
    exit_room();
}

void handle_player(struct epoll_event *event){
    char data[MAX_LEN];
    int clientfd = event->data.fd;
    int len = read(clientfd, data, MAX_LEN); // change this later to prevent short-counts

    if(!strcmp(data, "start game")){
        
    }
}