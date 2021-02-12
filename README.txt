CHAT SERVER

Simple chat server that listens on a given port.
The server creates a thread for every received connection.
Messages sent to the server are sent to all connected clients.

* CONTENT

The project includes chat server and client applications.

- src\ directory contains source code of the server
- include\ directory contains headers for osapi library
- client\ directory contains client source code
- libosapi.a is a library for thread operations 

* CONFIGURATION

- Server by default listens on port number defined by DEFAULT_PORT directive in src\main.cpp.
By default it is set to 27015.
All changes have to be applied to DEFAULT_PORT define in client\app_client.cpp also.

- Change the MAX_CLIENTS define value in simple_chat_server.h for max number of accepted connections.

* BUILD

You can build the server and the client with build_all batch file.
You should get server.exe and app_client.exe executables.

* RUN

To start the server, run the server.exe executable.
To run the client, run the app_client.exe executable with two parameters:

1. server-name: address of the server (i.e. 127.0.0.1)
2. user-name: name that will be displayed with sent messages

* CODE 

Server is an object of SimpleChatServer class (simple_chat_server.h).
It spawns osapi::ClientThread objects for every accepted client.
The created thread receives the messages from the client and executes send_to_all() server method.

Currently connected clients can be found in mClients vector in the server object.
Currently created threads can be found in mThreads vector in the server object.

Client class (src\client.h) objects store the client socket and message buffer.
