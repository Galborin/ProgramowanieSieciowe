#ifndef SCS_SIMPLE_CHAT_SERVER_H
#define SCS_SIMPLE_CHAT_SERVER_H

#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "osapi\osapi.h"
#include "client.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define MAX_CLIENTS 5

namespace scs 
{
class SimpleChatServer {

private:

private:
  SOCKET mListenSocket;
  std::vector<std::weak_ptr<Client>> mClients = {};
  osapi::Mutex mClientsMutex;

public:
  SimpleChatServer(const char *aPort);
  virtual ~SimpleChatServer(void);
  void start_listening(void);

  /**
   * Return value indicates how many send operations succeeded.
   * It is increased each time the full message was sent.
   * It is decreased each time the send operation failed and was aborted.
   */
  int send_to_all(const std::string &aMessage) const;
};

class ClientThread : public osapi::Thread {
  const SimpleChatServer *mServer;
  const std::shared_ptr<Client> mClient;

  void body() override;

public:
  ClientThread(const char *aName, const SimpleChatServer *aS, const std::shared_ptr<Client> aC) 
    : osapi::Thread(1, 0, osapi::Joinable::NOT_JOINABLE, aName), mServer(aS), mClient(aC) {};

};
} //namepsace
#endif