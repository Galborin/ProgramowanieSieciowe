/*
piotr
04.06.2019
*/

/*includes------------------------------------------------------*/
#include "chatroom.h"

/*function definitions------------------------------------------*/

int chList_init(chatList_t * list, pthread_mutex_t * mutex){
    if((!list) || (!mutex))
        return -1;
    
    list->head = NULL;
    list->list_mutex = mutex;
    
    return 0;
}

int chRoom_create_and_join(user_t * usr, chatList_t * list, char * ch_name){
    if((!usr) || (!list) || (!ch_name))
        return -1;

    /*create new chatroom*/
    chatroom_t * new_chtrm = (chatroom_t *)malloc(sizeof(chatroom_t));
    
    /*set name*/
    if(strlen(ch_name) > CHAT_NAME_LENGTH){
        printf("name too long\n\r");
        return -1;
    }    
    strcpy(new_chtrm->chatname,ch_name);

    /*create chatroom's user list*/
    userList_t * new_userlist = (userList_t *)malloc(sizeof(userList_t));

    if(userList_init(new_userlist) < 0){
        printf("init user list fail\n\r");
        return -1;
    }
        
    new_chtrm->chat_userlist = new_userlist;

    chatListElem_t * newelem;

    pthread_mutex_lock(list->list_mutex);

    if(!(newelem = chList_add_element(list,new_chtrm))){
        printf("chList_add_element() fail\n\r");
        return -1;
    }
        
    int result = chList_join_chatroom(list, newelem, usr);
    if((result) < 0){
        printf("chList_join_chatroom() fail\n\r");
    }
    pthread_mutex_unlock(list->list_mutex);
    
    return result;
    
}

chatListElem_t * chList_add_element(chatList_t * list, chatroom_t * chtrm){
    if((!chtrm) || (!list))
        return NULL;
    
    if(list->head){
        list->head->prev = (chatListElem_t *)malloc(sizeof(chatListElem_t));
        list->head->prev->prev = NULL;
        list->head->prev->next = list->head;
        list->head->prev->m_chatroom = chtrm;
        list->head = list->head->prev;  
    }
    else{
        list->head = (chatListElem_t *)malloc(sizeof(chatListElem_t));
        list->head->prev = NULL;
        list->head->next = NULL;
        list->head->m_chatroom = chtrm;
    }
    list->counter++;    
    return list->head;
}

chatListElem_t * chList_find_chatroom_by_name(chatList_t * list, const char * chatroomname){
   if((!list->head) || (!chatroomname) || (!list))
        return NULL;    

    chatListElem_t * tmp = list->head;

    while(strcmp(tmp->m_chatroom->chatname, chatroomname) != 0){
        tmp = tmp->next;

        if(!tmp){
            return NULL;
        }
    }
    
    return tmp;
}

int chList_send_list(chatList_t * list, user_t * usr){
    if((!list) || (!usr))
        return -1;
    
    pthread_mutex_lock(list->list_mutex);
    if(list->head){
        size_t length = (CHAT_NAME_LENGTH + 1)*list->counter+18;
        char all_name[length];
        bzero(all_name,length);

        chatListElem_t * tmp = list->head;
        strcat(all_name,"\n\r--CHATROOMS--\n\r");
        while(tmp){

            strncat(all_name,tmp->m_chatroom->chatname,strlen(tmp->m_chatroom->chatname)+1);
            strcat(all_name,"\n");

            tmp = tmp->next;
        }
        pthread_mutex_unlock(list->list_mutex);
        strcat(all_name,"\r");
        if(send(*usr->fildesc,all_name,length,0) < 0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }
        return list->counter;        
    }
    else{
        pthread_mutex_unlock(list->list_mutex);
        char * msg = "List is empty\n";
         if(send(*usr->fildesc,msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }
        return 0;
    }
}

int chList_display_list(chatList_t * list){

    if(!list)
        return -1;

    pthread_mutex_lock(list->list_mutex);
    if(list->head){
        size_t length = (CHAT_NAME_LENGTH + 1)*list->counter+1;
        char all_name[length];
        bzero(all_name,length);
        
        chatListElem_t * tmp = list->head;
        while(tmp){

            strncat(all_name,tmp->m_chatroom->chatname,strlen(tmp->m_chatroom->chatname)+1);
            strcat(all_name,"\n");

            tmp = tmp->next;
        }
        pthread_mutex_unlock(list->list_mutex);

        printf("\n%s\n",all_name);
        return list->counter;        
    }
    else{
        pthread_mutex_unlock(list->list_mutex);
        printf("List is empty\n");
        return 0;
    }
}

int chList_delete_chatroom(chatList_t * list, chatListElem_t * elem){
    
    if((!list) || (!elem))
        return -1;

    if(!list->head)
        return -1;

    if(elem == list->head){
        if(list->head->next){
            list->head = list->head->next;
            list->head->prev = NULL;
        }
        else
            list->head = NULL;
    }
    else{
        if(elem->next){
            elem->prev->next = elem->next;
            elem->next->prev = elem->prev;
        }  
        else
            elem->prev->next = NULL;
    }
    
    free(elem->m_chatroom);
    free(elem);
    return --list->counter;
}

int chList_join_chatroom(chatList_t * list, chatListElem_t * elem, user_t * usr){
    
    if((!list) || (!elem) || (!usr))
        return -1;
    
    int n_users;

    pthread_mutex_lock(&elem->m_chatroom->chat_userlist->list_mutex);

    /*check if user already is in chatroom*/
    if(find_user_by_name(elem->m_chatroom->chat_userlist,usr->user_name)){
        pthread_mutex_unlock(&elem->m_chatroom->chat_userlist->list_mutex);
        return -1;
    }

    if (store_element(elem->m_chatroom->chat_userlist, usr) < 0){
        pthread_mutex_unlock(&elem->m_chatroom->chat_userlist->list_mutex);
        return -1;
    }

    /*get number of users in user list*/
    n_users = elem->m_chatroom->chat_userlist->counter;

    pthread_mutex_unlock(&elem->m_chatroom->chat_userlist->list_mutex);

    /*set as new user's chatroom*/
    usr->chatroom_name = elem->m_chatroom->chatname;

    return n_users;
    
}

int chList_leave_chatroom(chatList_t * list, chatListElem_t * elem, listElem_t * usr_elem){
    if((!list) || (!elem) || (!usr_elem))
        return -1;
    
    int n_users;

    pthread_mutex_lock(&elem->m_chatroom->chat_userlist->list_mutex);
    
    if (delete_user(elem->m_chatroom->chat_userlist, usr_elem) < 0){
        pthread_mutex_unlock(&elem->m_chatroom->chat_userlist->list_mutex);
        return -1;
    }

    /*remove from user*/
    usr_elem->m_user->chatroom_name = NULL;

    /*get number of users in user list*/
    n_users = elem->m_chatroom->chat_userlist->counter;

    pthread_mutex_unlock(&elem->m_chatroom->chat_userlist->list_mutex);
    free(usr_elem);
    
    if(n_users < 1){
        pthread_mutex_lock(list->list_mutex);
        if(chList_delete_chatroom(list,elem) < 0){
            printf("Failed to remove an empty chatroom\n\r");
        }
        pthread_mutex_unlock(list->list_mutex);
    }

    return n_users;
}
