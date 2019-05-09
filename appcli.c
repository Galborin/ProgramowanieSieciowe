#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include<unistd.h>

#define BUFFER_SIZE 1024


int main(int argc, char**argv){
    int mysockfd;
    struct sockaddr_in myaddress, srvaddress;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    srvaddress.sin_family = AF_INET;
    srvaddress.sin_port = htons(5454);
    inet_pton(AF_INET,(const char*)argv[1],&srvaddress.sin_addr);

    if((mysockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() error: %s\n", strerror(errno));
        return 1;
    }

    if(connect(mysockfd,(const struct sockaddr*)&srvaddress,sizeof(srvaddress))<0){
        printf("connect() error: %s\n", strerror(errno));
        return 1;
    }

    char msg[100];
    while((n=recv(mysockfd,buffer,BUFFER_SIZE,0))>0){
        buffer[n]='\0';
        printf("%s\n",buffer);

        scanf("%s",msg);
        if(send(mysockfd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
        }
        else{
            printf("sended %d \n", (int)strlen(msg)+1);
        }
    }
    

    return 0;
}