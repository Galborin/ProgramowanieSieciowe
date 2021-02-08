#include "simple_chat_server.h"

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

void SimpleChatServer::start_listening() {
    int ret;
    
    while(true) {
        ret = listen(mListenSocket, SOMAXCONN);
        if (ret == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(mListenSocket);
            break;
        }

        // Accept a client socket
        mClientSocket = accept(mListenSocket, NULL, NULL);
        if (mClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(mListenSocket);
            break;
        }

        handle_client();

        Sleep(1);
    }
    
    // cleanup
    closesocket(mListenSocket);
}

void SimpleChatServer::handle_client() {
    int ret;
    int recvbuflen = sizeof(mRecvbuf);
    int bytes_received = 0;
    int bytes_sent = 0;
    // Receive until the peer shuts down the connection
    do {

        bytes_received = recv(mClientSocket, mRecvbuf, recvbuflen, 0);
        if (bytes_received > 0) {
            printf("Bytes received: %d\n", bytes_received);

        // Echo the buffer back to the sender
            bytes_sent = send(mClientSocket, mRecvbuf, bytes_received, 0);
            if (bytes_sent == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(mClientSocket);
                break;
            }
            printf("Bytes sent: %d\n", bytes_sent);
        }
        else if (bytes_received == 0) {
            printf("Connection closing...\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(mClientSocket);
            break;
        }
    } while (bytes_received > 0);

    // shutdown the connection since we're done
    ret = shutdown(mClientSocket, SD_SEND);
    if (ret == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(mClientSocket);
        return;
    }

    // cleanup
    closesocket(mClientSocket);
}
