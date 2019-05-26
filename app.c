/*
piotr
22.05.2019
*/

/*includes------------------------------------------------------*/
#include "app.h"
#include "userlist.h"
#include "command_interface.h"

/*imported------------------------------------------------------*/
extern void client(user * connected_usr);

/*exported------------------------------------------------------*/
userList_t * UserList;

/*global--------------------------------------------------------*/
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

/*private functions prototypes----------------------------------*/
static void sig_pipe(int signo);

/*functions prototypes------------------------------------------*/
int send_UserList(int * filedesc);
void closefd(int * filedesc);

/*main----------------------------------------------------------*/
int main(){
    pid_t forkpid;
    pid_t newsid;
    int listenfd;
    struct sockaddr_in myaddress, cliaddress;
    char address[INET_ADDRSTRLEN+1];
    socklen_t cliaddresslength;
    myaddress.sin_family = AF_INET;
    myaddress.sin_port = htons(5454);
    myaddress.sin_addr.s_addr = INADDR_ANY;

    //forkpid = fork(); /*create child process*/
    //if(forkpid < 0){ /*error*/
    //    printf("fork() fail, %s \n", strerror(errno));
    //    exit(1); 
    //}
    //if(forkpid > 0){ /*kill parent*/
    //    exit(0);
   // }
   // if ( forkpid == 0) {  /* execute in child process */
    //    newsid = setsid();
     //   if(newsid < 0){ /*error*/
     //       printf("setsid() fail, %s \n", strerror(errno));
      //      exit(1);
       // }
    //}    

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

    /*initialize UserList*/
    UserList = (userList_t *)malloc(sizeof(userList_t));
    if(userList_init(UserList,&client_mutex)<0){
        printf("UserList initialization fail\n");
        return 1;
    }

    /*vvvvvvvvvvv here commands are being stored vvvvvvvvvvvv*/

    /*send list of avilable command*/
    command listofcommands_task = {
        command_name: "?",
        func: (int(*)(int * ,void *))send_command_list 
    };
    if(store_command(&listofcommands_task)<0){
        printf("store_command() fail\n\r");
    }
    
    /*send user list */
    command sendusers_task = {
        command_name: "!users",
        func: (int(*)(int * ,void *))send_UserList 
    };
    if(store_command(&sendusers_task)<0){
        printf("store_command() fail\n\r");
    }
    
    /*close filedesc*/
    command closeconnection_task = {
        command_name: "!quit",
        func: (int(*)(int * ,void *))closefd 
    };
    if(store_command(&closeconnection_task)<0){
        printf("store_command() fail\n\r");
    }

    display_command_list();

    /*^^^^^^^^^^ here commands are being stored ^^^^^^^^^^^^*/

    signal(SIGPIPE, sig_pipe);

    int n = 0;
    while(1){ /*main loop*/
        cliaddresslength = sizeof(cliaddress);
        int * connectfd = (int*)malloc(sizeof(int));
        if((*connectfd=accept(listenfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            printf("accept() fail, %s \n", strerror(errno));
        }
        else{
            inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
            printf("\nConnection from: %s\n",address);

            user * conn_user = calloc(1, sizeof(user)); /*allocate memory for new user */
            conn_user->user_address = cliaddress;
            conn_user->fildesc = connectfd;
        
            pthread_t thread; /*new thread*/

            pthread_create(&thread, NULL, (void *(*)(void *))client, (void *)conn_user); /*create thread*/
            pthread_detach(thread);
        }
        sleep(1);
    }

    delete_command_list();
    return 0;
}

/*
handle SIGPIPE.
*/
static void sig_pipe(int signo){
    printf("Received sigpipe\n");
}

/*
send users from UserList function.
Same as send_user_list() (from userlist.h) 
but do not require userList_t argument
*/
int send_UserList(int * filedesc){
    return send_user_list(UserList, filedesc);
}

/*
close connection
*/
void closefd(int * filedesc){
    close(*filedesc);
}
