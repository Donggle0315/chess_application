#include "utility.h"


// reads all content from fd
// until you read count bytes
// puts into buf
int readall(int fd, char *buf, int count){

    char *ptr = buf;

    int bytes_read = 0;

    while (bytes_read < count) {

        int cur_bytes_read = read(fd, ptr, count-bytes_read);
        
        // an error happened
        if(cur_bytes_read <= 0){
            return cur_bytes_read;
        }
        
        ptr += cur_bytes_read; // move pointer to match bytes read
        bytes_read += cur_bytes_read;

        printf("%d\n", bytes_read);
    }

    return 1;
}

// writes all content from fd
// until you write count bytes from buf
int writeall(int fd, char *buf, int count){
    int bytes_written = 0;

    while (bytes_written < count) {
        int cur_bytes_written = write(fd, buf + bytes_written, count-bytes_written);
        
        // an error happened
        if(cur_bytes_written < 0){
            return cur_bytes_written;
        }
        bytes_written += cur_bytes_written;
    }

    return 1;
}