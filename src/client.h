#ifndef SCS_CLIENT_H
#define SCS_CLIENT_H

#include <memory>
#include <vector>
#include <winsock2.h>
#include "osapi\osapi.h"

#define CLIENT_BUFLEN 512

namespace scs {

class Client {
public:
  static int mNumberOfClients;
  SOCKET mSocket;

  std::unique_ptr<char []>mBuffer;

  Client();
  ~Client();
};


} //namespace

#endif