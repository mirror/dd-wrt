#ifndef CONNECT_NONB_H
#define CONNECT_NONB_H

#include "./def.h"

#ifdef WINDOWS
#include <Winsock2.h>
#else
#include <sys/socket.h>
#endif

int connect_nonb(SOCKET sk,struct sockaddr *psa);

#endif
