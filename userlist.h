/*piotr
22.05.2019
*/ 

#ifndef _USERLIST
#define _USERLIST

/*includes------------------------------------------------------*/
#include "app.h"

/*function declarations------------------------------------------*/

/*
initialize userList_t structure.
Return -1 if error.
*/
int userList_init(userList_t * list, pthread_mutex_t * mutex);

/*
store element at the end of the list.
If list is empty (head is NULL), head becomes element.
If element is NULL, return -1.
*/
int store_element(userList_t * list, listElem_t * element);

/*
return user from list with name equals to given.
If there is no such user, return NULL.
*/
user * find_user_by_name(userList_t * list, const char * username);

/*
Combine addresses and names of all users from list 
into one message and send it through filedesc.
Return number of elements in list (0 if empty).
Return -1 if error.
*/
int send_user_list(userList_t * list, int * filedesc);

/*Display address and name of every user from list.
Return number of elements in list.
Return -1 if error.
*/
int display_user_list(userList_t * list);

/*Delete user with name equal to given from list.
Return number of elements in list.
Return -1 if error.
*/
int delete_user(userList_t * list, const char * name);

#endif