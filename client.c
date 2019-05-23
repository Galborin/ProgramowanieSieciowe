/*
piotr
22.05.2019
*/ 

/*includes------------------------------------------------------*/
#include "app.h"
#include "userlist.h"

/*imported------------------------------------------------------*/
extern userList_t UserList;

/*local global variables----------------------------------------*/
static char buffer[BUFFER_SIZE];

/*private functions prototypes----------------------------------*/
static int log_in(userList_t * list, user * usr, char * namebuff, size_t size);

/*public functions prototypes-----------------------------------*/
void * client(user * connected_usr);

/*function definitions------------------------------------------*/

/*
function to handle clients.
Calls log_in() first.
Then handle client requests.
*/
void * client(user * connected_usr){    
    int nreceived;
    log_in(&UserList, connected_usr, buffer, BUFFER_SIZE); /*log in */
    display_user_list(&UserList);
    do{
        while((nreceived = recv(*connected_usr->fildesc,buffer,BUFFER_SIZE,0))>0){
            printf("\n%s(%lu): %s\n",connected_usr->user_name,strlen(buffer),buffer);
            if(send_user_list(&UserList,connected_usr->fildesc)<0){
                printf("send_user_list() fail, %s \n", strerror(errno));
               return NULL;
            }
        }
    }while((nreceived<0) && (errno == EINTR));

    if(nreceived<0){
        printf("recv() fail, %s \n", strerror(errno));
        return NULL;
    }
    
    sleep(2);
    if(delete_user(&UserList, connected_usr->user_name)<0){  
        printf("couldn't delete user\n");
    }
    printf("CLOSED\n");
    return NULL;
}

/*
send request to client for a name. Add that user to a list.
Return number of elements in list (number of available users).
Return -1 if error.
*/
int log_in(userList_t * list, user * usr, char * namebuff, size_t size){
    if((usr==NULL)||(namebuff==NULL)||(list==NULL))
        return -1;

    listElem_t * usrelem = (listElem_t*)malloc(sizeof(listElem_t));
    usrelem->m_user = usr;

    char * msg = "Log in\nGive your name\n";
    //printf("%s", msg);
    if(send(*usrelem->m_user->fildesc,(const void*)msg,strlen(msg)+1,0)<0){
        printf("send() fail, %s \n", strerror(errno));
        return -1;
    }

    int nreceived;
    do{
        nreceived = recv(*usrelem->m_user->fildesc,namebuff,size,0);
        printf("Name given : %s\n", namebuff);
    }while((nreceived<0) && (errno == EINTR));

    if(nreceived<0){
        printf("recv() fail, %s \n", strerror(errno));
        return -1;
    }

    strcpy(usrelem->m_user->user_name,namebuff);
    store_element(list, usrelem);

    return list->counter++;
}
