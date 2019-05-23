/*
piotr
22.05.2019
*/

#ifndef _APP
#define _APP

/*includes------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

/*defines------------------------------------------------------*/
#define BUFFER_SIZE 1024
#define LISTENQ 10
#define USER_NAME_LENGTH 20

/*user structure*/
typedef struct user{
    char user_name[USER_NAME_LENGTH];
    struct sockaddr_in user_address;
    int * fildesc;
}user;

/*list element structure*/
typedef struct listElem_t{
    user * m_user;
    struct listElem_t * next;
    struct listElem_t * prev;
}listElem_t;

/*list structure*/
typedef struct userList{
    listElem_t * head;
    pthread_mutex_t * list_mutex;
    int counter;
}userList_t;

#endif