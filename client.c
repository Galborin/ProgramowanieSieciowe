/*
piotr
22.05.2019
*/ 

/*includes------------------------------------------------------*/
#include "app.h"
#include "userlist.h"
#include "command_interface.h"
#include "chatroom.h"

/*imported------------------------------------------------------*/
extern userList_t GlobalUserList;
extern chatList_t GlobalChatList;

/*private functions prototypes----------------------------------*/
static int log_in(userList_t * list, user_t * usr);

/*public functions prototypes-----------------------------------*/
void * client(user_t * connected_usr);
static int command_proc(user_t * usr, char * input);

/*function definitions------------------------------------------*/

/*
function to handle clients.
Calls log_in() first.
Then handle client requests.
Delete user_t if connection lost.
*/
void * client(user_t * connected_usr){    
    int nreceived;
    char buffer[BUFFER_SIZE];
    /*log in */
    if(log_in(&GlobalUserList, connected_usr) < 0){ 
        printf("log_in() fail\n\r");
        return NULL;
    } 
    display_user_list(&GlobalUserList);
    int namelen = strlen(connected_usr->user_name);
    strcpy(buffer, connected_usr->user_name);
    buffer[namelen] = ':';
    chatListElem_t * user_chatroom = NULL;
    do{
        while((nreceived = recv(*connected_usr->fildesc,buffer + namelen + 1,BUFFER_SIZE,0)) > 0){
            printf("%s\n",buffer);
            if((command_proc(connected_usr,buffer + namelen + 1)) > 0)
                printf("Command called successfully\n\r");
            else{
                /*check if chatroom is available*/
                if(connected_usr->chatroom_name){
                    if((user_chatroom = chList_find_chatroom_by_name(&GlobalChatList, connected_usr->chatroom_name))){
                        send_to_all(user_chatroom->m_chatroom->chat_userlist, buffer);
                    }
                }
                else if(send(*connected_usr->fildesc,"NOT CONNECTED TO ANY CHAT\n\r",28,0) < 0){
                    printf("send() fail, %s \n", strerror(errno));
                }
            }  
        }
    }while((nreceived<0) && ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)));

    if(nreceived < 0){
        printf("recv() fail here, %s \n", strerror(errno));
    }
    

    /*delete user_t*/
    pthread_mutex_lock(&GlobalUserList.list_mutex);
    listElem_t * delet_user = find_user_by_name(&GlobalUserList, connected_usr->user_name);
    if(!delet_user){
        printf("couldn't delete user_t - not found\n");
    }
    if(delete_user(&GlobalUserList, delet_user)<0){  
        printf("couldn't delete user_t - delete_user() fail\n");
    }
    pthread_mutex_unlock(&GlobalUserList.list_mutex);
    printf("User deleted\n");
    if(display_user_list(&GlobalUserList) < 0){  
        printf("display_user_list() fail()\n");
    }
    printf("CLOSED\n");
    return NULL;
}

/*
send request to client for a name. Add that user_t to a list.
Return number of elements in list (number of available users).
Return -1 if error.
*/
int log_in(userList_t * list, user_t * usr){
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
        }while((nreceived<0) && ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)));

        if(nreceived<0){
            printf("recv() fail, %s \n", strerror(errno));
            return -1;
        }
        
        pthread_mutex_lock(&GlobalUserList.list_mutex);
        result = find_user_by_name(list,namebuff);
        pthread_mutex_unlock(&GlobalUserList.list_mutex);

    }while(result); /*check if name is not taken by someone*/
    
    strcpy(usr->user_name,namebuff);
    pthread_mutex_lock(&list->list_mutex);
    if(store_element(list, usr) < 0){
        printf("store_element() fail\n");
    };
    pthread_mutex_unlock(&list->list_mutex);
    return list->counter;
}

/*
check command list if there is a command with given name.
If so, call its function.
Return integer (output from found command) if success.
If there is no such command, return -1.
*/
static int command_proc(user_t * usr, char * input){
    if(!usr || !input)
        return -1;
    
    char * space = strchr(input,' ');/*find space in input*/
    if(space)
        *space = '\0'; /*separate command name from args*/

    command * found;
    if(!(found = find_command_by_name(input))){
        if(space)
            *space = ' ';
        return -1; /*there is no such command*/
    }
    else{
        int result;
        //if(space)
        //    printf("called command %s by user %s with args %s\n\r",input,usr->user_name,space + 1);
        //else
        //    printf("called command %s by user %s\n\r",input,usr->user_name);
        if((space)  &&  (strlen(space + 1) > 0)) 
            result = found->func(usr,(void *)(space + 1)); //there are args to pass
        else
            result = found->func(usr,NULL); //there are no args to pass

        return result;
    }
}
