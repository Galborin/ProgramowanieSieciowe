#include "app.h"

extern int send_echo(int destfd,const void * ptr,int n);
extern void my_echo(int destfd,void * ptr,size_t length);


void sig_child(int signo){
    pid_t	pid;
    int stat_loc;
    while((pid = waitpid(-1,&stat_loc,WNOHANG))>0){
        printf("Child %d terminated\n",pid);
    }
}

int main(){
    pid_t childprocesspid;
    int listenfd, connectfd;
    struct sockaddr_in myaddress, cliaddress;
    char address[INET_ADDRSTRLEN+1];
    socklen_t cliaddresslength;
    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(5454);
    myaddress.sin_addr.s_addr = INADDR_ANY;

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

    if(listen(listenfd,2)<0){
        printf("listen() fail, %s \n", strerror(errno));
        return 1;
    }

    signal(SIGCHLD, sig_child);

    int n = 0;
    while(1){
        if((connectfd=accept(listenfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
            return 1;
        }
        else{
            inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
            printf("Connection from: %s\n",address);
        }

        char * msg = "HELLO";
        if(send(connectfd,(const void*)msg,strlen(msg)+1,0)<0){
            printf("send() fail, %s \n", strerror(errno));
        }
        else{
            printf("sended Hello Message %d \n", (int)strlen(msg)+1);
        }

        char buffer[BUFFER_SIZE];
        
        childprocesspid = fork(); /*create child process*/
        if ( childprocesspid == 0) {	/* execute in child process */
            close(listenfd);
            my_echo(connectfd,buffer,BUFFER_SIZE);
            exit(0);
        }
        close(connectfd);
    }
    return 0;
}
