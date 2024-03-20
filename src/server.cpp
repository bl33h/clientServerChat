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
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>

void handleClient(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, 1024);

    ssize_t bytesRead = read(clientSocket, buffer, 1024);
    if (bytesRead > 0) {
        std::cout << "-> message received from client: " << buffer << std::endl;

        std::string response = "--- message received successfully ---";
        write(clientSocket, response.c_str(), response.length());
    } else {
        std::cout << "!error reading client message or connection closed" << std::endl;
    }

    close(clientSocket);
}

int main() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(serverFd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true) {
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::thread t(handleClient, newSocket);
        t.detach();
    }

    return 0;
}