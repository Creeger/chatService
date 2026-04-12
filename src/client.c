#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "getIpAddr.h"
#include "sndRcvFiles.h"
#include <inttypes.h>


#define MSG_TEXT 1
#define MSG_FILE 2


void clientReceiveFromServer(int sock);
void  clientSendToServer(int sock);

int main() {
    printf("To type a text message, simply type the message and hit enter\n");
    printf("To send a file, type the command '/file' and then provide the full path and name of the file\n");

    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server socket failedn");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);

    char *client_ip = getIpAddr();

    if (client_ip) {
        printf("Local IP: %s\n", client_ip);
    } else {
        printf("Could not get IP\n");
    }
    
    char* server_ip;
    if (strcmp(client_ip, "192.168.10.145") == 0) {
        server_ip = "192.168.10.145"; //"192.168.10.188";
    } else {
        server_ip = "192.168.10.145";
    }
    free(client_ip);


    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    printf("connecting to: %s\n", server_ip);
    if (connect(sock_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    // stdin - 0
    struct pollfd fds[2] = {
            {
            0,
            POLLIN,
            0
        },
        {
            sock_fd,
            POLLIN,
            0,
        }
    };
    printf("Client: ");
    fflush(stdout);

    for (;;) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        // Send something to the server
        if (fds[0].revents & POLLIN) {
            clientSendToServer(sock_fd);
        }
        // Recieve something from the server
        if (fds[1].revents & POLLIN) {
            clientReceiveFromServer(sock_fd);
        } 
    }
    close(sock_fd);
    return 0;
}


void clientSendToServer(int sock) {
    char input[1024];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        perror("clientSendToServer(): fgets failed");
        return;
    }

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
    printf("Client: ");
    fflush(stdout);
}

void clientReceiveFromServer(int sock) {
        uint8_t messageType;
        if (recv(sock, &messageType, 1, 0) <= 0) {
            return;
        }

        if (messageType == MSG_TEXT) {
            uint32_t messageLength;
            
            if (recv(sock, &messageLength, sizeof(messageLength), 0) <= 0) {
                printf("Client failed to get message length\n");
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
            printf("Server: %s\n", buffer);

            free(buffer);
        
            printf("Client: ");
            fflush(stdout); 

        } else if (messageType == MSG_FILE) { // File is being sent
            int result = getFile(sock);
            if (result < 0) {
                printf("Unable to receive file\n");
            }
            printf("Client: ");
            fflush(stdout); 
        }
    return;
}
