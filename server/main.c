#include "structures_constants.h"
#include "server.h"
#include "utility.h"

int main(){

    // initialize MYSQL struct
    MYSQL *mysql;
    mysql = initMysql();

    int listenfd = openClientfd();
    if(listenfd == -1){
        return -1;
    }    

    PoolClient pc;
    PoolRoom pr;

    initClientPool(&pc, listenfd);
    initRoomPool(&pr);

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
            addClientToPool(&pc, connfd);
            fprintf(stdout, "added client in fd: %d\n", connfd);
        }

        
        for(int i=0; (i<=pc.maxi) && (pc.nready>0); i++){
            char buf[MAX_LEN];
            int clientfd = pc.clientfd[i];
            if(!FD_ISSET(clientfd, &pc.ready_set))
                continue;

            int error = readall(clientfd, buf, MAX_LEN);

            SendInfo si;
            memset(si.send_string, 0, MAX_LEN);
            si.size = 0;

            // closed connection
            if(error == 0){
                FD_CLR(clientfd, &pc.read_set);
                close(clientfd);
                pc.clientfd[i] = -1;
                pc.has_login[i] = -1;
                pc.connection_count--;
                printf("closed connection: %d \n", clientfd);
				deleteClientFromRoom(&pr,&si,clientfd);
                continue;
            }
            if(handleClient(&pc, &pr, mysql, buf, i, &si)){
                wrappedWriteAll(si);
            }            
        }
    }
    //terminateProgram(mysql);
    return 0;
}
