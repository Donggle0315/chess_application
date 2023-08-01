// include of user h
#include "server.h"
#include "room.h"

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
    int opt = 1;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // address suitable for server
    hints.ai_flags |=  AI_ADDRCONFIG; // returns valid addresses
    hints.ai_flags |= AI_NUMERICSERV; // only accept number ip address

    getaddrinfo(NULL, PORT, &hints, &listp);

    for(p=listp; p != NULL; p->ai_next){
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            break;
        }
        close(listenfd);
    }

    freeaddrinfo(listp);
    if(!p) return -1;

    if(listen(listenfd, 100) < 0){
        close(listenfd);
        return -1;
    }
    


    // accept
    
    pool_client pc;
    pool_room pr;

    init_client_pool(&pc, listenfd);
    init_room_pool(&pr);

    struct sockaddress_storage *clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    int count = 0;
    while(1){
        printf("\ncount: %d\n", count++);
        pc.ready_set = pc.read_set;
        pc.nready = select(pc.maxfd+1, &pc.ready_set, NULL, NULL, NULL);
        // if listenfd
        if(pc.nready < 0){
            // error
            fprintf(stderr, "Error in select\n");
            return -1;
        } 

        // handle events
        if(FD_ISSET(listenfd, &pc.ready_set)){
            int connfd = accept(listenfd, (SA*)clientaddr, &clientlen);
            add_client_to_pool(&pc, connfd);
            fprintf(stdout, "added client in fd: %d\n", connfd);
            continue;
        }

        
        for(int i=0; (i<=pc.maxi) && (pc.nready>0); i++){
            char buf[MAX_LEN];
            char send_string[MAX_LEN];
            int clientfd = pc.clientfd[i];
            int error = readall(clientfd, buf, MAX_LEN);


            memset(send_string, 0, MAX_LEN);
            // closed connection
            if(error == 0){
                FD_CLR(clientfd, &pc.read_set);
                close(clientfd);
                pc.clientfd[i] = -1;
                pc.has_login[i] = -1;
                pc.conn_count--;
                printf("closed connection: %d \n", clientfd);
                continue;
            }
            if(handle_client(&pc, &pr, mysql, buf, clientfd, send_string)){
                writeall(clientfd, send_string, MAX_LEN);
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
    FD_SET(fd, &(pc->read_set));

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
    int len = strlen(buf);
    buf[len-1] = '\n';

    while(delim = strchr(buf, '\n')){
        arguments[argc++] = buf;
        *delim = '\0';
        buf = delim+1; // search next part
    }

    arguments[argc] = NULL;

}
int handle_client(pool_client *pc, pool_room *pr, MYSQL *mysql, char buf[], int client, char send_string[]){
    char *arguments[5];

    parseline(buf, arguments);
    
    // when sending data delimeter is \n
    if(!strcmp(buf, "LOG")){
        // arguments 0 = id, 1 = pw
        printf("login attempt:\n");
        bool success = user_login(mysql, pc, arguments, client);
        if(success){
            printf("success! login\n");
            strcpy(send_string, "SUC\n");
        }
        else{
            printf("failed login!\n");
        }
    }
    else if(!strcmp(buf, "REG")){
        // arguments 0 = id, 1 = pw
        user_register(mysql, arguments);
    }
    else if(!strcmp(buf, "CRE")){
        // CRE name maxuser time
        create_room(pr, arguments, client);
        return FALSE;
    }
    else if(!strcmp(buf, "FET")){
        fetch_information(pr, send_string);
        printf("fet\n");
    }
    else if(!strcmp(buf, "ENT")){
        enter_room(pr, atoi(arguments[1]), send_string);
    }

    return TRUE;
}

// login and register -> use mysql db
int user_login(MYSQL *mysql, pool_client *pc, char  **arguments, int client){
    MYSQL_RES *res;
    char buf[256];
    sprintf(buf, "select id from user_login_info where id='%s'", arguments[1]);

    if(mysql_query(mysql, buf) != 0){
        fprintf(stderr, "%s\n", mysql_error(mysql));
        fprintf(stderr, "error has occured on mysql_query in login\n");
    }

    res = mysql_store_result(mysql);
    if(res == NULL){
        fprintf(stderr, "failed to retrieve in mysql_store_result\n");
        return FALSE;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL){
        fprintf(stderr, "no login info\n");
        return FALSE;
    }
    if(!strcmp(row[1], arguments[2])){
        pc->has_login[client] = TRUE;
        strncpy(pc->client_info[client].user_id, row[0], 20);
        return TRUE;
    }
    
}

int user_register(MYSQL *mysql, char **arguments){
    MYSQL_RES *res;
    char buf[256];
    sprintf(buf, "insert into user_login_info(id, pw, username) values (%s, %s, %s)",arguments[1], arguments[2], arguments[3]);

    if(mysql_query(mysql, buf) != 0){
        fprintf(stderr, "error has occured on mysql_query in register\n");
        return FALSE;
    }
    return TRUE;
}


int create_room(pool_room* pr, char **arguments, int clientfd){
    pthread_t tid;
    // make room and add it to pool

    thread_arg* ta = (thread_arg*)malloc(sizeof(thread_arg));
    ta->pr = pr;
    ta->p1fd = clientfd;
    ta->roomidx = add_room_to_pool(pr, arguments);
    printf("addedroom\n");

    if(pthread_create(&tid, NULL, room_main, (void*)ta) < 0){
        fprintf(stderr, "pthread_create failed\n");
    }
    printf("end create room\n");
    return TRUE;
}

int fetch_information(pool_room* pr, char send_string[]){
    char *s = send_string;
    int total_len = 0;
    
    char sbuf[MAX_LEN];
    sem_wait(&pr->mutex);
    // fetch information from room_pool
    for(int i=0; i<MAX_ROOM; i++){
        if(pr->room[i].room_id != -1){
            sprintf(sbuf, "%d\\%s\\%d\\%d\\%d\\%s:%d\n", pr->room[i].room_id, pr->room[i].name, pr->room[i].max_user_count, pr->room[i].cur_user_count, pr->room[i].time, pr->room[i].address, pr->room[i].port);
            strncat(send_string, sbuf, MAX_LEN);
        }
        
    }
    sem_post(&pr->mutex);
    
    

    return TRUE;
}

int enter_room(pool_room *pr, int idx, char send_string[]){
    // fetch selected room from room pool
    // cur_user_count < max_user_count -> connect to room
    sem_wait(&pr->mutex);
    sprintf(send_string, "ENT\n%s:%d", pr->room[idx].address, pr->room[idx].port);
    sem_post(&pr->mutex);
}

int add_room_to_pool(pool_room *pr, char **arguments){
    sem_wait(&pr->mutex);
    int i;
    for(i=0; i<MAX_ROOM; i++){
        if(pr->room[i].room_id == -1){
            pr->room[i].room_id = i;
            strncpy(pr->room[i].name, arguments[1], 50);
            pr->room[i].max_user_count = atoi(arguments[2]);
            pr->room[i].cur_user_count = 0;
            pr->room[i].time = atoi(arguments[3]);
            sem_post(&pr->mutex);
            return i;
        }
    }

    sem_post(&pr->mutex);
    return 0;
}