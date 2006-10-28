#ifndef SETNONBLOCK_H
#define SETNONBLOCK_H

#include "./def.h"
#include <sys/types.h>

#ifdef WINDOWS
#include <Winsock2.h>
#else
#include <sys/socket.h>
#endif

int setfd_nonblock(SOCKET socket);

#endif
