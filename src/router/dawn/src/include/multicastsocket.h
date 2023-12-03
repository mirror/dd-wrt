#ifndef __DAWN_MULTICASTSTSOCKET_H
#define __DAWN_MULTICASTSSOCKET_H

#include <arpa/inet.h>

/**
 * Setup a multicast socket.
 * Setup permissions. Join the multicast group, etc. ...
 * @param _multicast_ip - multicast ip to use.
 * @param _multicast_port - multicast port to use.
 * @param addr
 * @return the multicast socket.
 */
int setup_multicast_socket(const char *_multicast_ip, unsigned short _multicast_port, struct sockaddr_in *addr);

/**
 * Removes the multicast socket.
 * @param socket
 * @return
 */
int remove_multicast_socket(int socket);

#endif
