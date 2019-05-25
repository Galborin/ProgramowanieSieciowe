/*piotr
22.05.2019
*/

/*includes------------------------------------------------------*/
#include "userlist.h"

/*function definitions------------------------------------------*/

int userList_init(userList_t * list, pthread_mutex_t * mutex){
    if((!list) || (!mutex))
        return -1;
    
    list->head = NULL;
    list->list_mutex = mutex;
    int counter = 0;
    return 0;
}

/*int store_element(userList_t * list, user * usr){
    pthread_mutex_lock(list->list_mutex);
    if((!usr) || (!list))
        return -1;
    
    if(list->head){
        listElem_t * tmp = list->head;
        while(tmp->next){
            tmp = tmp->next;
        }
        tmp->next = (listElem_t *)malloc(sizeof(listElem_t));
        tmp->next->next = NULL;
        tmp->next->m_user = usr;
    }
    else{
        list->head = (listElem_t *)malloc(sizeof(listElem_t));
        list->head->m_user = usr;
        list->head->next = NULL;    
    }
    list->counter++;
    pthread_mutex_unlock(list->list_mutex);
    return 0;
}*/

int store_element(userList_t * list, user * usr){
    if((!usr) || (!list))
        return -1;
    
    if(list->head){
        list->head->prev = (listElem_t *)malloc(sizeof(listElem_t));
        list->head->prev->prev = NULL;
        list->head->prev->next = list->head;
        list->head->prev->m_user = usr;
        list->head = list->head->prev;  
    }
    else{
        list->head = (listElem_t *)malloc(sizeof(listElem_t));
        list->head->prev = NULL;
        list->head->next = NULL;
        list->head->m_user = usr;
    }
    list->counter++;    
    return 0;
}

listElem_t * find_user_by_name(userList_t * list, const char * username){
    if((!list->head) || (!username) || (!list))
        return NULL;    

    listElem_t * tmp = list->head;

    while(strcmp(tmp->m_user->user_name, username) != 0){
        tmp = tmp->next;

        if(!tmp){
            return NULL;
        }
    }
    
    return tmp;
}

int send_user_list(userList_t * list, int * filedesc){
    if((!list) || (!filedesc))
        return -1;
    
    if(list->head){
        char address[INET_ADDRSTRLEN+1];
        size_t length = (USER_NAME_LENGTH+INET_ADDRSTRLEN+21)*list->counter+1;
        char all_address_name[length];
        bzero(all_address_name,length);
        pthread_mutex_lock(list->list_mutex);
        listElem_t * tmp = list->head;
        while(tmp){
            
            inet_ntop(AF_INET,(const struct sockaddr *)&tmp->m_user->user_address.sin_addr,address,INET_ADDRSTRLEN+1);
            strcat(all_address_name,address);
            strcat(all_address_name,"-------------------");
            strncat(all_address_name,tmp->m_user->user_name,strlen(tmp->m_user->user_name)+1);
            strcat(all_address_name,"\n");

            tmp = tmp->next;
        }
        pthread_mutex_unlock(list->list_mutex);
        if(send(*filedesc,all_address_name,length,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }
        return list->counter;        
    }
    else{
        char * msg = "List is empty\n";
         if(send(*filedesc,msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }
        return 0;
    }
}

int display_user_list(userList_t * list){
    if(!list)
        return -1;
    
    if(list->head){
        char address[INET_ADDRSTRLEN+1];
        size_t length = (USER_NAME_LENGTH+INET_ADDRSTRLEN+21)*list->counter+1;
        char all_address_name[length];
        bzero(all_address_name,length);
        
        pthread_mutex_lock(list->list_mutex);
        listElem_t * tmp = list->head;
        while(tmp){
            
            inet_ntop(AF_INET,(const struct sockaddr *)&tmp->m_user->user_address.sin_addr,address,INET_ADDRSTRLEN+1);
            strcat(all_address_name,address);
            strcat(all_address_name,"-------------------");
            strncat(all_address_name,tmp->m_user->user_name,strlen(tmp->m_user->user_name)+1);
            strcat(all_address_name,"\n");

            tmp = tmp->next;
        }
        pthread_mutex_unlock(list->list_mutex);
        printf("\n%s\n",all_address_name);
        return list->counter;        
    }
    else{
        printf("List is empty\n");
        return 0;
    }
}

int delete_user(userList_t * list, listElem_t * elem){

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
    
    free(elem->m_user->fildesc);
    free(elem->m_user);
    free(elem);
    list->counter--;
}

