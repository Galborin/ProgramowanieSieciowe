#include "client.h"

namespace scs {

Client::Client() { 
  mNumberOfClients++;
  mBuffer = std::make_unique<char []>(CLIENT_BUFLEN);
}

int Client::mNumberOfClients = 0;

Client::~Client() {
  mNumberOfClients--;
}

} // namespace