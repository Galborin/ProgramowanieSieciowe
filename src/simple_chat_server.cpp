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
    WSACleanup();
}

void SimpleChatServer::remove_thread(osapi::Thread *pThr) {
    for (int i = 0; i < mThreads.size(); ++i) {
        if (mThreads[i].get() == pThr) {
            mThreads.erase(mThreads.begin() + i);
        }
    }
}

void SimpleChatServer::start_listening() {
    int ret;
    int num = 0;

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
        
        // if (mClientsMutex.lock(DEFAULT_MUTEX_LOCK_TIMEOUT)) {
        mClients.push_back(cl);
        printf("Added new client\n");
            // mClientsMutex.unlock();
        // } else { continue; }

        // create thread for client.
        num = (num < INT_MAX) ? num++ : 0;
        std::string name = "client_thread" + std::to_string(num);
        mThreads.push_back(std::make_unique<ClientThread>(name.c_str(), this, cl));
        mThreads.back()->run();

        Sleep(1);
    }
    
    // cleanup
    closesocket(mListenSocket);
}

int SimpleChatServer::send_to_all(const std::string &aMessage) const {
    int bytes_sent;
    int ret = 0;
    for (auto i = 0; i < mClients.size(); ++i) {
        bytes_sent = 0;
        if (auto pCl = mClients[i].lock()) {
            while (bytes_sent < aMessage.length()) {
                bytes_sent += send(pCl->mSocket, aMessage.c_str(), aMessage.length(), 0);
                if (bytes_sent == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    ret--;
                    break;
                }
            }
            ret++;
        }
    } 
    return ret;
}

void ClientThread::begin() {
    printf("Hello from thread\n");
}

void ClientThread::loop() {
    std::shared_ptr<Client> cl = mClient.lock();
    if (!cl) {
        kill();
        return;
    }

    int ret;
    int bytes_received = 0;
    int bytes_sent = 0;
    std::string message;
    // Receive until the peer shuts down the connection
    do {
        bytes_received = recv(cl->mSocket, cl->mBuffer.get(), CLIENT_BUFLEN, 0);
        if (bytes_received > 0) {
            printf("Bytes received: %d\n", bytes_received);
            printf("Message: %s\n", cl->mBuffer.get());
            message = cl->mBuffer.get();

            // // Echo the buffer back to the sender
            // bytes_sent = send(cl->mSocket, message.c_str(), bytes_received, 0);
            // if (bytes_sent == SOCKET_ERROR) {
            //     printf("send failed with error: %d\n", WSAGetLastError());
            //     closesocket(cl->mSocket);
            //     return;
            // }
            // printf("Bytes sent: %d\n", bytes_sent);

            mServer->send_to_all(message);

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