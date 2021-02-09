#include <stdio.h>

#include "simple_chat_server.h"

#define DEFAULT_PORT "27015"

int __cdecl main(void) 
{
    scs::SimpleChatServer chat(DEFAULT_PORT);
    chat.start_listening();

    return 0;
}