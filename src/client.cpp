/*---------------------------------------------------------------------------
Copyright (C), 2024-2025, bl33h & Mendezg1
@author Sara Echeverria, Ricardo Mendez
FileName: client.cpp
@version: I
Creation: 19/03/2024
Last modification: 30/03/2024
------------------------------------------------------------------------------*/
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include "chat.pb.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <string>
#include <atomic>

// ------- variables -------
void sendMessage(int sock, const std::string& username);
std::atomic<bool> isInGeneralRoom{false};
void receiveMessages(int sock);
void menu();

// ------- methods -------
// print local IP addresses
void printLocalIPAddresses() {
    struct ifaddrs *addrs, *tmp;
    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            std::cout << "Interface: " << tmp->ifa_name << " IP Address: " << inet_ntoa(pAddr->sin_addr) << std::endl;
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

// available options menu
void menu() {
    std::cout << "\n";
    const char *commands[] = {
        "1. General message (general room)",
        "2. Private message (DM)",
        "3. Change status",
        "4. View connected users",
        "5. User info",
        "6. Help",
        "7. Exit"};

    std::cout << "--- Available options ---\n";
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        std::cout << "- " << commands[i] << "\n";
    }
}

// help menu
void help() {
    const char *commands[] = {
        "1 <message>: sends a message to all connected users",
        "2 <user> <message>: sends a direct message to the specified user",
        "3 <status>: changes your status to the specified value (0: active, 1: busy, 2: inactive)",
        "4: shows a list of all connected users",
        "5 <username>: displays information about the specified user",
        "6: shows this help message",
        "7: exits the chat client"};

    std::cout << "\n--- Command syntax ---\n";
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        std::cout << "- " << commands[i] << "\n";
    }
}

// send message in the general room
void sendMessage(int sock, const std::string& username) {
    chat::MessageCommunication msg;
    msg.set_sender(username);
    std::string messageContent;
    std::cout << "Enter your message: ";
    std::cin.ignore();
    std::getline(std::cin, messageContent);
    msg.set_message(messageContent);

    // serialize the message to a string
    std::string serializedMsg;
    if (!msg.SerializeToString(&serializedMsg)) {
        std::cerr << "Failed to serialize message." << std::endl;
        return;
    }

    // send the serialized message to the server
    send(sock, serializedMsg.c_str(), serializedMsg.size(), 0);
    std::cout << "Message sent to server.\n";
}

// receive messages from the server
void receiveMessages(int sock) {
    while (true) {
        char buffer[1024] = {0};
        int bytesReceived = read(sock, buffer, 1024);
        if (bytesReceived <= 0) {
            // Handle disconnects or errors
            break;
        }

        chat::MessageCommunication msg;
        if (msg.ParseFromArray(buffer, bytesReceived)) {
            if (isInGeneralRoom.load()) {
                // Temporarily clear the line and display the incoming message
                std::cout << "\r\033[K" << msg.sender() << ": " << msg.message() << std::endl;
                
                // Re-display the input prompt
                std::cout << "[Your message]: " << std::flush;
            }
        } else {
            std::cerr << "\r\033[KFailed to parse message." << std::endl;
            std::cout << "[Your message]: " << std::flush;
        }
    }
}

// main method
int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = std::stoi(argv[1]);
    int sock = 0;
    std::string username;
    struct sockaddr_in serv_addr;

    // socket creation
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "\nSocket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // initialize serv_addr.sin_addr.s_addr
    if(inet_pton(AF_INET, "172.18.80.3", &serv_addr.sin_addr)<=0) {
        std::cerr << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    // connection
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "\nConnection Failed\n";
        return -1;
    }

    std::cout << "<------- Welcome to the chat ------->" << std::endl;
    std::cout << "<< Your info >>" << std::endl;
    printLocalIPAddresses();
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);
    chat::MessageCommunication msg;
    msg.set_sender(username);
    std::string serializedMsg;
    msg.SerializeToString(&serializedMsg);
    send(sock, serializedMsg.c_str(), serializedMsg.size(), 0);
    std::thread(receiveMessages, sock).detach(); 

    menu();
    int command = 0;
    do {
    std::cout << "Enter option: \n";
    std::cin >> command;

    if(std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        continue;
    }
    bool
     userIsTyping = false;
    switch (command) {
        case 1:
            isInGeneralRoom.store(true);
            std::cout << "<<<--- You are now in the general chatroom. Type '/exit' to leave --->>>\n";
            do {
                std::cout << "[Your message]: ";
                std::string messageContent;
                std::getline(std::cin >> std::ws, messageContent); // Capture user message

                if (messageContent == "/exit") {
                    break;
                }

                // set flag indicating user is typing a message
                userIsTyping = true;

                chat::MessageCommunication msg;
                msg.set_sender(username);
                msg.set_message(messageContent);

                // serialize and send the message
                std::string serializedMsg;
                if (!msg.SerializeToString(&serializedMsg)) {
                    std::cerr << "Failed to serialize message.\n";

                    // reset flag as sending failed
                    userIsTyping = false; 
                    continue;
                }

                send(sock, serializedMsg.c_str(), serializedMsg.size(), 0);

                // reset flag after sending
                userIsTyping = false;

                // ensure clear separation after sending
                std::cout << "\n"; 
            } while (true);

            isInGeneralRoom.store(false);
            break;

        case 2:
            break;

        case 3:
            break;

        case 4:
            break;

        case 5:
            break;

        case 6:
            help();
            break;

        case 7:
            std::cout << "Exiting...\n";
            break;

        default:
            std::cout << "Invalid command. Please try again.\n";
            break;
    }
} while (command != 7);
    close(sock);
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}