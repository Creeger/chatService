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
        perror("server socket failedn");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (inet_pton(AF_INET, "192.168.10.146", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
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
    printf("Client: ");
    fflush(stdout);

    for (;;) {
        char buffer[1024] = { 0 };
        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }


        if (fds[0].revents & POLLIN) {
            read(0, buffer, 255);
            if (send(sock_fd, buffer, 255, 0) < 0) {
                perror("failed to send");
                exit(EXIT_FAILURE);
            }
            printf("Client: ");
            fflush(stdout);
        } else if (fds[1].revents & POLLIN) {
            if (recv(sock_fd, buffer, 255, 0) == 0) {
                return 0;
            } else {
                printf("\r");
                printf("%*s\r", 80, "");
                printf("Server: %s\n", buffer);
            }
            printf("Client: ");
            fflush(stdout);
        }
    }
    close(sock_fd);
    return 0;
}
