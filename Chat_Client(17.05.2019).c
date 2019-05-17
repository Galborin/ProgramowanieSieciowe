#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h> //potrzebne do timeout�w (SO_RCVTIMEO i strcut timeval)
#include <errno.h>

#define BUFFER_SIZE 1024

void error(const char* msg)  //funkcja do wy�wietlania b��d�w, domy�lnie b��dy maj� by� w logu
{
    perror(msg);
    exit(1);
}

int main(int argc, char**argv){

    if(argc < 2) //sprawdzanie czy user wpisa� adres serwera
    {
        error("Not enough input arguments!");
    }

    int mysockfd, n_bytes;
    struct sockaddr_in myaddress, srvaddress;
    struct timeval time_out_send;                //struktura potrzebna do ustawienia flagi SO_RCTIMEO, i do timeout na odbiranie danych
    struct timeval time_out_recv;                //struktura do ustawienia flagi SNDTIMEOi timeout do wysy�ania danych
    char buffer[BUFFER_SIZE];
    ssize_t n;

    time_out_recv.tv_sec = 5; //ustawiamy timeout na 10 sekund na operacje wej�cia
    time_out_recv.tv_usec = 0;

    time_out_send.tv_sec = 5; //ustawiamy timeout na 10 sekund na operacje wyj�cia
    time_out_send.tv_usec = 0;

    bzero(&srvaddress, sizeof(srvaddress)); //zerowanie struktury jakby by�y tam jakie� �mieci

    srvaddress.sin_family = AF_INET;
    srvaddress.sin_port = htons(5454);
    inet_pton(AF_INET,(const char*)argv[1],&srvaddress.sin_addr);

    if((mysockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket() error: %s\n", strerror(errno));
        return 1;
    }
  

    if(connect(mysockfd,(const struct sockaddr*)&srvaddress,sizeof(srvaddress))<0){
        printf("connect() error: %s\n", strerror(errno));
        return 1;
    }

    if(setsockopt(mysockfd, SOL_SOCKET, SO_RCVTIMEO, &time_out_recv, sizeof(time_out_recv)) < 0){ //aby ustawi� timeout na gniezdzie nale�y ustawi� odpowiedni� flag�
            error("Setsockopt failed");
        }

    if(setsockopt(mysockfd, SOL_SOCKET, SO_SNDTIMEO, &time_out_send, sizeof(time_out_send)) < 0){
            error("Setsockopt failed");
        }

    while(1)
    {
        printf("Ja: ");
        printf("\n");
        fgets(buffer, BUFFER_SIZE, stdin);
        n_bytes = send(mysockfd, buffer, strlen(buffer), 0);

        if(n_bytes < 0){                                //kontrola b��d�w dla send()
                if(errno == EWOULDBLOCK){                   //obs�uga b��du dla timeout, zar�no dla SNDTIMEO i RCVTIMEO
                        error("Tiemout at reciving!");
                        continue;                              //je�eli b�dzie timeout zako�cz program
                   }
                   else{
                       error("Reciving Failure!");
                   }
            }

        if(strcmp(buffer, "exit") == 0){ //je�eli wpisali�my w wierszu polece� exit wychodzimy z programu
            printf("Disconnected!");
            close(mysockfd);
            exit(1);
            }

            if(recv(mysockfd, buffer, BUFFER_SIZE,0) < 0){
                if(errno == EWOULDBLOCK){                      //obs�uga b��du dla timeout, zar�no dla SNDTIMEO i RCVTIMEO
                        error("Tiemout at reciving!");
                        continue;                              //je�eli b�dzie timeout zako�cz program
                   }
                   else{
                       error("Reciving Failure!");
                   }
            }
            else{
                printf("Obcy: %s\n", buffer);
            }
    }
    

    return 0;
}