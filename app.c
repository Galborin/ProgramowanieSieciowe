#include "app.h"

extern int send_echo(int destfd, void * ptr, int n);
extern void my_echo(int destfd, void * ptr, size_t length);
extern struct sockaddr * client(int clifd, const void * ptr, size_t length);
extern user * log_in(int clifd, struct sockaddr_in * usraddr);
extern void display_user_list();
extern int delete_user(const char * del_user_name);

void sig_child(int signo){
    pid_t	pid;
    int stat_loc;
    while((pid = waitpid(-1,&stat_loc,WNOHANG))>0){
        printf("Child %d terminated\n",pid);
    }
}

void sig_pipe(int signo){
	printf("Server received SIGPIPE - Default action is exit \n");
	//exit(1);
}

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
    if ( forkpid == 0) {	/* execute in child process */
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

    signal(SIGCHLD, sig_child);
    signal(SIGPIPE, sig_pipe);

    int n = 0;
    while(1){
        cliaddresslength = sizeof(cliaddress);
        if((connectfd=accept(listenfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
            return 1;
        }
        else{
            inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
            printf("Connection from: %s\n",address);
        }

        char * msg = "HELLO";
        /*if(send(connectfd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
        }
        else{
            printf("sended Hello Message %d \n", (int)strlen(msg)+1);
        }*/

        char buffer[BUFFER_SIZE];
        
        forkpid = fork();

        if(forkpid < 0){ /*error*/
            printf("fork() fail, %s \n", strerror(errno));
            exit(1); 
        }

        if(forkpid == 0){ /*execute in child process */
            close(listenfd);

            /*log in */
            user * myuser = log_in(connectfd, &cliaddress);
            msg = "\nLOGGED IN\n\n";
            if(send(connectfd,(const void*)msg,strlen(msg)+1,0)<0){
                printf("send() fail, %s \n", strerror(errno));
            }
            printf("\nLOGGED IN :\n\n");
            display_user_list();
            my_echo(connectfd,buffer,BUFFER_SIZE);
            client(connectfd, buffer, BUFFER_SIZE);
            printf("\nDELETED %i:\n\n", delete_user(myuser->user_name));
            sleep(1);
            exit(0);
        }
        close(connectfd); 
    }
    return 0;
}
