
#include "app.h"

int send_echo(int destfd,const void * ptr,int n){
    int ntosend = n;
    int sended = 0;
    while(ntosend>0){
        if((sended = send(destfd,ptr,ntosend,0))<=0){
            if((sended < 0) && (errno == EINTR)){
                sended = 0;
            }
            else
            {
                printf("send() fail, %s \n", strerror(errno));
                return(-1);
            }
        }
        printf("sended= %i \n", sended);
        ntosend-=sended;
    }
    printf("ntosend after= %d \n", ntosend);
    return ntosend;
}

void my_echo(int destfd,void * ptr,size_t length){
    int nreceived;   
    do{
        while((nreceived = recv(destfd,ptr,BUFFER_SIZE,0))>0){
            printf("\nnreceived: %d\n",nreceived);
            if(send_echo(destfd,ptr,nreceived)<0){
                printf("send_echo() fail, %s \n", strerror(errno));
                close(destfd);
            }
        }
    }while((nreceived<0) && (errno == EINTR));

    if(nreceived<0){
        printf("recv() fail, %s \n", strerror(errno));
        exit(-1);
    }
}