/*---------------------------------------------------------------------------
Copyright (C), 2024-2025, bl33h & Mendezg1
@author Sara Echeverria, Ricardo Mendez
FileName: client.c
@version: I
Creation: 19/03/2024
Last modification: 31/03/2024
------------------------------------------------------------------------------*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "chat.pb-c.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVER_IP "127.0.0.1" // Adjust as necessary
#define SERVER_PORT 8080
#define MAX_BUF_SIZE 1024

void print_menu() {
    printf("\n--- Menu ---\n");
    printf("1. Send message\n");
    printf("2. Exit\n");
}

void send_message(int sock) {
    char message_content[MAX_BUF_SIZE];
    printf("Enter your message: ");
    fgets(message_content, MAX_BUF_SIZE, stdin);

    // Prepare the message
    Chat__MessageCommunication msg = CHAT__MESSAGE_COMMUNICATION__INIT;
    char sender[] = "YourUsername"; // Example username, replace as necessary
    char recipient[] = "everyone"; // or specify a recipient username
    msg.sender = sender;
    msg.recipient = recipient;
    msg.message = message_content;

    // Serialize the message
    unsigned len = chat__message_communication__get_packed_size(&msg);
    uint8_t *buf = malloc(len);
    chat__message_communication__pack(&msg, buf);

    // Send the message
    send(sock, buf, len, 0);

    free(buf);
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/Address not supported\n");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to the server.\n");

    // Main loop
    int running = 1;
    while (running) {
        print_menu();
        printf("Select an option: ");
        int option;
        scanf("%d", &option);
        getchar(); // consume newline

        switch (option) {
            case 1:
                send_message(sock);
                break;
            case 2:
                printf("Exiting...\n");
                running = 0;
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }

    close(sock);
    return 0;
}