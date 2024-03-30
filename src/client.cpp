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
#include "chat.pb.h"
#include <iostream>
#include <unistd.h>

void menu();
void help();
void sendMessage(int sock);

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // initialize protocol buffers library
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // socket creation
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "\nSocket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080); // port number

    // binary ip convertion
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "\nInvalid address/ Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "\nConnection Failed\n";
        return -1;
    }

    menu();

    int command = 0;
    do {
        std::cout << "Enter command: ";
        std::cin >> command;

        switch (command) {
            case 1:
                sendMessage(sock);
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

    // socket cleanup
    close(sock);
    google::protobuf::ShutdownProtobufLibrary();

    return 0;
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

void sendMessage(int sock) {
    chat::MessageCommunication msg;

    // sender
    msg.set_sender("ClientUsername"); 

    // recipient
    msg.set_recipient("General"); 
    std::string messageContent;

    // input
    std::cout << "Enter your message: ";
    std::cin.ignore();
    std::getline(std::cin, messageContent);
    msg.set_message(messageContent);

    // serialize message
    std::string serializedMsg;
    msg.SerializeToString(&serializedMsg);

    send(sock, serializedMsg.c_str(), serializedMsg.size(), 0);

    std::cout << "Message sent to server.\n";
}