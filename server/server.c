#include "server.h"
#include "room.h"

/* INITIALIZATION AND TERMINATION */
MYSQL *initMysql(){
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

int initClientPool(PoolClient *pc, int listenfd){
    pc->connection_count = 0;
    pc->maxfd = listenfd;

    FD_ZERO(&pc->read_set);
    FD_SET(listenfd, &pc->read_set);

    for(int i = 0; i < MAX_CLIENT; i++){
        pc->clientfd[i] = -1;
        pc->has_login[i] = false;
    }

    return true;
}

int initRoomPool(PoolRoom *pr){
    for (int i = 0; i < MAX_ROOM; i++){
        pr->room[i].room_id = -1;
        pr->room[i].max_user_count = -1;
        pr->room[i].cur_user_count = -1;
        pr->room[i].time = -1;
    }
    return true;
}

void terminateProgram(MYSQL *mysql){
    mysql_close(mysql);
    mysql_library_end(); 
}
/* INITIALIZATION AND TERMINATION */

void addClientToPool(PoolClient *pc, int fd){
    // find appropriate place to put index
    int i;
    (pc->nready)--;
    for(i = 0; i < MAX_CLIENT; i++){
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
    (pc->connection_count)++;

    if(i == MAX_CLIENT)
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

int handleClient(PoolClient *pc, PoolRoom *pr, MYSQL *mysql, char buf[], int clientidx, SendInfo *si){
    char *arguments[32];
    int clientfd = pc->clientfd[clientidx];
    parseline(buf, arguments);
    
    // when sending data delimeter is \n
    if(!strcmp(buf, "LOG")){
        // arguments 0 = id, 1 = pw
        printf("login attempt:\n");
        bool success = userLogin(mysql, pc, arguments, clientidx);
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
        bool success = userRegister(mysql, arguments);
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
        fetchInformation(pr, si);
        si->send_fds[(si->size)++] = clientfd;
        printf("fet\n");
    }
    else if(!strcmp(buf, "CRE")){
        // CRE name maxuser time
        createRoom(pr, arguments, clientfd, si);
        si->send_fds[(si->size)++] = clientfd;
    }
    else if(!strcmp(buf, "ENT")){
        // ENT room_id
        int room_id = atoi(arguments[1]);
        if(enterRoom(pr, room_id, clientfd,clientidx)){
            sprintf(si->send_string, "ENT\nSUC\n%d\n", room_id);
        }
        else{
            sprintf(si->send_string, "ENT\nFAL\n%d\n", room_id);
        }
        si->send_fds[(si->size)++] = clientfd;
        
    }   
    else if (!strcmp(buf, "ROO")){
        // ROO roomid PLY
        roomMain(pc, pr, arguments, clientidx, si);
    }
    
    return true;
}

// login and register -> use mysql db
int userLogin(MYSQL *mysql, PoolClient *pc, char  **arguments, int clientidx){
    MYSQL_RES *res;
    char buf[256];
    sprintf(buf, "select * from user_login_info where id='%s'", arguments[1]);

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
		printf("row[1] :%s\narguments[2] :%s row[2]:%s\n",row[1],arguments[2],row[2]);
        pc->has_login[clientidx] = true;
        strncpy(pc->client_info[clientidx].user_id, row[2], 20);
        return true;
    }
 	printf("return False?\n");
	return false;
}

int userRegister(MYSQL *mysql, char **arguments){
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

int fetchInformation(PoolRoom* pr, SendInfo *si){
    char sbuf[MAX_LEN];
    strncat(si->send_string, "FET\n", 5);
    // fetch information from room_pool
    for(int i = 0; i < MAX_ROOM; i++){
        if(pr->room[i].room_id != -1){
            sprintf(sbuf, "%d\\%s\\%d\\%d\\%d\n", pr->room[i].room_id, pr->room[i].name, pr->room[i].max_user_count, pr->room[i].cur_user_count, pr->room[i].time);
            strncat(si->send_string, sbuf, MAX_LEN);
        }
        
    }

    return true;
}

int createRoom(PoolRoom *pr, char **arguments, int clientfd, SendInfo *si){
    int room_id = addRoomToPool(pr, arguments);
    if(room_id == -1){
        return false;
    }

    char buf[32];
    sprintf(buf, "CRE\nSUC\n%d\n", room_id);
    strncpy(si->send_string, buf, 32);

    return true;
}

int enterRoom(PoolRoom *pr, int room_id, int clientfd,int clientidx){
    // check for errors
    if(pr->room[room_id].room_id == -1 ||
       pr->room[room_id].cur_user_count >= pr->room[room_id].max_user_count){
        return false;
    }
    
    pr->room[room_id].player_fd[pr->room[room_id].cur_user_count] = clientfd;
    pr->room[room_id].player_idx[pr->room[room_id].cur_user_count++]= clientidx;
    return true;
}

int addRoomToPool(PoolRoom *pr, char **arguments){
    for(int i=0; i<MAX_ROOM; i++){
        if(pr->room[i].room_id == -1){
            pr->room[i].room_id = i;
            strncpy(pr->room[i].name, arguments[1], 50);
            pr->room[i].max_user_count = atoi(arguments[2]);
            pr->room[i].cur_user_count = 0;
            pr->room[i].time = atoi(arguments[3])*60;
            return i;
        }
    }
    return -1;
}

int openClientfd(){
    // openClientfd like function
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

    for(p=listp; p != NULL; p=p->ai_next){
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

void deleteClientFromRoom(PoolRoom* pr, SendInfo* si,int clientfd){
    printf("delete client %d\n",clientfd);
    for(int roomidx = 0; roomidx < MAX_ROOM; roomidx++){
        if(pr->room[roomidx].room_id == -1) continue;
        //no one is in a room, deleteroom from PoolRoom
        if(pr->room[roomidx].cur_user_count == 0) {
            pr->room[roomidx].room_id = -1;
        }
        //if one player is in a room, send FIN to the clientfd
        if(pr->room[roomidx].cur_user_count == 2){
            if(pr->room[roomidx].player_fd[0] == clientfd){
                si->send_fds[(si->size)++] = pr->room[roomidx].player_fd[1];
                sprintf(si->send_string,"ROO\nFIN\nP2\n");
                pr->room[roomidx].room_id = -1;
            }
            else if(pr->room[roomidx].player_fd[1] == clientfd){
                si->send_fds[(si->size)++] = pr->room[roomidx].player_fd[0];
                sprintf(si->send_string,"ROO\nFIN\nP1\n");
                pr->room[roomidx].room_id = -1;
            }
            wrappedWriteAll(si);
        }
        else if(pr->room[roomidx].cur_user_count == 1 && pr->room[roomidx].player_fd[0] == clientfd){
            pr->room[roomidx].room_id = -1;
        }
    }
}
