/*---------------------------------------------------------------------------
Copyright (C), 2024-2025, bl33h & Mendezg1
@author Sara Echeverria, Ricardo Mendez
FileName: server.cpp
@version: I
Creation: 19/03/2024
Last modification: 30/03/2024
------------------------------------------------------------------------------*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "chat.pb.h"  
#include <iostream>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <map>

// ------- variables -------
std::map<int, std::pair<std::string, std::string>> clients; // Map socket to <username, IP>
std::mutex clientsMutex;

// ------- methods -------
// handle client messages
void handleClient(int clientSocket, sockaddr_in clientAddr) {
    char buffer[1024] = {0};
    std::string clientIP = inet_ntoa(clientAddr.sin_addr);
    
    // read the username
    read(clientSocket, buffer, 1024);
    chat::MessageCommunication msg;
    msg.ParseFromArray(buffer, 1024);

    // assume the first message from the client is the username
    std::string username = msg.sender(); 

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients[clientSocket] = std::make_pair(username, clientIP);
    }

    while (true) {
        // clear the buffer and read the new message
        memset(buffer, 0, sizeof(buffer));
        int readBytes = read(clientSocket, buffer, 1024);
        if (readBytes == 0) {
            // client disconnected
            break; 
        }
        msg.ParseFromArray(buffer, readBytes);
        std::cout << "Message from " << username << " (" << clientIP << "): " << msg.message() << std::endl;

        // broadcast message to all other clients
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& client : clients) {
            if (client.first != clientSocket) { // Don't echo back to the sender
                send(client.first, buffer, readBytes, 0);
            }
        }
    }

    // remove the disconnected client
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(clientSocket);
    }
    close(clientSocket);
}

// main method
int main() {
    int serverFd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(serverFd, (struct sockaddr *)&address, sizeof(address));

    // increase backlog for more pending connections
    listen(serverFd, 10); 

    while (true) {
        struct sockaddr_in clientAddr;
        int clientSocket = accept(serverFd, (struct sockaddr *)&clientAddr, (socklen_t*)&addrlen);

        // handle each client in a separate thread
        std::thread(handleClient, clientSocket, clientAddr).detach(); 
    }

    close(serverFd);
    return 0;
}