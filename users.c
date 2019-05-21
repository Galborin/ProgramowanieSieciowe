#include "app.h"

/*list element structure*/
typedef struct userElement{
    user * m_user;
    struct userElement * next;
    struct userElement * prev;
}userElement;

/*private functions*/
static void store_user(userElement * element);

/*head of the list */
static userElement * head; 
static int counter = 0;
/*functions definitions-----------------------------------------------------------/

/*
Receive name from user.
Then save user as available to connect by others. 
Return number of client (>=0) if success, otherwise return -1 (error occured).
*/
int log_in(user * usr){
    static char namebuff[USER_NAME_LENGTH];
    userElement * usrelem = (userElement*)malloc(sizeof(userElement));
    usrelem->m_user = usr;

    char * msg = "Log in\nGive your name\n";
    //printf("%s", msg);
    if(send(usrelem->m_user->fildesc,(const void*)msg,strlen(msg)+1,0)<0){
        printf("send() fail, %s \n", strerror(errno));
        return -1;
    }

    int nreceived;
    do{
        nreceived = recv(usrelem->m_user->fildesc,namebuff,USER_NAME_LENGTH,0);
        printf("Name given : %s\n", (char *)namebuff);
    }while((nreceived<0) && (errno == EINTR));

    if(nreceived<0){
        printf("recv() fail, %s \n", strerror(errno));
        return -1;
    }

    strcpy(usrelem->m_user->user_name,namebuff);
    store_user(usrelem);

    return counter++;
}

/*
Delete element from list with name equal to given.
*/
int delete_user(const char * del_user_name){
    userElement * tmp = head;
    
    if(head == NULL)
    return -1;

    while(strcmp(tmp->m_user->user_name, del_user_name) != 0){
        tmp = tmp->next;

        if(tmp == NULL){
            return -1;
        }
    }
    tmp->prev = tmp->next;
    free(tmp);
    return 0;
}

/*
Display list of users.
*/
int display_user_list(user * usr){
    if(head != NULL){
        char address[INET_ADDRSTRLEN+1];
        size_t length = (USER_NAME_LENGTH+INET_ADDRSTRLEN+21)*counter+1;
        char all_address_name[length];
        bzero(all_address_name,length);
        
        userElement * tmp = head;
        while(tmp!=NULL){
            
            inet_ntop(AF_INET,(const struct sockaddr *)&tmp->m_user->user_address.sin_addr,address,INET_ADDRSTRLEN+1);
            strcat(all_address_name,address);
            strcat(all_address_name,"-------------------");
            strncat(all_address_name,tmp->m_user->user_name,strlen(tmp->m_user->user_name)+1);
            strcat(all_address_name,"\n");

            tmp = tmp->next;
        }
        if(send(usr->fildesc,all_address_name,length,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }
        return counter;        
    }
    else{
        printf("List is empty\n");
        return 0;
    }
}

/*

*/
user * find_user_by_name(char * user_name){
    userElement * tmp = head;
    
    if(head == NULL)
    return NULL;

    while(strcmp(tmp->m_user->user_name, user_name) != 0){
        tmp = tmp->next;

        if(tmp == NULL){
            return NULL;
        }
    }

    return tmp->m_user;
}   

/*
Store element at the end of list. If list is empty (head is NULL), head becomes element.
If element is NULL, nothing happens.
*/
static void store_user(userElement * element){
    if(element!=NULL){
        if(head != NULL){
            userElement * tmp = head;
            while((tmp->next)!=NULL){
                tmp = tmp->next;
            }
            tmp->next = element;
            element->next = NULL;
            element->prev = tmp;
        }
        else{
            head = element;
            head->next = NULL;
            head->prev = NULL;
        }
    }
}
