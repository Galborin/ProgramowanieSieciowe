#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>                       //potrzebne do funkcji inet_ntop
#include <time.h>
#include <sys/wait.h>                        //potrzebne do waitpid()
#include <signal.h>

#define BUFFER_SIZE 1024                     //TESTOWO POTEM USUNAC

char *get_time() //funkcja zwraca date i obecny czas
{
    time_t current_time;
    time(&current_time);
    return ctime(&current_time);
}

void sig_child(int signo)  //handler dla sygna³u SIGCHILD
{
    pid_t child_pid;
    int status;

        while(waitpid((pid_t)(-1), &status, WNOHANG) > 0){
            printf("%s child: %d has been terminated", get_time(), child_pid);
            return;
        }
//The reason for placing waitpid within a loop is to allow for the possibility that multiple 
//child processes could terminate while one is in the process being reaped. Only one instance 
//of SIGCHLD can be queued, so it may be necessary to reap several zombie processes during one invocation of the handler function 
//WNOHANG wychodzi od razu je¿eli proces potomny nie zosta³ nigdy utworzony       
}

int main(){
    char buffer[BUFFER_SIZE];                //TESTOWO POTEM USUNAC
    int mysockfd, clisockfd, n_bytes;
    struct sockaddr_in myaddress, cliaddress;
    socklen_t cliaddresslength;
    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(5454);
    myaddress.sin_addr.s_addr = INADDR_ANY;
    char connected_address[INET_ADDRSTRLEN]; //tablica bêdzie przechowywaæ adres pod³¹czonego klienta
    pid_t child_pid;                         //przechowuje pid potomka


    if((mysockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() fail: %s \n", strerror(errno));
        return 1;
    }

    int reuse = 1;
    if(setsockopt(mysockfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&reuse,sizeof(reuse))<0){
        printf("setsockopt() fail: %s \n", strerror(errno));
        return 1;
    }

    if(bind(mysockfd,(struct sockaddr *)&myaddress,sizeof(myaddress))<0){
        printf("bind() fail: %s \n", strerror(errno));    
        return 1;
    }

    if(listen(mysockfd,10) == 0){
        printf("Listening \n");
    }
        else{
        printf("listen() fail, %s \n", strerror(errno));
        return 1;
        }
    
    signal(SIGCHLD, sig_child);

    while(1){
        if((clisockfd=accept(mysockfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
            return 1;
        }
        else{   //wyœwietalnie informacji pod³¹czonego hosta
            bzero(&connected_address, sizeof(connected_address));
            inet_ntop(AF_INET, (struct sockaddr* ) &(cliaddress.sin_addr), connected_address, sizeof(connected_address)); 
            //funkcja wy¿ej zamienia adres na fromat mo¿liwy do odczytania przez cz³owieka
            printf("%s Connection from: %s, on port: %d\n",get_time() ,connected_address, ntohs(cliaddress.sin_port));
        }

        if(child_pid = fork() == 0){ //tworzymy nowy proces dla ka¿ego klienta
            close(mysockfd);

            while (1){  //odpowiadanie klientom echo
                bzero(&buffer, sizeof(buffer));
                n_bytes = recv(clisockfd, buffer, BUFFER_SIZE, 0);
                    if(n_bytes < 0){
                        printf("Receiving error!");
                        close(clisockfd);
                        break;
                    }
                    else{
                        send(clisockfd, buffer, strlen(buffer), 0);
                        bzero(&buffer, sizeof(buffer));
                    }
            }
            
        }
        close(clisockfd);
       }

    return 0;
}