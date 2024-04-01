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

<<<<<<< HEAD
#define MAX_BUF_SIZE 1024

char username[MAX_BUF_SIZE];
int clientSocket;

void print_menu() {
    printf("\n--- Available options ---\n");
    printf("1. Chat with everyone (general room)\n");
    printf("2. View connected users\n");
    printf("3. Change status\n");
    printf("4. User info\n");
    printf("5. Exit\n");
}

void send_message() {
    char message_content[MAX_BUF_SIZE];
    printf("Enter your message: ");
    fgets(message_content, MAX_BUF_SIZE, stdin);
    message_content[strcspn(message_content, "\n")] = 0; // Remove newline

    Chat__MessageCommunication msg = CHAT__MESSAGE_COMMUNICATION__INIT;
    char recipient[] = "everyone";
    msg.sender = username;
    msg.recipient = recipient;
    msg.message = message_content;

=======
// --- variables ---
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_BUF_SIZE 1024

// --- options ---
#define opSignIn 1
#define opConnectedUsers 2
#define opStatus 3
#define opChat 4
#define opUserInfo 5
#define opExit 6

// username pointer
char *username;

// other features
int state;
int clientSocket;
char connectedUsers;
char input[BUFFER_SIZE];
char userInfo[BUFFER_SIZE];
char content[BUFFER_SIZE];
char destination[BUFFER_SIZE];

// ------- methods -------
void print_menu() {
    printf("\n--- Available options ---\n");
    printf("1. SignIn (choose your username)\n");
    printf("2. View connected users \n");
    printf("3. Change status\n");
    printf("4. Chat \n");
    printf("5. User info\n");
    printf("6. Exit\n");
}

void send_message(int sock) {
    char message_content[MAX_BUF_SIZE];
    printf("Enter your message: ");
    fgets(message_content, MAX_BUF_SIZE, stdin);

    Chat__MessageCommunication msg = CHAT__MESSAGE_COMMUNICATION__INIT;
    char sender[] = "YourUsername";
    char recipient[] = "everyone";
    msg.sender = sender;
    msg.recipient = recipient;
    msg.message = message_content;

    // serialize the message
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
    unsigned len = chat__message_communication__get_packed_size(&msg);
    uint8_t *buf = malloc(len);
    chat__message_communication__pack(&msg, buf);

<<<<<<< HEAD
    send(clientSocket, buf, len, 0);
=======
    // send the message
    send(sock, buf, len, 0);
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3

    free(buf);
}

<<<<<<< HEAD
int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <username> <serverIP> <IPport>\n", argv[0]);
        return 1;
    }

    strcpy(username, argv[1]);
    char* serverIP = argv[2];
    int IPport = atoi(argv[3]);

    struct sockaddr_in serv_addr;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
=======
int main() {
    int sock;
    struct sockaddr_in serv_addr;

    // create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
<<<<<<< HEAD
    serv_addr.sin_port = htons(IPport);
    if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/Address not supported \n");
        return -1;
    }

    if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
=======
    serv_addr.sin_port = htons(SERVER_PORT);

    // ip convertion
    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/Address not supported\n");
        return -1;
    }

    // server connection
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
        perror("Connection Failed");
        return -1;
    }

<<<<<<< HEAD
    printf("Connected to the server as %s.\n", username);

    int option;
    do {
        print_menu();
        printf("Select an option: ");
        scanf("%d", &option);
        getchar(); // consume newline

        switch (option) {
            case 1: // Chat with everyone
                send_message();
                break;
            case 5: // Exit
                printf("Exiting...\n");
                close(clientSocket);
                exit(0);
            default:
                printf("Option not implemented.\n");
        }
    } while (option != 5);

=======
    printf("Connected to the server.\n");

    // main loop
    int running = 1;
    while (running) {
        print_menu();
        printf("Select an option: ");
        int option;
        scanf("%d", &option);

        // consume newline
        getchar(); 

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
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
    return 0;
}