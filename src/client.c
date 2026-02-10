#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>





int main() {

    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server socket failedn");
        exit(EXIT_FAILURE);
    }

    struct sockaddr address = {
        AF_INET,
        htons(9999),
        0,
    };

    if (connect(sock_fd, &address, sizeof(address)) < 0) {
        perror("bind failed");
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

    for (;;) { // for (;;) means infinite loop

        char buffer[256] = { 0 };
            
        if (poll(fds, 2, 50000) < 0) {
            perror("server poll failed");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN) { // Using the bit operator & to check if revents include POLLIN
            if (read(0, buffer, 255) < 0) {
                perror("failed to read");
                exit(EXIT_FAILURE);
            }
            if (send(sock_fd, buffer, 255, 0) < 0) {
                perror("failed to send");
                exit(EXIT_FAILURE);
            }
        } else if (fds[1].revents & POLLIN) {
            if (recv(sock_fd, buffer, 255, 0) == 0) {
                return 0;
            } else {
                printf("Client: %s\n", buffer);
            }
            
        }
    }
    return 0;
}
