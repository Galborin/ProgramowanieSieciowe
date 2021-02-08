#include <stdio.h>

#include "simple_chat_server.h"

#define DEFAULT_PORT "27015"

int __cdecl main(void) 
{
    SimpleChatServer scs(DEFAULT_PORT);
    scs.start_listening();

    return 0;
}