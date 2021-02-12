#include "simple_chat_server.h"

#include <limits.h>

#define DEFAULT_MUTEX_LOCK_TIMEOUT 1000

namespace scs 
{
SimpleChatServer::SimpleChatServer(const char *aPort) {
    
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    int ret;
    // Resolve the server address and port
    ret = getaddrinfo(NULL, aPort, &hints, &result);
    if ( ret != 0 ) {
        printf("getaddrinfo failed with error: %d\n", ret);
        return;
    }

    // Create a SOCKET for connecting to server
    mListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (mListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return;
    }

    // Setup the TCP listening socket
    ret = bind(mListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (ret == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(mListenSocket);
        return;
    }

    freeaddrinfo(result);
}

SimpleChatServer::~SimpleChatServer() {
    mClients.clear();
    mThreads.clear();
    WSACleanup();
}

bool SimpleChatServer::remove_thread(osapi::Thread *pThr) {
    if (mThreadMutex.lock(DEFAULT_MUTEX_LOCK_TIMEOUT)) {
    
        for (int i = 0; i < mThreads.size(); ++i) {
            if (mThreads[i].get() == pThr) {
                mThreads.erase(mThreads.begin() + i);
                mThreadMutex.unlock();
                return true;
            }
        }
        mThreadMutex.unlock();
    } 
    return false;
}

void SimpleChatServer::start_listening() {
    int ret;

    ret = listen(mListenSocket, SOMAXCONN);
    if (ret == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(mListenSocket);
        return;
    }
    
    while(true) {
        
        if (Client::mNumberOfClients >= MAX_CLIENTS) {
            continue;
        }

        std::shared_ptr<Client> cl = std::make_shared<Client>();

        // Accept a client socket
        cl->mSocket = accept(mListenSocket, NULL, NULL);
        if (cl->mSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            break;
        }
        
        if (mClientsMutex.lock(DEFAULT_MUTEX_LOCK_TIMEOUT)) {
            mClients.push_back(cl);
            printf("Added new client\n");
            mClientsMutex.unlock();
        } else { continue; }

        // create thread for client.
        std::string name = std::to_string(mClientNumber);
        printf("Create user with name %d\n", mClientNumber);
        if (mThreadMutex.lock(DEFAULT_MUTEX_LOCK_TIMEOUT)) {   
            mThreads.push_back(std::make_unique<ClientThread>(name.c_str(), this, cl));
            mThreads.back()->run();
            mThreadMutex.unlock();
        }
        
        if (mClientNumber < INT_MAX) { mClientNumber++; }
        else {mClientNumber = 0;}

        Sleep(1);
    }
    
    // cleanup
    closesocket(mListenSocket);
}

int SimpleChatServer::send_to_all(const std::string &aMessage) {
    int bytes_sent;
    int ret = 0;
    if (mClientsMutex.lock(DEFAULT_MUTEX_LOCK_TIMEOUT)) {

        for (auto i = 0; i < mClients.size(); ++i) {
            bytes_sent = 0;
            if (auto pCl = mClients[i].lock()) {
                while (bytes_sent < aMessage.length()) {
                    bytes_sent += send(pCl->mSocket, aMessage.c_str(), aMessage.length(), 0);
                    if (bytes_sent == SOCKET_ERROR) {
                        mClientsMutex.unlock();
                        printf("send failed with error: %d\n", WSAGetLastError());
                        ret--;
                        break;
                    }
                }
                ret++;
            }
        }
        mClientsMutex.unlock();
    }
    return ret;
}

void ClientThread::begin() {
    printf("Hello from thread\n");
}

void ClientThread::loop() {
    std::shared_ptr<Client> cl = mClient;
    if (!cl) {
        kill();
        return;
    }

    int ret;
    int bytes_received = 0;
    int bytes_sent = 0;
    std::string message;
    std::string prefix = getName(); 
    prefix = "[" + prefix + "]";
    // Receive until the peer shuts down the connection
    do {
        bytes_received = recv(cl->mSocket, cl->mBuffer.get(), CLIENT_BUFLEN, 0);
        if (bytes_received > 0) {
            printf("Bytes received: %d\n", bytes_received);
            message = std::string(cl->mBuffer.get(), bytes_received);
            printf("Message: %s\n", message.c_str());
            mServer->send_to_all(prefix + message);

        }
        else if (bytes_received == 0) {
            printf("Connection closing...\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(cl->mSocket);
            kill();
            return;
        }
    } while (bytes_received > 0);

    // shutdown the connection since we're done
    ret = shutdown(cl->mSocket, SD_SEND);
    if (ret == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }

    // cleanup
    closesocket(cl->mSocket);
    kill();
}

void ClientThread::end() {
    printf("End thread\n");

    // nothing more to be done in the thread.
    mServer->remove_thread(this);
}
} // namespace