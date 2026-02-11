#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>





int main() {

    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr address = {
        AF_INET,
        htons(9999),
        0,
    };

    if (bind(sock_fd, &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    int client_fd;
    if ((client_fd = accept(sock_fd, 0, 0)) < 0) {
        perror("accept");
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
            client_fd,
            POLLIN,
            0,
        }
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


        if (fds[0].revents & POLLIN) { // Using the bit operator & to check if revents include POLLIN

            ssize_t n = read(0, buffer, 255);

            if (n < 0) {
                perror("read failed");
                exit(EXIT_FAILURE);
            }
            if (send(client_fd, buffer, 255, 0) < 0) {
                perror("failed to send");
                exit(EXIT_FAILURE);
            }
            printf("Server: ");
            fflush(stdout);

        } else if (fds[1].revents & POLLIN) {
            if (recv(client_fd, buffer, 255, 0) == 0) {
                return 0;
            } else {
                printf("\r");
                printf("%*s\r", 80, "");
                printf("\n");
                printf("Client: %s\n", buffer);
            }
            printf("Server: ");
            fflush(stdout);
        }
    }
    printf("\n");
    return 0;
}
