#include "app.h"

extern int send_echo(int destfd, void * ptr, int n);
extern void my_echo(int destfd, void * ptr, size_t length);
void client(user * connected_usr);
extern int delete_user(const char * del_user_name);

int main(){
    pid_t forkpid;
    pid_t newsid;
    int listenfd, connectfd;
    struct sockaddr_in myaddress, cliaddress;
    char address[INET_ADDRSTRLEN+1];
    socklen_t cliaddresslength;
    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(5454);
    myaddress.sin_addr.s_addr = INADDR_ANY;

    forkpid = fork(); /*create child process*/
    if(forkpid < 0){ /*error*/
        printf("fork() fail, %s \n", strerror(errno));
        exit(1); 
    }
    if(forkpid > 0){ /*kill parent*/
        exit(0);
    }
    if ( forkpid == 0) {  /* execute in child process */
        newsid = setsid();
        if(newsid < 0){ /*error*/
            printf("setsid() fail, %s \n", strerror(errno));
            exit(1);
        }
    }    

    if((listenfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() fail: %s \n", strerror(errno));
        return 1;
    }

    int reuse = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&reuse,sizeof(reuse))<0){
        printf("setsockopt() fail: %s \n", strerror(errno));
        return 1;
    }

    if(bind(listenfd,(struct sockaddr *)&myaddress,sizeof(myaddress))<0){
        printf("bind() fail: %s \n", strerror(errno));    
        return 1;
    }

    if(listen(listenfd,LISTENQ)<0){
        printf("listen() fail, %s \n", strerror(errno));
        return 1;
    }

    int n = 0;
    while(1){ /*main loop*/
        cliaddresslength = sizeof(cliaddress);
        if((connectfd=accept(listenfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
            return 1;
        }
        inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
        printf("\nConnection from: %s\n",address);
            
        user * conn_user = calloc(1, sizeof(user)); /*allocate memory for new user */
        conn_user->user_address = cliaddress;
        conn_user->fildesc = connectfd;
        
        pthread_t thread; /*new thread*/

        pthread_create(&thread, NULL, (void *(*)(void *))client, (void *)conn_user); /*create thread*/
        pthread_detach(thread);
    }
    return 0;
}
