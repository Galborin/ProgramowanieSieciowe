#include "app.h"

typedef struct userElement{
    user * m_user;
    userElement * next;
}userElement;

void log_in(int clifd, const void * ptr,size_t length, user * usr){
    char * msg = "Log in\nGive your name\n";
    if(send(clifd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
    }

    int nreceived;
    do{
        nreceived = recv(clifd,ptr,length,0);
        printf("Name given : %s\n", (char *)ptr);
    }while((nreceived<0) && (errno == EINTR));

}

static void store_user(){
    static userElement * head;

    
}

static user * find_user(){

}