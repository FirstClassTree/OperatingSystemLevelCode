
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


#include "message_slot.h"


int main(int argc,char *argv[]){
    if (argc != 4){
        perror("invalid command line arguments");
        exit(1);
    }
    int targetChannel = atoi(argv[2]);
    int message_len = strlen(argv[3]);
    int fileDesc = open(argv[1],O_WRONLY);
    if ( 0 > fileDesc){
        perror("failure in opening file");
        exit(1);
    }
    if(0 > ioctl(fileDesc,MSG_SLOT_CHANNEL,targetChannel) ){
        perror("failure in calling ioctl");
        close(fileDesc);
        exit(1);
    }
    if(0> write(fileDesc,argv[3],message_len)){
        perror("failure to write");
        exit(1);
    }
    close(fileDesc);
    exit(0);
}