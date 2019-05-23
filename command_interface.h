/*piotr
22.05.2019
*/ 

#ifndef _COMMAND_INTERFACE
#define _COMMAND_INTERFACE

/*includes------------------------------------------------------*/
#include "app.h"

/*command structure*/
typedef struct command{
    char command_name[MAX_CMD_LENGTH];
    int (*func)(void * args);
}command;

/*command list element*/
typedef struct commandElem_t{
    command * cmd;
    struct commandElem_t * next;
}commandElem_t;

/*head of command list*/
static commandElem_t * head_cmd;

/*counter of commands*/
static int command_counter = 0;

/*function declarations------------------------------------------*/

/*
find command with given name.
Return reference to it.
Return NULL if fail.
*/
command * find_command_by_name(const char * commandname);

/*
Add commmand to the end of list. 
Return command_counter if success, -1 if fail.
*/
int store_command(command * comm);

/*
send command list to user.
Return command_counter.
Return -1 if error.
*/
int send_command_list(int * filedesc);

/*
display command list.
Return command_counter.
*/
int display_command_list();

/*
free all resources from command list.
*/
int delete_command_list();


#endif
