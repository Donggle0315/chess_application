#include "server.h"

int main(){

    // initialize MYSQL struct
    MYSQL *mysql;
    mysql = init_mysql();

    int listenfd = open_clientfd();
    if(listenfd == -1){
        return -1;
    }    

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
        }

        
        for(int i=0; (i<=pc.maxi) && (pc.nready>0); i++){
            char buf[MAX_LEN];
            char send_string[MAX_LEN];
            int clientfd = pc.clientfd[i];
            if(!FD_ISSET(clientfd, &pc.ready_set))
                continue;

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