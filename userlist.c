/*piotr
22.05.2019
*/

/*includes------------------------------------------------------*/
#include "userlist.h"

/*function definitions------------------------------------------*/

int userList_init(userList_t * list, pthread_mutex_t * mutex){
    if((list==NULL) || (mutex == NULL))
        return -1;
    
    list->head = NULL;
    list->list_mutex = mutex;
    int counter = 0;
    return 0;
}

int store_element(userList_t * list, listElem_t * element){
    if(element!=NULL){
        pthread_mutex_lock(list->list_mutex);
        if(list->head != NULL){
            listElem_t * tmp = list->head;
            while((tmp->next)!=NULL){
                tmp = tmp->next;
            }
            tmp->next = element;
            element->next = NULL;
            element->prev = tmp;
        }
        else{
            list->head = element;
            list->head->next = NULL;
            list->head->prev = NULL;
        }
        pthread_mutex_unlock(list->list_mutex);
        return 0;
    }
    return -1;
}

user * find_user_by_name(userList_t * list, const char * username){
    if((list->head == NULL)||(username == NULL)||(list==NULL))
        return NULL;    

    pthread_mutex_lock(list->list_mutex);
    listElem_t * tmp = list->head;

    while(strcmp(tmp->m_user->user_name, username) != 0){
        tmp = tmp->next;

        if(tmp == NULL){
            return NULL;
        }
    }
    pthread_mutex_unlock(list->list_mutex);
    
    return tmp->m_user;
}

int send_user_list(userList_t * list, int * filedesc){
    if((list == NULL)||(filedesc == NULL))
        return -1;
    
    if(list->head != NULL){
        char address[INET_ADDRSTRLEN+1];
        size_t length = (USER_NAME_LENGTH+INET_ADDRSTRLEN+21)*list->counter+1;
        char all_address_name[length];
        bzero(all_address_name,length);
        
        pthread_mutex_lock(list->list_mutex);
        listElem_t * tmp = list->head;
        while(tmp!=NULL){
            
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
        printf("List is empty\n");
        return 0;
    }
}

int display_user_list(userList_t * list){
    if(list == NULL)
        return -1;
    
    if(list->head != NULL){
        char address[INET_ADDRSTRLEN+1];
        size_t length = (USER_NAME_LENGTH+INET_ADDRSTRLEN+21)*list->counter+1;
        char all_address_name[length];
        bzero(all_address_name,length);
        
        pthread_mutex_lock(list->list_mutex);
        listElem_t * tmp = list->head;
        while(tmp!=NULL){
            
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

int delete_user(userList_t * list, const char * name){

    if((list == NULL)||(name == NULL))
        return -1;

    pthread_mutex_lock(list->list_mutex);
    listElem_t * tmp = list->head;
    
    if(list->head == NULL)
        return -1;

    while(strcmp(tmp->m_user->user_name, name) != 0){
        tmp = tmp->next;

        if(tmp == NULL){
            return -1;
        }
    }
    if(tmp->prev!=NULL)
        tmp->prev->next = tmp->next;
    if(tmp->next!=NULL)
        tmp->next->prev = tmp->prev;
    
    if(tmp==list->head){
        list->head = tmp->next;
    }
    free(tmp->m_user->fildesc);
    free(tmp->m_user);
    free(tmp);
    //tmp=NULL;
    pthread_mutex_unlock(list->list_mutex);
    printf("User deleted\n");
    if(display_user_list(list)<0){  
        printf("couldn't display in delete\n");
    }
}
