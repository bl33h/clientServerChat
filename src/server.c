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
<<<<<<< HEAD
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
=======
#include <sys/socket.h>
#include <netinet/in.h>
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
#include <pthread.h>
#include "chat.pb-c.h"

#define PORT 8080
#define MAX_CLIENTS 10

<<<<<<< HEAD
int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(uint8_t *buffer, ssize_t buf_len, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0 && client_sockets[i] != sender_sock) {
            if (send(client_sockets[i], buffer, buf_len, 0) == -1) {
                perror("send");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    uint8_t buffer[1024];
    ssize_t bytes_received;

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_sock, (struct sockaddr *)&addr, &addr_len);

    while (1) {
        bytes_received = recv(client_sock, buffer, 1024, 0);
        if (bytes_received <= 0) {
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == client_sock) {
                    client_sockets[i] = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            close(client_sock);
            break;
        }

        Chat__MessageCommunication *msg;
        msg = chat__message_communication__unpack(NULL, bytes_received, buffer);
        if (msg != NULL) {
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
            // Modified to print in the desired format
            printf("message from %s %s: %s\n", msg->sender, ip_str, msg->message);
            broadcast_message(buffer, bytes_received, client_sock);
            chat__message_communication__free_unpacked(msg, NULL);
        } else {
            fprintf(stderr, "Error unpacking incoming message\n");
        }
    }

=======
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
        chat__message_communication__free_unpacked(msg, NULL);
    }

    // Communication with client
    while ((bytes_read = read(clientSocket, buffer, 1024)) > 0) {
        msg = chat__message_communication__unpack(NULL, bytes_read, buffer);
        if (msg == NULL) {
            fprintf(stderr, "Error unpacking incoming message\n");
            continue;
        }
        printf("Message from %s: %s\n", msg->sender, msg->message);

        // Broadcasting message to other clients should be implemented here

        chat__message_communication__free_unpacked(msg, NULL);
    }

    // Cleanup
    close(clientSocket);
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
    return NULL;
}

int main() {
<<<<<<< HEAD
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, MAX_CLIENTS) == -1) {
=======
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
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
        perror("listen");
        exit(EXIT_FAILURE);
    }

<<<<<<< HEAD
    printf("Server started on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int new_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (new_sock == -1) {
=======
    // Accepting clients
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0) {
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
            perror("accept");
            continue;
        }

<<<<<<< HEAD
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = new_sock;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_sock;
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_sock_ptr) != 0) {
            perror("pthread_create");
=======
        pthread_t thread_id;
        new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        if (pthread_create(&thread_id, NULL, handle_client, (void*) new_sock) != 0) {
            perror("pthread_create failed");
>>>>>>> 839afa22ae8ae452d329f2a96e2cd40fd2cb82d3
        }
    }

    return 0;
}