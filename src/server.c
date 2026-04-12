#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "sndRcvFiles.h"
#include <inttypes.h>


#define MSG_TEXT 1
#define MSG_FILE 2

void serverSendToClient(int sock);
void serverRecieveFromClient(int sock);

int main() {
    printf("To type a text message, simply type the message and hit enter\n");
    printf("To send a file, type the command '/file' and then provide the full path and name of the file\n");

    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
  

    address.sin_family = AF_INET;
    address.sin_port = htons(9999);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port 9999...\n");
    fflush(stdout);

    int client_fd;
    if ((client_fd = accept(sock_fd, 0, 0)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    // stdin - 0
    struct pollfd fds[2] = {
        {0, POLLIN, 0},
        {client_fd, POLLIN, 0}
    };


    printf("Server: ");
    fflush(stdout);

    for (;;) { // for (;;) means infinite loop
        char buffer[1024] = { 0 }; // { 0 } initializes all the values to 0
        int ret = poll(fds, 2, 50000);
        if (ret < 0) {
            perror("server poll failed");
            exit(EXIT_FAILURE);
        }
        if (ret == 0) {
            continue; // timeout, nothing to read
        }

        // Send something to the client
        if (fds[0].revents & POLLIN) { // Using the bit operator & to check if revents include POLLIN
            serverSendToClient(client_fd); 
        }

        // Receive something from the client
        if (fds[1].revents & POLLIN) {
            serverRecieveFromClient(client_fd);
        }
    }
    printf("\n");
    return 0;
}


void serverSendToClient(int sock) {
    char input[1024];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        perror("serverSendToClient(): fgets failed");
        return;
    }



    input[strcspn(input, "\n")] = '\0';
    
    if (strncmp(input, "/file ", 6) == 0) {
        input[strcspn(input, "\n")] = 0;
        char *path = input + 6;

        uint8_t messageType = MSG_FILE;
        send(sock, &messageType, 1, 0);

        sendFile(sock, path);
    } else {
        uint8_t messageType = MSG_TEXT;
        uint32_t messageLength = htonl(strlen(input));
        
        send(sock, &messageType, 1, 0);
        send(sock, &messageLength, sizeof(messageLength), 0);
        send(sock, input, strlen(input), 0);
    }
    printf("Server: ");
    fflush(stdout);
}

void serverRecieveFromClient(int sock) {
    uint8_t messageType;

    if (recv(sock, &messageType, 1, 0) <= 0) {
        return;
    }

    if (messageType == MSG_TEXT) { 
        uint32_t messageLength;

        if (recv(sock, &messageLength, sizeof(messageLength), 0) <= 0) {
            printf("Server failed to get the message length\n");
            return;
        }
        messageLength = ntohl(messageLength);

        char *buffer = malloc(messageLength + 1);
        if (recv(sock, buffer, messageLength, 0) == 0) {
            free(buffer);
            printf("Client failed to get message length\n");
            return;
        }
        buffer[messageLength] = '\0';

        printf("\r");
        printf("%*s\r", 80, "");
        printf("\n");
        printf("Client: %s\n", buffer);

        free(buffer);
    
        printf("Server: ");
        fflush(stdout); 

    } else if (messageType == MSG_FILE) {
        int result = getFile(sock);
        if (result < 0) {
            printf("Unable to recieve file\n");
        } 
        printf("Server: ");
        fflush(stdout);
    }
    return;
}
