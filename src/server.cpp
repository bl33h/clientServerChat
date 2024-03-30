/*---------------------------------------------------------------------------
Copyright (C), 2024-2025, bl33h & Mendezg1
@author Sara Echeverria, Ricardo Mendez
FileName: server.cpp
@version: I
Creation: 19/03/2024
Last modification: 19/03/2024
------------------------------------------------------------------------------*/
#include <sys/socket.h>
#include <netinet/in.h>
#include "chat.pb.h"  
#include <iostream>
#include <unistd.h>
#include <string>

int main() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(serverFd, (struct sockaddr *)&address, sizeof(address));
    listen(serverFd, 3);

    newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    // read message from client
    read(newSocket, buffer, 1024);
    chat::MessageCommunication msg;
    msg.ParseFromArray(buffer, 1024);

    std::cout << "Message received: " << msg.message() << std::endl;

    close(newSocket);
    close(serverFd);
    return 0;
}