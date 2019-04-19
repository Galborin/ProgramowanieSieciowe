#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LISTENQUEUE 2

int 
main(int argc, char **argv)
{
    int mysckfd,myconnfd; //file descriptors

    if((mysckfd = socket(AF_INET,SOCK_STREAM,0))<0)
    {
        fprintf(stderr,"socket error : %s\n",strerror(errno));
        return 1;
    }

    struct sockaddr_in6 srvaddr,cliaddr;

    bzero(&srvaddr,sizeof(srvaddr));
    srvaddr.sin6_family = AF_INET6;
    srvaddr.sin6_port = htons(13);
    srvaddr.sin6_addr = in6addr_any;

    if(bind(mysckfd,(struct sockaddr *)&srvaddr,sizeof(srvaddr))<0)
    {
        fprintf(stderr,"bind error : %s\n", strerror(errno));
        return 1;
    }

    if(listen(mysckfd, LISTENQUEUE)<0)
    {
        fprintf(stderr,"listen error : %s\n",strerror(errno));
        return 1;
    }

    char str[INET6_ADDRSTRLEN];
    socklen_t length;
    for( ; ;)
    {
        length = sizeof(cliaddr);
        if(myconnfd = accept(mysckfd, (struct sockaddr *)&cliaddr,&length)<0)
        {
            fprintf(stderr,"accept error : %s\n",strerror(errno));
            continue;
        }

        bzero(str,sizeof(str));
        inet_ntop(AF_INET6, (struct sockaddr *)&cliaddr.sin6_addr,str,sizeof(str));
        printf("Connection from %s\n", str);
        close(myconnfd);
    }
}
