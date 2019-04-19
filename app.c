#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>


int main(){
    int mysockfd, clisockfd;
    struct sockaddr_in myaddress, cliaddress;
    socklen_t cliaddresslength;
    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(5454);
    myaddress.sin_addr.s_addr = INADDR_ANY;


    if((mysockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() fail: %s \n", strerror(errno));
        return 1;
    }

    int reuse = 1;
    if(setsockopt(mysockfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&reuse,sizeof(reuse))<0){
        printf("setsockopt() fail: %s \n", strerror(errno));
        return 1;
    }

    if(bind(mysockfd,(struct sockaddr *)&myaddress,sizeof(myaddress))<0){
        printf("bind() fail: %s \n", strerror(errno));    
        return 1;
    }

    if(listen(mysockfd,2)<0){
        printf("listen() fail, %s \n", strerror(errno));
        return 1;
    }

    while(1){
        if((clisockfd=accept(mysockfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
            return 1;
        }

        char * msg = "HELLO";
        if(send(clisockfd,(const void*)msg,strlen(msg),0)<0){
            printf("send() fail, %s \n", strerror(errno));
        }
        else{
            printf("sended %d \n", (int)strlen(msg));
        }
        close(clisockfd);
    }

    

    return 0;
}