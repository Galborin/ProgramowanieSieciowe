#include "app.h"

//#define BUFFER_SIZE 1024

static void * recv_from_server(int * connfd){
    static char buffer[BUFFER_SIZE];
    int n;

    while(1){
        if((n=recv(*connfd,buffer,BUFFER_SIZE,0))<0){
            printf("recv() fail, %s \n", strerror(errno));
            return NULL;
        }
        buffer[n]='\0';
        printf("%s\n",buffer);
    }
    return NULL;
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
    pthread_detach(recv_thread);

    static char msg[BUFFER_SIZE];
    while(1){
        scanf("%s",msg);
        if(send(mysockfd,(const void*)msg,BUFFER_SIZE,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            break;
        }
        else{
            //printf("sended %d \n", (int)strlen(msg)+1);
        }
    }
    
    close(mysockfd);
    printf("\nCLOSING :\n");
    return 0;
}