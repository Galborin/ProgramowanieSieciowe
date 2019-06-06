/*
piotr
22.05.2019
*/

/*includes------------------------------------------------------*/
#include "command_interface.h"

/*function definitions------------------------------------------*/

command * find_command_by_name(const char * commandname){
    if(!commandname)
        return NULL;
    
    if(!head_cmd)
        return NULL;

    commandElem_t * tmp = head_cmd;

    while(strcmp(tmp->cmd->command_name, commandname)){
        tmp = tmp->next;

        if(!tmp){
            return NULL;
        }
    }
    return tmp->cmd;
}

int store_command(command * comm){
    if(!comm)
        return -1;

    commandElem_t * newelem = (commandElem_t *)malloc(sizeof(commandElem_t));
    newelem->cmd = comm;

    if(!head_cmd){
        head_cmd = newelem;
        head_cmd->next = NULL;
        return ++command_counter;
    }
    else{
        commandElem_t * tmp = head_cmd;
        while(tmp->next){
            tmp = tmp->next;
        }
        tmp->next = newelem;
        newelem->next = NULL;
        return ++command_counter;
    }
}

int send_command_list(user_t * usr){
    size_t length = (command_counter*MAX_CMD_LENGTH) + 21;
    char buffer[length];
    bzero(buffer,length);
    if(!usr)
        return -1;
    if(!head_cmd){
        char * msg = "No commands\n\r";
        if(send(*usr->fildesc,msg,strlen(msg),0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
        }
        return command_counter;
    }
    strcat(buffer,"list of commands\n\r");
    commandElem_t * tmp = head_cmd;
    while(tmp){
        strcat(buffer,tmp->cmd->command_name);
        strcat(buffer,"\n\r");
        tmp = tmp->next;
    }
    
    if(send(*usr->fildesc,buffer,length,0)<0){
            printf("send() fail, %s \n", strerror(errno));
            return -1;
    }

    return command_counter;
}

int display_command_list(){
    if(!head_cmd){
        printf("No commands\n\r");
        return 0;
    }
    printf("list of commands\n\r");
    commandElem_t * tmp = head_cmd;

    while(tmp){
        printf("%s\n\r", tmp->cmd->command_name);
        tmp = tmp->next;
    }
    return command_counter;
}

int delete_command_list(){
    if(!head_cmd){
        printf("No commands\n\r");
        return 0;
    }

    commandElem_t * tmp = head_cmd;

    while(tmp){
        head_cmd = tmp->next;
        free(tmp);
        command_counter--;
        tmp = head_cmd;
    }
    return 0;
}
