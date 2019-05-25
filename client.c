/*
piotr
22.05.2019
*/ 

/*includes------------------------------------------------------*/
#include "app.h"
#include "userlist.h"
#include "command_interface.h"

/*imported------------------------------------------------------*/
extern userList_t * UserList;

/*private functions prototypes----------------------------------*/
static int log_in(userList_t * list, user * usr);

/*public functions prototypes-----------------------------------*/
void * client(user * connected_usr);
static int command_proc(int * filedesc, const char * input);

/*function definitions------------------------------------------*/

/*
function to handle clients.
Calls log_in() first.
Then handle client requests.
*/
void * client(user * connected_usr){    
    int nreceived;
    char buffer[BUFFER_SIZE];
    static int test = 0;
    log_in(UserList, connected_usr); /*log in */
    display_user_list(UserList);
    do{
        while((nreceived = recv(*connected_usr->fildesc,buffer,BUFFER_SIZE,0)) > 0){
            printf("%s(%lu): %s\n",connected_usr->user_name,strlen(buffer),buffer);
            command_proc(connected_usr->fildesc,buffer);
        }
    }while((nreceived < 0) && (errno == EINTR));

    if(nreceived < 0){
        printf("recv() fail here, %s \n", strerror(errno));
    }
    
    sleep(1);
    /*delete user*/
    pthread_mutex_lock(UserList->list_mutex);
    listElem_t * delet_user = find_user_by_name(UserList, connected_usr->user_name);
    if(!delet_user){
        printf("couldn't delete user\n");
    }
    if(delete_user(UserList, delet_user)<0){  
        printf("couldn't delete user\n");
    }
    pthread_mutex_unlock(UserList->list_mutex);
    printf("User deleted\n");
    if(display_user_list(UserList) < 0){  
        printf("couldn't display in delete\n");
    }
    printf("CLOSED\n");
    return NULL;
}

/*
send request to client for a name. Add that user to a list.
Return number of elements in list (number of available users).
Return -1 if error.
*/
int log_in(userList_t * list, user * usr){
    char namebuff[BUFFER_SIZE];
    if((!usr) || (!namebuff) || (!list))
        return -1;
    listElem_t * result = NULL;
    char * msg = "Log in\nGive your name\n";
    do{
        if(result)
            msg = "Name is taken\nGive your name\n";

        if(send(*usr->fildesc,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }

        int nreceived;
        do{
            nreceived = recv(*usr->fildesc,namebuff,BUFFER_SIZE,0);
            printf("Name given : %s\n", namebuff);
        }while((nreceived<0) && (errno == EINTR));

        if(nreceived<0){
            printf("recv() fail, %s \n", strerror(errno));
            return -1;
        }

    }while(result = find_user_by_name(UserList,namebuff)); /*check if name is not taken by someone*/
    
    strcpy(usr->user_name,namebuff);
    pthread_mutex_lock(list->list_mutex);
    if(store_element(list, usr) < 0){
        printf("store_element() fail\n");
    };
    pthread_mutex_unlock(list->list_mutex);
    return list->counter;
}

/*
check command list if there is a command with given name.
If so, call its function.
Return 0 if success.
If there is no such command, return -1.
*/
static int command_proc(int * filedesc, const char * input){
    if(!filedesc || !input)
        return -1;
    
    char * space = strchr(input,' ');/*find space in input*/
    if(space)
        *space = '\0'; /*separate command name from args*/

    command * found;
    if(!(found = find_command_by_name(input))){
        return -1; /*there is no such command*/
    }
    else{
        found->func(filedesc,space+1);
        return 0;
    }
}
