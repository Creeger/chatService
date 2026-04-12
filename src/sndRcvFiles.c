#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdint.h>
#include <endian.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

const char *downloadDir = "~/Documents/projects/chatService/Files/";

ssize_t getStream(int sock, void *buf, size_t dataLength) {
    uint64_t totalRead = 0; // To be recieved from the network
    char *ptr = buf;

    while (totalRead < dataLength) {
        ssize_t n = recv(sock, ptr + totalRead, dataLength - totalRead, 0);
        if (n < 0) {
            return -1; 
        }
        if (n == 0) {
            return 0;
        }
        totalRead += n;
    }
    return totalRead;
}


void sendFile(int sock, const char *filename) {
    printf("Sending file\n");
    char *tmp = strdup(filename);
    char *fileName = basename(tmp); // Get Wallpaper.jpg from path
    uint16_t fileNameLen = strlen(fileName);
    uint16_t netFileNameLen = htons(fileNameLen);

    send(sock, &netFileNameLen, sizeof(netFileNameLen), 0);
    send(sock, fileName, fileNameLen, 0);
    
    printf("filename: %s\n", filename);
    FILE *filePointer = fopen(filename, "rb");
    if (!filePointer) {
        perror("Unable to open file for sending\n");
        free(tmp);
        return;
    }
    free(tmp);


    // Get file size
    fseek(filePointer, 0, SEEK_END);
    uint64_t fileSize = ftell(filePointer); 
    rewind(filePointer);

    // Network hardware uses big endian, but CPU might use little endian instead
    uint64_t networkSize = htobe64(fileSize);
    send(sock, &networkSize, sizeof(networkSize), 0);

    char buff[1024]; // Determines how many bytes that fread() will read before sending
    size_t n;
    while ((n = fread(buff, 1, sizeof(buff), filePointer)) > 0) {
        send(sock, buff, n, 0); 
    }
    fclose(filePointer);
}


int getFile(int sock) {
    uint16_t networkNameLen;
    if (getStream(sock, &networkNameLen, sizeof(networkNameLen)) < 0) {
        printf("Could not get the file name length\n");
        return 1;
    }
    uint16_t nameLen = ntohs(networkNameLen);

    char *fileName = malloc(nameLen + 1);
    if (!fileName)  {
        printf("Could not malloc fileName\n");
        return 1;
    }

    if (getStream(sock, fileName, nameLen) < 0) {
        free(fileName);
        printf("Could not get file name\n");
        return 1;
    }
    fileName[nameLen] = '\0';

    uint64_t networkFileSize;
    if (getStream(sock, &networkFileSize, sizeof(networkFileSize)) < sizeof(networkFileSize)) {
        printf("Failed to get file size\n");
        return 1;
    }
    uint64_t fileSize = be64toh(networkFileSize);
    
    const char *home = getenv("HOME");
    if(!home) {
        printf("Could not get HOME directory\n");
        free(fileName);
        return 1;
    }
    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s%s", downloadDir, fileName);

    FILE *fp = fopen(fullPath, "wb");
    if (!fp) {
        printf("Failed to open file to write\n");
        return 1;
    }
    char buffer[4096];
    uint64_t totalRecieved = 0;

    printf("Downloading...\n");
    while (totalRecieved < fileSize) {
        uint64_t remaining = fileSize - totalRecieved;
        
        size_t toRead;
        if (remaining > sizeof(buffer)) {
            toRead = sizeof(buffer);
        } else {
            toRead = remaining;
        }

        ssize_t n = getStream(sock, buffer, toRead);
        if (n <= 0) {
            printf("Connection closed or error during file transfer\n");
            break;
        }

        fwrite(buffer, 1, n, fp);
        totalRecieved += n;

    }        
    fclose(fp);

    printf("Download complete\n");

    if (totalRecieved == fileSize) {
        printf("File recieved successfully\n");
    } else {
        printf("File transfer incomplete\n");
    }
    printf("Download Complete\n");
    return 0; 
}
