#include "app.h"

extern user * log_in(user * usr);
extern int display_user_list(user * usr);
extern user * find_user_by_name(char * user_name);
extern void my_echo(int destfd, void * ptr, size_t length);
extern int delete_user(const char * del_user_name);

void client(user * connected_usr){

    char buffer[BUFFER_SIZE];
    int nreceived;

    log_in(connected_usr); /*log in */

    do{
        while((nreceived = recv(connected_usr->fildesc,buffer,BUFFER_SIZE,0))>0){
            printf("\n%s: %s\n",connected_usr->user_name,buffer);
            if(display_user_list(connected_usr)<0){
                printf("display_user_list() fail, %s \n", strerror(errno));
                exit(-1);
            }
        }
    }while((nreceived<0) && (errno == EINTR));

    if(nreceived<0){
        printf("recv() fail, %s \n", strerror(errno));
        exit(-1);
    }
    
    if(delete_user(connected_usr->user_name)<0){
        printf("couldn't delete user\n");
    }
    printf("CLOSED\n");
}