/*---------------------------------------------------------------------------
Copyright (C), 2024-2025, bl33h & Mendezg1
@author Sara Echeverria, Ricardo Mendez
FileName: server.c
@version: I
Creation: 19/03/2024
Last modification: 02/04/2024
------------------------------------------------------------------------------*/
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "chat.pb-c.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

// --- variables ---
#define BACKLOG 10
#define MAX_USERS 25
#define BUFFER_SIZE 1024

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// --- proto structure ---
typedef struct {
    char username[100];
    char ip[100];
    int socketFD;
    int status;
    time_t activityTimer;
}

// --- user structure ---
User;
User userList[MAX_USERS];
int numUsers = 0;
int clientsCount = 0; 

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// add a new user to the list
void addUser(char * username, char * ip, int socketFD, int status) {
    if (numUsers >= MAX_USERS) {
        printf("!SERVER AT FULL CAPACITY. COME BACK LATER\n");
        return;
    }
    User newUser;
    strcpy(newUser.username, username);
    strcpy(newUser.ip, ip);
    newUser.socketFD = socketFD;
    newUser.status = status;
    newUser.activityTimer = time(NULL);
    userList[numUsers] = newUser;
    numUsers++;
}

// remove a user from the list
void removeUser(char * username, char * ip, int socketFD, int status) {
    int i, j;
    for (i = 0; i < numUsers; i++) {
        User user = userList[i];
        if (strcmp(user.username, username) == 0 && strcmp(user.ip, ip) == 0 && user.socketFD == socketFD) {
            for (j = i; j < numUsers - 1; j++) {
                userList[j] = userList[j + 1];
            }
            numUsers--;
            printf("!user removed: %s\n", username);
            return;
        }
    }
    printf("%s hasn't signed up\n", username);
}

// verify if the user is already registered
int userExists(char * username) {
    int i;
    pthread_mutex_lock(&lock);
    for (i = 0; i < numUsers; i++) {
        if (strcmp(userList[i].username, username) == 0) {
            pthread_mutex_unlock(&lock);
            return 1;
        }
    }
    pthread_mutex_unlock(&lock);
    return 0;
}

// find a user by username
User* findUserByUsername(const char* username) {
    for (int i = 0; i < numUsers; i++) {
        if (strcmp(userList[i].username, username) == 0) {
            return &userList[i]; // User found, return a pointer to the user
        }
    }
    return NULL; // User not found
}

// status of the user
const char* convertStatusToString(int status) {
    switch (status) {
        case 1:
            return "ACTIVE";
        case 2:
            return "INACTIVE";
        case 3:
            return "BUSY";
        default:
            return "ACTIVE";
    }
}

// function to handle the clients requests
void * handleClient(void * arg) {
    int client_socket = *(int *) arg;
    char recv_buffer[BUFFER_SIZE];
    int recv_size = recv(client_socket, recv_buffer, sizeof(recv_buffer), 0);
    if (recv_size < 0) {
        perror("!error, unable to get message");
        exit(1);
    }
    Chat__ClientPetition *user_registration = chat__client_petition__unpack(NULL, recv_size, recv_buffer);
    if (user_registration == NULL) {
        fprintf(stderr, "!error, unable to unpack message\n");
        exit(1);
    }
    
    Chat__UserRegistration *chat_registration = user_registration->registration;

    printf("\n >connected >>user: %s  >>> ip: %s\n", chat_registration->username, chat_registration->ip);

    User MyInfo;
    strcpy(MyInfo.username, chat_registration->username);
    strcpy(MyInfo.ip, chat_registration->ip);
    MyInfo.socketFD = client_socket;

    Chat__ServerResponse server_response_registro = CHAT__SERVER_RESPONSE__INIT;

    if (!userExists(chat_registration->username)) {
        addUser(chat_registration->username, chat_registration->ip, client_socket, 1);
        server_response_registro.option = 0;
        server_response_registro.code = 200;
        server_response_registro.servermessage = "success";
    } else {
        server_response_registro.option = 0;
        server_response_registro.code = 400;
        server_response_registro.servermessage = "already signed up";
    }

    size_t serialized_size_servidor_registro = chat__server_response__get_packed_size(&server_response_registro);
    void *server_buffer_registro = malloc(serialized_size_servidor_registro);
    chat__server_response__pack(&server_response_registro, server_buffer_registro);

    if (send(MyInfo.socketFD, server_buffer_registro, serialized_size_servidor_registro, 0) < 0) {
        perror("!error unable to send response");
        exit(1);
    }
    free(server_buffer_registro);
    chat__client_petition__free_unpacked(user_registration, NULL);

    printf("\n\n [%s] ---> at the menu\n", MyInfo.username);
    while (1) {

        printf("\n");
        uint8_t  recv_buffer_option[BUFFER_SIZE];
        ssize_t recv_size_option = recv(client_socket, recv_buffer_option, sizeof(recv_buffer_option), 0);
        
        if (recv_size_option < 0) {
            perror("!error unable to receive message");
            exit(1);
        }

        if (recv_size_option == 0) {
            perror("!client disconnected");
            removeUser(MyInfo.username, MyInfo.ip, client_socket, MyInfo.status);
            break;
        }

        Chat__ClientPetition * client_option = chat__client_petition__unpack(NULL, recv_size_option, recv_buffer_option);
        if (client_option == NULL) {
            fprintf(stderr, "!error unable tu unpack\n");
            exit(1);
        }

        int selected_option = client_option -> option;
         printf("[%s] ---> Using option ---> [%d]", MyInfo.username, selected_option);

        switch (selected_option){

            // user registration, its already handled
            case 1:{
                
                break;
            }

            // connected users list
            case 2:{
                Chat__ConnectedUsersResponse connected_users_response = CHAT__CONNECTED_USERS_RESPONSE__INIT;
                connected_users_response.n_connectedusers = numUsers;
                connected_users_response.connectedusers = malloc(sizeof(Chat__UserInfo *) * numUsers);

                for (int i = 0; i < numUsers; i++) {
                    Chat__UserInfo *user_info = malloc(sizeof(Chat__UserInfo));
                    chat__user_info__init(user_info);
                    user_info->username = strdup(userList[i].username);
                    user_info->status = strdup(convertStatusToString(userList[i].status));
                    user_info->ip = strdup(userList[i].ip);
                    connected_users_response.connectedusers[i] = user_info;
                }

                Chat__ServerResponse server_response = CHAT__SERVER_RESPONSE__INIT;
                server_response.option = 2;
                server_response.code = 200;
                server_response.connectedusers = &connected_users_response;

                size_t serialized_size = chat__server_response__get_packed_size(&server_response);
                void *buffer = malloc(serialized_size);
                chat__server_response__pack(&server_response, buffer);

                if (send(MyInfo.socketFD, buffer, serialized_size, 0) < 0) {
                    perror("!error in sending connected users list");
                }

                // rfee allocated memory
                free(buffer);
                for (int i = 0; i < numUsers; i++) {
                    free(connected_users_response.connectedusers[i]->username);
                    free(connected_users_response.connectedusers[i]->status);
                    free(connected_users_response.connectedusers[i]->ip);
                    free(connected_users_response.connectedusers[i]);
                }
                free(connected_users_response.connectedusers);
                break;
            }

            // change status
            case 3:{
                Chat__ChangeStatus *status_change = client_option->change;
                for (int i = 0; i < numUsers; i++) {
                    if (strcmp(userList[i].username, status_change->username) == 0) {

                        // update the user's status based on the received string
                        if (strcmp(status_change->status, "ACTIVE") == 0) {
                            userList[i].status = 1;
                        } else if (strcmp(status_change->status, "INACTIVE") == 0) {
                            userList[i].status = 2;
                        } else if (strcmp(status_change->status, "BUSY") == 0) {
                            userList[i].status = 3;
                        } else if (strcmp(status_change->status, "activo") == 0) {
                            userList[i].status = 1;
                        } else if (strcmp(status_change->status, "inactivo") == 0) {
                            userList[i].status = 2;
                        } else if (strcmp(status_change->status, "ocupado") == 0) {
                            userList[i].status = 3;
                        }else {

                            // handle unexpected status string
                            printf("Received unexpected status string: %s\n", status_change->status);
                        }

                        userList[i].activityTimer = time(NULL);

                        // prepare and send a server response to confirm the status change
                        Chat__ServerResponse server_response = CHAT__SERVER_RESPONSE__INIT;
                        server_response.option = 3;
                        server_response.code = 200;
                        server_response.change = status_change;
                        size_t response_size = chat__server_response__get_packed_size(&server_response);
                        void *response_buf = malloc(response_size);
                        chat__server_response__pack(&server_response, response_buf);

                        if (send(MyInfo.socketFD, response_buf, response_size, 0) < 0) {
                            perror("Failed to send status change confirmation");
                        }

                        free(response_buf);

                        break;
                    }
                }
                break;
            }

            case 4:{
                Chat__MessageCommunication *received_message = client_option->messagecommunication;
                if (strcmp(received_message->recipient, "everyone") == 0 || strcmp(received_message->recipient, "") == 0){
                    printf("\n");

                    for (int i = 0; i < numUsers; i++){
                        if (strcmp(userList[i].username, MyInfo.username) == 0){
                            if (userList[i].status == 3){
                                userList[i].status = 1;
                            }
                            userList[i].activityTimer = time(NULL);
                            continue;
                        }

                        Chat__ServerResponse server_response = CHAT__SERVER_RESPONSE__INIT;
                        server_response.option= 4;
                        server_response.code = 200;
                        server_response.messagecommunication = received_message;

                        size_t serialized_size_server = chat__server_response__get_packed_size(&server_response);
                        void *server_buffer = malloc(serialized_size_server);
                        chat__server_response__pack(&server_response, server_buffer);

                        if (send(userList[i].socketFD, server_buffer, serialized_size_server, 0) < 0){
                            perror("!error in response");
                            exit(1);
                        }

                        free(server_buffer);
                    }
                }
                else{
                    printf("\n");

                    int sendMessage = 0;
                    int userId = 0;
                    for (int i = 0; i < numUsers; i++){
                        if (strcmp(userList[i].username, received_message->recipient) == 0){
                            userList[i].activityTimer = time(NULL);
                            sendMessage = 1;
                            userId = i;
                        }
                    }

                    if (sendMessage == 1){
                        Chat__ServerResponse server_response = CHAT__SERVER_RESPONSE__INIT;
                        server_response.option= 4;
                        server_response.code = 200;
                        server_response.messagecommunication = received_message;

                        size_t serialized_size_server = chat__server_response__get_packed_size(&server_response);
                        void *server_buffer = malloc(serialized_size_server);
                        chat__server_response__pack(&server_response, server_buffer);

                        if (send(userList[userId].socketFD, server_buffer, serialized_size_server, 0) < 0){
                            perror("!error in response");
                            exit(1);
                        }

                        free(server_buffer);
                    }
                    else{

                        Chat__ServerResponse server_response = CHAT__SERVER_RESPONSE__INIT;
                        server_response.option= 4;
                        server_response.code = 400;
                        server_response.servermessage = "!error, user not found";
                        server_response.servermessage = received_message->message;

                        size_t serialized_size_server = chat__server_response__get_packed_size(&server_response);
                        void *server_buffer = malloc(serialized_size_server);
                        chat__server_response__pack(&server_response, server_buffer);

                        if (send(MyInfo.socketFD, server_buffer, serialized_size_server, 0) < 0){
                            perror("!error in response");
                            exit(1);
                        }

                        free(server_buffer);
                    }
                }
            }

            // user info
            case 5: { // Assuming 5 is for user info request
                Chat__UserRequest *user_request = client_option->users;
                User *user = findUserByUsername(user_request->user);
                Chat__ServerResponse server_response = CHAT__SERVER_RESPONSE__INIT;
                chat__server_response__init(&server_response);
                
                if (user) {
                    // User found, prepare and send user info
                    Chat__UserInfo user_info = CHAT__USER_INFO__INIT;
                    chat__user_info__init(&user_info);
                    user_info.username = user->username;
                    user_info.status = convertStatusToString(user->status);
                    user_info.ip = user->ip;

                    server_response.option = 5;
                    server_response.code = 200;
                    server_response.userinforesponse = &user_info;
                } else {
                    // User not found, prepare and send error message
                    server_response.option = 5;
                    server_response.code = 404;
                    server_response.servermessage = "User not found";
                }

                size_t serialized_size = chat__server_response__get_packed_size(&server_response);
                void *buffer = malloc(serialized_size);
                chat__server_response__pack(&server_response, buffer);

                if (send(client_socket, buffer, serialized_size, 0) < 0) {
                    perror("Failed to send user information or error message");
                }

                free(buffer);
                break;
            }
            
            default:{
                fprintf(stderr, "!error invalid option %d\n", selected_option);
                break;
            }
        }

        chat__client_petition__free_unpacked(client_option, NULL);
    }
    close(client_socket);
}


// main method
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("!error opening socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("!error on binding");
        exit(EXIT_FAILURE);
    }

    listen(server_socket, BACKLOG);
    printf("Server listening on port [%d]\n", server_port);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        
        if (client_socket < 0) {
            perror("!error on accept client");

            // continue to accept next connection
            continue; 
        }

        printf("Client connected from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        pthread_t thread;
        if (pthread_create(&thread, NULL, handleClient, (void *)&client_socket) != 0) {
            perror("!error on creating thread");

            // close the client socket if thread creation fails
            close(client_socket); 

            // continue to accept next connection
            continue;
        }

        // detach the thread to free resources upon completion
        pthread_detach(thread); 
    }

    // close the server socket when done
    close(server_socket); 

    // cleanup the mutex
    pthread_mutex_destroy(&lock); 
    return 0;
}
