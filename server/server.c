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
    for (int i=0; i<MAX_ROOM; i++){
        pr->room[i].room_id = -1;
        pr->room[i].roomfd = -1;
        pr->room[i].max_user_count = -1;
        pr->room[i].cur_user_count = -1;
        pr->room[i].time = -1;
    }
    return sem_init(&pr->mutex, 0, 1);
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
        if(FD_ISSET(listenfd, &pc.ready_set)){
            int connfd = accept(listenfd, (SA*)clientaddr, clientlen);
            add_client_to_pool(&pc, connfd);
            fprintf(stdout, "added client in fd: %d", connfd);
            continue;
        }

        char buf[MAX_LEN];
        char send_string[MAX_LEN];
        for(int i=0; (i<=pc.maxi) && (pc.nready>0); i++){
            int clientfd = pc.clientfd[i];
            int len = read(clientfd, buf, MAX_LEN);

            handle_client(&pc, &pr, &mysql, buf, i, send_string);

            write(clientfd, send_string, MAX_LEN);
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

void parseline(char *buf, char **arguments){
    char *delim;
    int argc = 0;

    while(delim = strchr(buf, '\0')){
        arguments[argc++] = buf;
        buf = delim+1; // search next part
    }

    arguments[argc] = NULL;

}
int handle_client(pool_client *pc, pool_room *pr, MYSQL *mysql, char buf[], int client, char send_string[]){
    char *arguments[5];

    parseline(buf, arguments);
    
    if(!strcmp(buf, "LOG")){
        // arguments 0 = id, 1 = pw
        user_login(mysql, pc, arguments, client);
    }
    else if(!strcmp(buf, "REG")){
        // arguments 0 = id, 1 = pw
        user_register(mysql, arguments);
    }
    else if(!strcmp(buf, "CRE")){
        create_room(pr);
    }
    else if(!strcmp(buf, "FET")){
        fetch_information(pr, send_string);
    }
    else if(!strcmp(buf, "ENT")){
        enter_room();
    }
    else if(!strcmp(buf, "EXT")){
        exit_client(); // 굳이 여기 있을 필요가?
    }

    return TRUE;
}

// login and register -> use mysql db
int user_login(MYSQL *mysql, pool_client *pc, char  **arguments, int client){
    MYSQL_RES *res;
    char buf[256];
    sprintf(buf, "select %s from user", arguments[0]);

    if(mysql_query(mysql, buf) != 0){
        fprintf(stderr, "error has occured on mysql_query in login\n");
    }

    res = mysql_store_result(mysql);
    if(res == NULL){
        fprintf(stderr, "failed to retrieve in mysql_store_result\n");
        return FALSE;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if(!strcmp(row[1], arguments[1])){
        pc->has_login[client] = TRUE;
        strncpy(pc->client_info[client].user_id, row[0], 20);
        return TRUE;
    }
}

int user_register(MYSQL *mysql, char **arguments){
    MYSQL_RES *res;
    char buf[256];
    sprintf(buf, "insert into user_login_info(id, pw, username) values (%s, %s, %s)",arguments[0], arguments[1], arguments[2]);

    if(mysql_query(mysql, buf) != 0){
        fprintf(stderr, "error has occured on mysql_query in register\n");
        return FALSE;
    }
    return TRUE;
}


int create_room(pool_room* pr){
    pthread_t tid;
    // make room and add it to pool
    add_room_to_pool();
    if(pthread_create(&tid, NULL, room_main, NULL) < 0){
        fprintf(stderr, "pthread_create failed\n");
    }
    return TRUE;
}

int fetch_information(pool_room* pr, char send_string[]){
    char *s = send_string;
    int total_len = 0;
    // fetch information from room_pool
    for(int i=0; i<MAX_ROOM; i++){
        if(pr->room[i].room_id != -1){
            sprintf(s, "%d %s %d %d %d %s", pr->room[i].room_id, pr->room[i].name, pr->room[i].max_user_count, pr->room[i].cur_user_count, pr->room[i].time, pr->room[i].address);
        }
    }
    return TRUE;
}

int enter_room(){
    // fetch selected room from room pool
    // cur_user_count < max_user_count -> connect to room

}

int exit_client(){

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