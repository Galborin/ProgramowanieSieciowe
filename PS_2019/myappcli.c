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

#define MAXRCV 1024

int 
main(int argc, char **argv)
{
    int mysckfd;
    struct sockaddr_in6 srvaddr;

    if(argc!=2)
    {
        fprintf(stderr, "usage: %s <IPaddress> \n", argv[0]);
        return 1;
    }

    if(mysckfd = socket(AF_INET6, SOCK_STREAM, 0)<0)
    {
        fprintf(stderr,"socket error : %s\n", strerror(errno));
		return 1;
    }

    bzero(&srvaddr,sizeof(srvaddr));
    srvaddr.sin6_family = AF_INET6;
    srvaddr.sin6_port = htons(50000);

    int ptonerr;
    if(ptonerr = inet_pton(AF_INET6,argv[1],&srvaddr.sin6_addr)==-1)
    {
        fprintf(stderr,"ERROR: inet_pton error for %s : %s \n", argv[1], strerror(errno));
		return 1;
    }else if(ptonerr == 0)
    {
        fprintf(stderr,"ERROR: Invalid address family \n");
		return 1;
    }

    socklen_t len = sizeof(srvaddr);
    if(connect(mysckfd, (struct sockaddr *)&srvaddr,len))
    {
        fprintf(stderr,"connect error : %s\n",strerror(errno));
        return 1;
    }

    char rcvline[MAXRCV+1];
    int nobytes;
    while(nobytes = read(mysckfd,rcvline,MAXRCV)>0)
    {
        rcvline[nobytes] = 0;
        if(fputs(rcvline,stdout) == EOF)
        {
            fprintf(stderr,"fputs error : %s\n", strerror(errno));
			return 1;
        }
    }

    if (nobytes < 0)
	    fprintf(stderr,"read error : %s\n", strerror(errno));
    
    exit(0);
}