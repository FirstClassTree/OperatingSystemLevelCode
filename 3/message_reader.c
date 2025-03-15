
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include "message_slot.h"


int main(int argc,char *argv[]){
    if (argc != 3){
        perror("invalid command line arguments");
        exit(1);
    }
    int targetChannel = atoi(argv[2]);
    char buffer[BUF_LEN];
    int fd = open(argv[1],O_RDONLY);
    if( 0 > fd){
        perror("failed to open");
        exit(1);
    }
    if( 0 > ioctl(fd,MSG_SLOT_CHANNEL,targetChannel)){
        perror("failed to set channel");
        exit(1);
    }
    int message_len = read(fd,buffer,BUF_LEN);
    if(message_len < 0){
        perror("failed to read");
        exit(1);
    }
    close(fd);
    if(message_len != write(1,buffer,message_len)){
        perror("failed to write to stdout");
        exit(1);
    }
    exit(0);
    

}