#include <fcntl.h>

#ifndef SNDRCVFILES_H
#define SNDRCVFILES_H

int getFile(int sock);

void sendFile(int sock, const char *filename);

int getStream(int sock, void *buf, size_t dataLength);

#endif
