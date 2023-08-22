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
        pc->has_login[i] = false;
    }

    return true;
}

int init_room_pool(pool_room *pr){
    for (int i=0; i<MAX_ROOM; i++){
        pr->room[i].room_id = -1;
        pr->room[i].max_user_count = -1;
        pr->room[i].cur_user_count = -1;
        pr->room[i].time = -1;
    }
    return true;
}

void terminate_program(MYSQL *mysql){
    mysql_close(mysql);
    mysql_library_end(); 
}
/* INITIALIZATION AND TERMINATION */

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
    pc->has_login[i] = false;
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

    while((delim = strchr(buf, '\n'))){
        arguments[argc++] = buf;
        *delim = '\0';
        buf = delim+1; // search next part
    }

    arguments[argc] = NULL;

}

int handle_client(pool_client *pc, pool_room *pr, MYSQL *mysql, char buf[], int clientfd, send_info *si){
    char *arguments[32];

    parseline(buf, arguments);
    
    // when sending data delimeter is \n
    if(!strcmp(buf, "LOG")){
        // arguments 0 = id, 1 = pw
        printf("login attempt:\n");
        bool success = user_login(mysql, pc, arguments, clientfd);
        if(success){
            printf("success! login\n");
            si->send_fds[(si->size)++] = clientfd;
            strcpy(si->send_string, "LOG\nSUC\n");
        }
        else{
            si->send_fds[(si->size)++] = clientfd;
            strcpy(si->send_string, "LOG\nFAL\n");
            printf("failed login!\n");
        }
    }
    else if(!strcmp(buf, "REG")){
        // arguments 0 = id, 1 = pw
        bool success = user_register(mysql, arguments);
        if(success){
            printf("register success\n");
            si->send_fds[(si->size)++] = clientfd;
            strcpy(si->send_string, "REG\nSUC\n");
        }
        else{
            strcpy(si->send_string, "REG\nFAL\n");
            printf("failed login!\n");
        }
    }
    else if(!strcmp(buf, "FET")){
        fetch_information(pr, si);
        si->send_fds[(si->size)++] = clientfd;
        printf("fet\n");
    }
    else if(!strcmp(buf, "CRE")){
        // CRE name maxuser time
        create_room(pr, arguments, clientfd, si);
        si->send_fds[(si->size)++] = clientfd;
    }
    else if(!strcmp(buf, "ENT")){
        // ENT room_id
        int room_id = atoi(arguments[1]);
        if(enter_room(pr, room_id, clientfd)){
            sprintf(si->send_string, "ENT\nSUC\n%d\n", room_id);
        }
        else{
            sprintf(si->send_string, "ENT\nFAL\n%d\n", room_id);
        }
        si->send_fds[(si->size)++] = clientfd;
        
    }   
    else if (!strcmp(buf, "ROO")){
        // ROO roomid PLY
        room_main(pr, arguments, clientfd, si);
    }


    

    return true;
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
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL){
        fprintf(stderr, "no login info\n");
        return false;
    }
    if(strlen(row[1]) == strlen(arguments[2]) && !strcmp(row[1], arguments[2])){
        pc->has_login[client] = true;
        strncpy(pc->client_info[client].user_id, row[0], 20);
        return true;
    }
    
}

int user_register(MYSQL *mysql, char **arguments){
    MYSQL_RES *res;
    char buf[256];
    sprintf(buf, "insert into user_login_info(id, pw, username) values ('%s', '%s', '%s')",arguments[1], arguments[2], arguments[3]);

    if(mysql_query(mysql, buf) != 0){
        fprintf(stderr, "error has occured on mysql_query in register\n");
        fprintf(stderr, "%s\n", mysql_error(mysql));
        return false;
    }
    return true;
}

int fetch_information(pool_room* pr, send_info *si){
    char sbuf[MAX_LEN];
    strncat(si->send_string, "FET\n", 5);
    // fetch information from room_pool
    for(int i=0; i<MAX_ROOM; i++){
        if(pr->room[i].room_id != -1){
            sprintf(sbuf, "%d\\%s\\%d\\%d\\%d\n", pr->room[i].room_id, pr->room[i].name, pr->room[i].max_user_count, pr->room[i].cur_user_count, pr->room[i].time);
            strncat(si->send_string, sbuf, MAX_LEN);
        }
        
    }
    return true;
}

int create_room(pool_room *pr, char **arguments, int clientfd, send_info *si){
    int room_id = add_room_to_pool(pr, arguments);
    if(room_id == -1){
        return false;
    }

   

    char buf[32];
    sprintf(buf, "CRE\nSUC\n%d\n", room_id);
    strncpy(si->send_string, buf, 32);
    return true;
}

int enter_room(pool_room *pr, int room_id, int clientfd){
    // check for errors
    if(pr->room[room_id].room_id == -1 ||
       pr->room[room_id].cur_user_count >= pr->room[room_id].max_user_count){
        return false;
    }
    
    pr->room[room_id].player_fd[pr->room[room_id].cur_user_count++] = clientfd;
    return true;
}

int add_room_to_pool(pool_room *pr, char **arguments){
    int i;
    for(i=0; i<MAX_ROOM; i++){
        if(pr->room[i].room_id == -1){
            pr->room[i].room_id = i;
            strncpy(pr->room[i].name, arguments[1], 50);
            pr->room[i].max_user_count = atoi(arguments[2]);
            pr->room[i].cur_user_count = 0;
            pr->room[i].time = atoi(arguments[3]);
            return i;
        }
    }
    return -1;
}

int open_clientfd(){
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
    return listenfd;
}
