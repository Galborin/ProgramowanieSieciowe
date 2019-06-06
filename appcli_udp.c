/*
piotr
22.05.2019
*/

/**TODO
 * alarm() when waiting for reply from server (timeout)
 * assert when there is no argument (argc < 2) 
 * /

/*includes------------------------------------------------------*/
#include "app.h"

/*private functions prototypes----------------------------------*/

static char * get_input(char * buffer, int size);
static void * recvfrom_peer(int * udpsockfd);

/*main----------------------------------------------------------*/

int main(int argc, char**argv){
    int mysockfd;
    char buffer[BUFFER_SIZE];
    char address[INET_ADDRSTRLEN+1];
    struct sockaddr_in myaddress, srvaddress;
    ssize_t n;
    pthread_t recv_thread;
    socklen_t cliaddresslength;

    srvaddress.sin_family = AF_INET;
    srvaddress.sin_port = htons(5454);
    inet_pton(AF_INET,(const char*)argv[1],&srvaddress.sin_addr);

    if((mysockfd = socket(AF_INET,SOCK_DGRAM,0))<0){
        printf("socket() error: %s\n", strerror(errno));
        return 1;
    }
    

    char * hellomsg = "";
    if(sendto(mysockfd,(const void*)hellomsg,BUFFER_SIZE,0,(struct sockaddr*)&srvaddress, sizeof(srvaddress))<0){
        printf("send() fail\n");
    }

    printf("Waiting for reply from server...\n");
    
    int nreceived;
    do{
        if((nreceived = recvfrom(mysockfd, buffer, BUFFER_SIZE, 0 , (struct sockaddr*)&srvaddress, &cliaddresslength)) >= 0){
            inet_ntop(AF_INET,(const struct sockaddr *)&srvaddress.sin_addr,address,sizeof(address));
            printf("\n\rConnection from on UDP socket: %s\n\r",address);
        }
    }while((nreceived < 0) && ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)));
    
    if(nreceived < 0){
        printf("recvfrom() fail here, %s \n", strerror(errno));
    }
    printf("Success!\n");
    
    pthread_create(&recv_thread,NULL, (void*(*)(void *))recvfrom_peer, (void *)&mysockfd);
    pthread_detach(recv_thread);

    static char msg[BUFFER_SIZE];
    while(1){
        if(!get_input(msg,BUFFER_SIZE)){
            printf("get_input() fail\n");
            break;
        }
        if(sendto(mysockfd,(const void*)msg,BUFFER_SIZE,0,(struct sockaddr*)&srvaddress, sizeof(srvaddress))<0){
            printf("send() fail\n");
            break;
        }
        else{
            //printf("sended %d \n", (int)strlen(msg)+1);
        }
    }
    
    close(mysockfd);
    printf("\nCLOSING :\n");
    return 0;
}

/*
Receive messages from server and print on stdout.
Return NULL if error.
*/
static void * recvfrom_peer(int * udpsockfd){
    static char buffer[BUFFER_SIZE];
    char address[INET_ADDRSTRLEN+1];
    struct sockaddr_in peeraddress;
    int nreceived;
    socklen_t cliaddresslength;

    while(1){
        do{
            if((nreceived = recvfrom(*udpsockfd, buffer, BUFFER_SIZE, 0 , (struct sockaddr*)&peeraddress, &cliaddresslength)) >= 0){
                inet_ntop(AF_INET,(const struct sockaddr *)&peeraddress.sin_addr,address,sizeof(address));
                printf("\n\rConnection from on UDP socket: %s\n\r",address);
            }
        }while((nreceived < 0) && ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)));
    
        if(nreceived < 0){
            printf("recvfrom() fail here, %s \n", strerror(errno));
            return NULL;
        }
    }
    return NULL;
}

/*
get message from stdin. 
Returns input if newline. 
If error return NULL.
*/
static char * get_input(char * buffer, int size){
    char * input;

    if(!(input = fgets(buffer,size,stdin)) && ferror(stdin)){
        printf("fgets error\n");
        return NULL;
    }
    buffer[strlen(buffer)-1] = '\0';
    return input;
} 
