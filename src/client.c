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
#include <netinet/in.h>
#include <arpa/inet.h>
#include "chat.pb-c.h"
#include <pthread.h>
#include <ifaddrs.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// --- variables ---
#define BUFFER_SIZE 1024
int clientSocket = 0;

// --- methods ---
// function to display the menu
void menu() {
    printf("\n--- Welcome to the chat ---\n");
    printf("1. Chat with everyone (broadcast)\n");
    printf("2. Send a direct message (DM)\n");
    printf("3. View connected users\n");
    printf("4. See user information\n");
    printf("5. Change Status");
    printf("6. Help\n");
    printf("7. Exit\n");
    printf("Select an option: ");
}

// help menu
void helpCenter() {
    printf("\n-<-- Help center --->\n");
    printf("1. Here you will be in the general room where you can chat with everybody\n");
    printf("2. Here you will be able to send a direct message to the user of your choice\n");
    printf("3. Here you will see everybody that's connected to the server\n");
    printf("4. Here you can see the information of the connected users\n");
    printf("5. Here you can change your status\n");
    printf("6. Here you will see this complete message.\n");
    printf("7. Here you will exit the chat\n");
}

// function to handle the server response
void *serverResponse(void *arg) {
    int socket = *(int *)arg;
    uint8_t buffer_rx[BUFFER_SIZE];
    ssize_t bytesR;

    while (1) {
        bytesR = recv(socket, buffer_rx, BUFFER_SIZE, 0);
        if (bytesR < 0) {
            printf("!error failed response\n");
            continue;
        }

        Chat__ServerResponse *answer = chat__server_response__unpack(NULL, bytesR, buffer_rx);
        if (answer == NULL) {
            printf("!error during the unpacking process\n");
            continue;
        }

        // handling server response based on the option field
        if (answer->option != 0){
        	printf("Server response option: %d\n", answer->option);
        }
        switch (answer->option) {
            // registration response
            case 1: break;
            // connected users list
            case 2: 
                printf("Connected users:\n");
                for (size_t i = 0; i < answer->connectedusers->n_connectedusers; i++) {
                    Chat__UserInfo *connected_user = answer->connectedusers->connectedusers[i];
                    
                    printf("Username: %s\n", connected_user->username);
                    printf("\n");
                }
                break;
            //status
            case 3:
                break;
            //mesage
            case 4: 
                if (strcmp(answer->messagecommunication->recipient, "everyone") == 0 || strcmp(answer->messagecommunication->recipient, "") == 0) {
                    printf("[GLOBAL] %s: %s\n", answer->messagecommunication->sender, answer->messagecommunication->message);
                } else {
                    if (answer->code == 200 && answer->messagecommunication) {
                        printf("[PRIVATE] %s: %s\n", answer->messagecommunication->sender, answer->messagecommunication->message);
                    }
                }
                break;
            
            // info específica
            case 5: 
                break;
            default:
                printf("Unknown option received from server.\n");
                break;
        }

        chat__server_response__free_unpacked(answer, NULL);
    }

    return NULL;
}



// function to get the user status
char* userStatus(int status_value){
    char* response = malloc(sizeof(char) * 40);
    switch(status_value){
        case 1:{
            strcpy(response, "ACTIVE");
            break;
        }
        case 2:{
            strcpy(response, "INACTIVE");
            break;
        }
        case 3:{
            strcpy(response, "BUSY");
            break;
        }
        default:
            break;
    }

    return response;
}

// main function
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <username> <serverIP> <serverPort>\n", argv[0]);
        return -1;
    }

    struct sockaddr_in serverAddress;
    char *username = argv[1];
    char *serverIp = argv[2];
    int serverPort = atoi(argv[3]);
    int userOption = 1;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("socket");
        return -1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIp, &serverAddress.sin_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("connect");
        return -1;
    }

    // user registration
    Chat__UserRegistration registration = CHAT__USER_REGISTRATION__INIT;
    registration.username = username;
    registration.ip = "127.0.0.1"; 

    Chat__ClientPetition petition = CHAT__CLIENT_PETITION__INIT;
    petition.option = userOption; 
    petition.registration = &registration;

    size_t len = chat__client_petition__get_packed_size(&petition);
    uint8_t *buf = malloc(len);
    chat__client_petition__pack(&petition, buf);

    if (send(clientSocket, buf, len, 0) < 0) {
        perror("send");
        free(buf);
        return -1;
    }
    free(buf);

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, serverResponse, &clientSocket) != 0) {
        perror("pthread_create");
        return -1;
    }

    while (userOption != 5) {
         menu();
        scanf("%d", &userOption);
        switch (userOption){
            case 1:{
                char message_content[BUFFER_SIZE];

                printf("Your message: ");
                scanf(" %[^\n]", message_content);

                Chat__MessageCommunication userMessage = CHAT__MESSAGE_COMMUNICATION__INIT;
                userMessage.message = message_content;
                userMessage.recipient = "everyone";
                userMessage.sender = username;

                Chat__ClientPetition userOption_new = CHAT__CLIENT_PETITION__INIT;
                userOption_new.option = 4;
                userOption_new.messagecommunication = &userMessage;

                size_t serialized_size_option = chat__client_petition__get_packed_size(&userOption_new);
                uint8_t *buffer_option = malloc(serialized_size_option);
                chat__client_petition__pack(&userOption_new, buffer_option);
                
                if (send(clientSocket, buffer_option, serialized_size_option, 0) < 0) {
                    perror("!error in the message");
                    exit(1);
                }

                free(buffer_option);
                printf("\n");
                break;
            }
            case 2:{
                char destination[BUFFER_SIZE];

                printf("Insert destinatary's username: ");
                scanf(" %[^\n]", destination);
                
                char message_content[BUFFER_SIZE];

                printf("Your message: ");
                scanf(" %[^\n]", message_content);

                Chat__MessageCommunication userMessage = CHAT__MESSAGE_COMMUNICATION__INIT;
                userMessage.message = message_content;
                userMessage.recipient = destination;
                userMessage.sender = username;
                Chat__ClientPetition userOption_new = CHAT__CLIENT_PETITION__INIT;
                userOption_new.option = 4;
                userOption_new.messagecommunication = &userMessage;

                size_t serialized_size_option = chat__client_petition__get_packed_size(&userOption_new);
                uint8_t *buffer_option = malloc(serialized_size_option);
                chat__client_petition__pack(&userOption_new, buffer_option);
                
                if (send(clientSocket, buffer_option, serialized_size_option, 0) < 0) {
                    perror("!error in the message");
                    exit(1);
                }

                free(buffer_option);
                printf("\n");
                break;
            }

            case 3:{
                Chat__ClientPetition userOption_new = CHAT__CLIENT_PETITION__INIT;
                userOption_new.option = 2;
                userOption_new.registration = &registration;
                
                size_t serialized_size_option = chat__client_petition__get_packed_size(&userOption_new);
                uint8_t *buffer_option = malloc(serialized_size_option);
                chat__client_petition__pack(&userOption_new, buffer_option);
                
                if (send(clientSocket, buffer_option, serialized_size_option, 0) < 0) {
                    perror("!error in the message");
                    exit(1);
                }

                free(buffer_option);
                printf("\n");
                break;
            }

            case 4:{
                break;
            }

            case 5:{
                break;
            }

            case 6:{
                helpCenter();
            }

            case 7:{
                userOption = 5;
                break;
            }

            default:
                break;
        }
    }
    close(clientSocket);
    return 0;
}