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

void init_client_pool(){

}

void init_room_pool(){
    sem_init(POOL_ROOM.mutex,1);
}

void init_program(){
    // connect with db

    init_client_pool();
    init_room_pool();
}

void terminate_program(MYSQL *mysql){
    mysql_close(mysql);
    mysql_library_end(); 
}

/* INITIALIZATION AND TERMINATION */

int main(){

    // initialize MYSQL struct
    MYSQL *mysql;
    init_program();
    mysql = init_mysql();

    // open_clientfd like function
    
    // accept
    
    int epollfd;
    struct epoll_event event[MAX_EVENTS];

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
                add_client_to_pool();
                continue;
            }
            else{
                handle_clients();
            }
        }

    }


    terminate_program(mysql);
    return 0;
}

void handle_clients(struct epoll_event *event){
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