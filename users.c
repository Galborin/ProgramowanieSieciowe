#include "app.h"

typedef struct userElement{
    user m_user;
    struct userElement * next;
    struct userElement * prev;
}userElement;

static void store_user(userElement * element);
int delete_user(const char * del_user_name);
void display_user_list();
static user * find_user_by_name();

static userElement * head; /*head of the list */

user * log_in(int clifd, struct sockaddr_in * usraddr){
    char namebuff[USER_NAME_LENGTH];

    char * msg = "Log in\nGive your name\n";
    printf("%s\n", msg);
    if(send(clifd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
    }

    int nreceived;
    do{
        nreceived = recv(clifd,namebuff,USER_NAME_LENGTH,0);
        printf("Name given : %s\n", (char *)namebuff);
    }while((nreceived<0) && (errno == EINTR));

    userElement * usrelem = (userElement*)malloc(sizeof(userElement));
    strcpy(usrelem->m_user.user_name,namebuff);
    usrelem->m_user.user_address = *usraddr;
    store_user(usrelem);
    return &usrelem->m_user;
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

/*
Delete element from list with name equal to given.
*/
int delete_user(const char * del_user_name){
    userElement * tmp = head;
    
    if(head == NULL)
    return -1;

    while(strcmp(tmp->m_user.user_name, del_user_name) != 0){
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
void display_user_list(){
    if(head != NULL){
        char * address;
        userElement * tmp = head;
        while(tmp!=NULL){
            printf("\n%s  :  %s\n", tmp->m_user.user_name, \
            inet_ntop(AF_INET,(const struct sockaddr *)&tmp->m_user.user_address.sin_addr,address,INET_ADDRSTRLEN+1));
            tmp = tmp->next;
        }
    }
    else{
        printf("List is empty\n");
    }
}


static user * find_user_by_name(){

}   