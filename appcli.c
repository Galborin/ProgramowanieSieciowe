/*#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <pthread.h>*/
#include "app.h"

#define BUFFER_SIZE 1024

static int * recv_from_server(int * connfd){
    char buffer[BUFFER_SIZE];
    int n;
    int * result = (int *)malloc(sizeof(int));
    
    while(1){
        if((n=recv(*connfd,buffer,BUFFER_SIZE,0))<0){
            printf("recv() fail, %s \n", strerror(errno));
            *result = -1;
            return result;
        }
        buffer[n]='\0';
        printf("%s\n",buffer);
    }
    
    *result = 0;
    return result;
}

int main(int argc, char**argv){
    int mysockfd;
    struct sockaddr_in myaddress, srvaddress;
    ssize_t n;
    pthread_t recv_thread;

    srvaddress.sin_family = AF_INET;
    srvaddress.sin_port = htons(5454);
    inet_pton(AF_INET,(const char*)argv[1],&srvaddress.sin_addr);

    if((mysockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() error: %s\n", strerror(errno));
        return 1;
    }
    printf("Connecting to server...\n");
    if(connect(mysockfd,(const struct sockaddr*)&srvaddress,sizeof(srvaddress))<0){
        printf("connect() error: %s\n", strerror(errno));
        return 1;
    }
    printf("Success!\n");
    
    pthread_create(&recv_thread,NULL, (void*(*)(void *))recv_from_server, (void *)&mysockfd);
    //int * recv_from_server_result;
    //pthread_join(recv_thread, (void **)&recv_from_server_result);
    pthread_detach(recv_thread);
    //printf("Result : %i", *recv_from_server_result);
    char msg[100];
    while(1){
        scanf("%s",msg);
        if(send(mysockfd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
        }
        else{
            //printf("sended %d \n", (int)strlen(msg)+1);
        }
    }
    
    printf("\nCLOSING :\n\n");
    return 0;
}