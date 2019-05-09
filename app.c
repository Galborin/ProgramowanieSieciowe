#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

static int send_echo(int destfd,const void * ptr,int n);
static void my_echo(int destfd,void * ptr,size_t length);

int main(){
    int listenfd, connectfd;
    struct sockaddr_in myaddress, cliaddress;
    char address[INET_ADDRSTRLEN+1];
    socklen_t cliaddresslength;
    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(5454);
    myaddress.sin_addr.s_addr = INADDR_ANY;

    if((listenfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() fail: %s \n", strerror(errno));
        return 1;
    }

    int reuse = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&reuse,sizeof(reuse))<0){
        printf("setsockopt() fail: %s \n", strerror(errno));
        return 1;
    }

    if(bind(listenfd,(struct sockaddr *)&myaddress,sizeof(myaddress))<0){
        printf("bind() fail: %s \n", strerror(errno));    
        return 1;
    }

    if(listen(listenfd,2)<0){
        printf("listen() fail, %s \n", strerror(errno));
        return 1;
    }

    int n = 0;
    while(1){
        if((connectfd=accept(listenfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
            return 1;
        }
        else{
            inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
            printf("Connection from: %s\n",address);
        }

        char * msg = "HELLO";
        if(send(connectfd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
        }
        else{
            printf("sended Hello Message %d \n", (int)strlen(msg)+1);
        }

        char buffer[BUFFER_SIZE];

        my_echo(connectfd,buffer,BUFFER_SIZE);

        close(connectfd);
    }
    return 0;
}

static int send_echo(int destfd,const void * ptr,int n){
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

static void my_echo(int destfd,void * ptr,size_t length){
    int nreceived;
    
    again:
    while((nreceived = recv(destfd,ptr,BUFFER_SIZE,0))>0){
        printf("\nnreceived: %d\n",nreceived);
        if(send_echo(destfd,ptr,nreceived)<0){
                printf("send_echo() fail, %s \n", strerror(errno));
                close(destfd);
        }
    }
    if((nreceived<0) && (errno == EINTR)){//back to the loop
        goto again;
    }
}