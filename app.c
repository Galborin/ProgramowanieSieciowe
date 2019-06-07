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
userList_t GlobalUserList;
chatList_t GlobalChatList;

/*global--------------------------------------------------------*/
pthread_mutex_t chatlist_mutex = PTHREAD_MUTEX_INITIALIZER;

/*private functions prototypes----------------------------------*/
static void sig_pipe(int signo);

/*functions prototypes------------------------------------------*/
int send_GlobalUserList(user_t * usr);
void closefd(user_t * usr);
int GlobalChatList_send_list(user_t * usr);
int GlobalChatList_create_and_join_chatroom(user_t * usr, void * name);
int GlobalChatList_join_chatroom(user_t * usr, void * chatroomname);
int GlobalChatList_leave_chatroom(user_t * user);
int chRoom_send_user_list(user_t * usr);

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

    /*initialize GlobalUserList*/
    if(userList_init(&GlobalUserList) < 0){
        printf("GlobalUserList initialization fail\n");
        return 1;
    }

    /*initialize GlobalChatList*/
    if(chList_init(&GlobalChatList,&chatlist_mutex) < 0){
        printf("GlobalUserList initialization fail\n");
        return 1;
    }

    /*vvvvvvvvvvv here commands are being stored vvvvvvvvvvvv*//*vvvvvvvvvvv here commands are being stored vvvvvvvvvvvv*/

    /*send list of avilable commands*/
    command listofcommands_command = {
        command_name: "?",
        func: (int(*)(user_t * ,void *))send_command_list 
    };
    if(store_command(&listofcommands_command) < 0){
        printf("store_command() fail\n\r");
    }
    
    /*send user list */
    command sendglobalusers_command = {
        command_name: "!global",
        func: (int(*)(user_t * ,void *))send_GlobalUserList 
    };
    if(store_command(&sendglobalusers_command) < 0){
        printf("store_command() fail\n\r");
    }

    /*send chatroom list */
    command sendchatrooms_command = {
        command_name: "!chats",
        func: (int(*)(user_t * ,void *))GlobalChatList_send_list 
    };
    if(store_command(&sendchatrooms_command) < 0){
        printf("store_command() fail\n\r");
    }
    
    /*create and join chatroom*/
    command createchatroom_command = {
        command_name: "!create",
        func: (int(*)(user_t * ,void *))GlobalChatList_create_and_join_chatroom
    };
    if(store_command(&createchatroom_command) < 0){
        printf("store_command() fail\n\r");
    }

    /*join chatroom*/
    command joinchatroom_command = {
        command_name: "!join",
        func: (int(*)(user_t * ,void *))GlobalChatList_join_chatroom
    };
    if(store_command(&joinchatroom_command) < 0){
        printf("store_command() fail\n\r");
    }

    /*send chatroom's user list*/
    command sendchroomusers_command = {
        command_name: "!users",
        func: (int(*)(user_t *, void *))chRoom_send_user_list
    };
    if(store_command(&sendchroomusers_command) < 0){
        printf("store_command() fail\n\r");
    }

    /*leave chatroom*/
    command leavechatroom_command = {
        command_name: "!leave",
        func: (int(*)(user_t *, void *))GlobalChatList_leave_chatroom
    };
    if(store_command(&leavechatroom_command) < 0){
        printf("store_command() fail\n\r");
    }

    /*close filedesc*/
    command closeconnection_command = {
        command_name: "!quit",
        func: (int(*)(user_t * ,void *))closefd 
    };
    if(store_command(&closeconnection_command) < 0){
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
            conn_user->chatroom_name = NULL;
        
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
Send users from GlobalUserList.
Same as send_user_list() (from userlist.h) 
but does not require userList_t argument
*/
int send_GlobalUserList(user_t * usr){
    if(usr)
        return send_user_list(&GlobalUserList, usr);
    
    return -1;
}

/*
Send chatrooms from GlobalChatList.
Same as chList_send_list() (from chatroom.h) 
but does not require chatList_t argument
*/
int GlobalChatList_send_list(user_t * usr){
    if(usr)
        return chList_send_list(&GlobalChatList, usr);
    
    return -1;
}

/*
Function based on chRoom_create_and_join() (from chatroom.h),
but always takes GlobalChatList as argument. 
*/
int GlobalChatList_create_and_join_chatroom(user_t * usr, void * name){

    if((usr) && (name)){
        return chRoom_create_and_join(usr,&GlobalChatList,(char *)name);
    }

    return -1;
}

/*
Find and join chatroom from GlobalChatList with given name.
*/
int GlobalChatList_join_chatroom(user_t * usr, void * chatroomname){

    if((usr) && (chatroomname)){
        chatListElem_t * roomtojoin;
        int result;
        pthread_mutex_lock(GlobalChatList.list_mutex);
        if((roomtojoin = chList_find_chatroom_by_name(&GlobalChatList, (char *)chatroomname))){
            result = chList_join_chatroom(&GlobalChatList, roomtojoin, usr);
        }
        pthread_mutex_unlock(GlobalChatList.list_mutex);
        return result;
    }

    return -1;
}

/*
Leave chatroom.
*/
int GlobalChatList_leave_chatroom(user_t * user){
    listElem_t * user_elem;
    chatListElem_t * roomtoleave;

    /*find chatroom to leave*/
    pthread_mutex_lock(GlobalChatList.list_mutex);
    roomtoleave = chList_find_chatroom_by_name(&GlobalChatList, user->chatroom_name);
    pthread_mutex_unlock(GlobalChatList.list_mutex);
    if(!roomtoleave)    
        return -1;

    /*find user element in chatroom's user list*/
    pthread_mutex_lock(&roomtoleave->m_chatroom->chat_userlist->list_mutex);
    user_elem = find_user_by_name(roomtoleave->m_chatroom->chat_userlist, user->user_name);
    pthread_mutex_unlock(&roomtoleave->m_chatroom->chat_userlist->list_mutex);

    if(!user_elem)
        return -1;
    
    return chList_leave_chatroom(&GlobalChatList,roomtoleave,user_elem);
}

/*
Send chatroom's user list.
*/
int chRoom_send_user_list(user_t * usr){
    if(!usr->chatroom_name)
        return -1;
    
    chatListElem_t * room_elem;
    pthread_mutex_lock(GlobalChatList.list_mutex);
    if(!(room_elem = chList_find_chatroom_by_name(&GlobalChatList, usr->chatroom_name))){
        pthread_mutex_unlock(GlobalChatList.list_mutex);
        return -1;
    }

    pthread_mutex_unlock(GlobalChatList.list_mutex);

    return send_user_list(room_elem->m_chatroom->chat_userlist, usr);
}



/*
close connection
*/
void closefd(user_t * usr){
    close(*usr->fildesc);
}
