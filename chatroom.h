/*
piotr
04.06.2019
*/

#ifndef _CHATROOM
#define _CHATROOM

/*includes------------------------------------------------------*/
#include "app.h"
#include "userlist.h"

/*chatroom typedef*/
typedef struct chatroom{
    char chatname[CHAT_NAME_LENGTH + 1];
    userList_t * chat_userlist;
}chatroom_t;

/*chat list element typedef*/
typedef struct chatListElem{
    chatroom_t * m_chatroom;
    struct chatListElem * next;
    struct chatListElem * prev;
}chatListElem_t;

/*chat list typedef*/
typedef struct chatList{
    chatListElem_t * head;
    pthread_mutex_t * list_mutex;
    int counter;
}chatList_t;

/*function declarations------------------------------------------*/

/*
initialize chatList_t structure.
Return -1 if error.
*/
int chList_init(chatList_t * list, pthread_mutex_t * mutex);

/*
Create chatroom with given name and add it to chat list.
Join it. 
Return number of users in chatroom if success.
Return -1 if fail.
*/
int chRoom_create_and_join(user_t * usr, chatList_t * list, char * ch_name);

/*
Create element with given data and store at the beginning of the list.
If list is empty (head is NULL), data is stored as head.
Return reference to stored list element.
If failed, return NULL.
*/
chatListElem_t * chList_add_element(chatList_t * list, chatroom_t * chtrm);

/*
return element from list with name equal to given.
If there is no such chatroom, return NULL.
*/
chatListElem_t * chList_find_chatroom_by_name(chatList_t * list, const char * chatroomname);

/*
Combine names of all chatrooms from list 
into one message and send it through filedesc.
Return number of elements in list (0 if empty).
Return -1 if error.
*/
int chList_send_list(chatList_t * list, user_t * usr);

/*
Display name of every chatroom from list.
Return number of elements in list.
Return -1 if error.
*/
int chList_display_list(chatList_t * list);

/*
Delete element from list.
Return number of elements in list.
Return -1 if error.
*/
int chList_delete_chatroom(chatList_t * list, chatListElem_t * elem);

/*
Add user to chatroom's user list.
Return number of users in chatroom if success.
Return -1 if fail.
*/
int chList_join_chatroom(chatList_t * list, chatListElem_t * elem, user_t * usr);

/*
Remove user from chatroom's user list.
If that was last user, remove chatroom.
Return number of users in chatroom if success.
Return -1 if fail.
*/
int chList_leave_chatroom(chatList_t * list, chatListElem_t * elem, listElem_t * usr_elem);


#endif
