#ifndef SIMPLE_CHAT_SERVER_H
#define SIMPLE_CHAT_SERVER_H

#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512


class SimpleChatServer {

private:
  SOCKET mListenSocket;
  SOCKET mClientSocket;

  char mRecvbuf[DEFAULT_BUFLEN] = {};

public:
  SimpleChatServer(const char *aPort);
  virtual ~SimpleChatServer(void);
  void start_listening(void);

protected:
  virtual void handle_client(void);
};

#endif