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

#define BUFFER_SIZE 1024
#define LISTENQ 10
#define USER_NAME_LENGTH 20

typedef struct user{
    char user_name[USER_NAME_LENGTH];
    struct sockaddr_in user_address;
}user;
