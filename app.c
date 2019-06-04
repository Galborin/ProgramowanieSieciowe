/*
piotr
22.05.2019
*/


/*includes------------------------------------------------------*/
#include "app.h"
#include "userlist.h"
#include "command_interface.h"
#include "chatroom.h"

/*imported------------------------------------------------------*/
extern void * client(user_t * connected_usr);

/*exported------------------------------------------------------*/
userList_t UserList;
chatList_t ChatList;

/*global--------------------------------------------------------*/
pthread_mutex_t chatlist_mutex = PTHREAD_MUTEX_INITIALIZER;

/*private functions prototypes----------------------------------*/
static void sig_pipe(int signo);

/*functions prototypes------------------------------------------*/
int send_UserList(user_t * usr);
void closefd(user_t * usr);
int ChatList_send_list(user_t * usr);
int ChatList_create_and_join_chatroom(user_t * usr, void * name);

/*main----------------------------------------------------------*/
int main(){
    pid_t forkpid;
    pid_t newsid;
    int listenfd, udp_socket;
    char buffer[BUFFER_SIZE];
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

    if((udp_socket = socket(AF_INET,SOCK_DGRAM,0))<0){
        printf("socket() fail: %s \n", strerror(errno));
        return 1;
    }

    int reuse = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&reuse,sizeof(reuse))<0){
        printf("setsockopt() fail: %s \n", strerror(errno));
        return 1;
    }

    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(udp_socket, F_GETFL, 0);
    fcntl(udp_socket, F_SETFL, flags | O_NONBLOCK);

    if(bind(listenfd,(struct sockaddr *)&myaddress,sizeof(myaddress))<0){
        printf("bind() fail: %s \n", strerror(errno));    
        return 1;
    }

    if(bind(udp_socket,(struct sockaddr *)&myaddress,sizeof(myaddress))<0){
        printf("bind() fail: %s \n", strerror(errno));    
        return 1;
    }

    if(listen(listenfd,LISTENQ)<0){
        printf("listen() fail, %s \n", strerror(errno));
        return 1;
    }

    /*initialize UserList*/
    if(userList_init(&UserList) < 0){
        printf("UserList initialization fail\n");
        return 1;
    }

    /*initialize ChatList*/
    if(chList_init(&ChatList,&chatlist_mutex) < 0){
        printf("UserList initialization fail\n");
        return 1;
    }

    /*vvvvvvvvvvv here commands are being stored vvvvvvvvvvvv*//*vvvvvvvvvvv here commands are being stored vvvvvvvvvvvv*/

    /*send list of avilable commands*/
    command listofcommands_task = {
        command_name: "?",
        func: (int(*)(user_t * ,void *))send_command_list 
    };
    if(store_command(&listofcommands_task) < 0){
        printf("store_command() fail\n\r");
    }
    
    /*send user list */
    command sendusers_task = {
        command_name: "!users",
        func: (int(*)(user_t * ,void *))send_UserList 
    };
    if(store_command(&sendusers_task) < 0){
        printf("store_command() fail\n\r");
    }

    /*send chatroom list */
    command sendchatrooms_task = {
        command_name: "!chats",
        func: (int(*)(user_t * ,void *))ChatList_send_list 
    };
    if(store_command(&sendchatrooms_task) < 0){
        printf("store_command() fail\n\r");
    }
    
    /*create and join chatroom*/
    command createchatroom_task = {
        command_name: "!create",
        func: (int(*)(user_t * ,void *))ChatList_create_and_join_chatroom
    };
    if(store_command(&createchatroom_task) < 0){
        printf("store_command() fail\n\r");
    }

    /*close filedesc*/
    command closeconnection_task = {
        command_name: "!quit",
        func: (int(*)(user_t * ,void *))closefd 
    };
    if(store_command(&closeconnection_task) < 0){
        printf("store_command() fail\n\r");
    }

    display_command_list();

    /*^^^^^^^^^^ here commands are being stored ^^^^^^^^^^^^*//*^^^^^^^^^^ here commands are being stored ^^^^^^^^^^^^*/

    signal(SIGPIPE, sig_pipe);

    int * connectfd = (int*)malloc(sizeof(int));
    int n = 0;
    while(1){ /*main loop*/
        cliaddresslength = sizeof(cliaddress);

        /*TCP************************************************/

        if((*connectfd=accept(listenfd,(struct sockaddr*)&cliaddress,&cliaddresslength))<0){
            if((errno != EAGAIN) && (errno != EWOULDBLOCK))
                printf("accept() fail, %s \n", strerror(errno));
        }
        else{
            inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
            printf("\nConnection from: %s\n",address);

            user_t * conn_user = calloc(1, sizeof(user_t)); /*allocate memory for new user_t */
            conn_user->user_address = cliaddress;
            conn_user->fildesc = connectfd;
        
            pthread_t thread; /*new thread*/

            pthread_create(&thread, NULL, (void *(*)(void *))client, (void *)conn_user); /*create thread*/
            pthread_detach(thread);
            connectfd = (int*)malloc(sizeof(int));
        }

        /*UDP************************************************/

        if((n = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0 , (struct sockaddr*)&cliaddress,&cliaddresslength)) < 0){
            if((errno != EAGAIN) && (errno != EWOULDBLOCK))
                printf("recvfrom() fail, %s \n", strerror(errno));
        }
        else{
            inet_ntop(AF_INET,(const struct sockaddr *)&cliaddress.sin_addr,address,sizeof(address));
            printf("\nConnection from on UDP socket: %s\n",address);
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
Send users from UserList.
Same as send_user_list() (from userlist.h) 
but does not require userList_t argument
*/
int send_UserList(user_t * usr){
    if(usr)
        return send_user_list(&UserList, usr);
    
    return -1;
}

/*
Send chatrooms from ChatList.
Same as chList_send_list() (from chatroom.h) 
but does not require chatList_t argument
*/
int ChatList_send_list(user_t * usr){
    if(usr)
        return chList_send_list(&ChatList, usr);
    
    return -1;
}

/*
Function based on chRoom_create_and_join() (from chatroom.h),
but always takes ChatList as argument. 
*/
int ChatList_create_and_join_chatroom(user_t * usr, void * name){

    if((usr) && (name)){
        return chRoom_create_and_join(usr,&ChatList,(char *)name);
    }

    return -1;
}

/*
close connection
*/
void closefd(user_t * usr){
    close(*usr->fildesc);
}
