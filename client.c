#include "app.h"

struct sockaddr * client(int clifd, void * ptr,size_t length){
    char * msg = "Give peer adress\n";
    if(send(clifd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
    }

    int nreceived;
    do{
        nreceived = recv(clifd,ptr,length,0);
        printf("Peer given : %s\n", (char *)ptr);
    }while((nreceived<0) && (errno == EINTR));
    




}