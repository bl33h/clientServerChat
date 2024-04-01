/*---------------------------------------------------------------------------
Copyright (C), 2024-2025, bl33h & Mendezg1
@author Sara Echeverria, Ricardo Mendez
FileName: server.c
@version: I
Creation: 19/03/2024
Last modification: 31/03/2024
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "chat.pb-c.h"

#define PORT 8080
#define MAX_CLIENTS 10

// Thread function to handle clients
void* handle_client(void* arg) {
    int clientSocket = *(int*)arg;
    free(arg); // Since we dynamically allocated memory for the thread argument

    Chat__MessageCommunication *msg;
    uint8_t buffer[1024];

    // Read the username
    ssize_t bytes_read = read(clientSocket, buffer, 1024);
    if (bytes_read > 0) {
        msg = chat__message_communication__unpack(NULL, bytes_read, buffer);
        if (msg == NULL) {
            fprintf(stderr, "Error unpacking incoming message\n");
        }
        printf("User %s connected\n", msg->sender);
        // Free the unpacked message
        printf("Message from client: %s\n", msg->message);
        chat__message_communication__free_unpacked(msg, NULL);
    }

    // Cleanup
    close(clientSocket);
    return NULL;
}

int main() {
    int server_fd, new_socket, *new_sock;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accepting clients
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            continue;
        }

        pthread_t thread_id;
        new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        if (pthread_create(&thread_id, NULL, handle_client, (void*) new_sock) != 0) {
            perror("pthread_create failed");
        }
    }

    return 0;
}