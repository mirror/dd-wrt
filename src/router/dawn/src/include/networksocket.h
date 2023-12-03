#ifndef __DAWN_NETWORKSOCKET_H
#define __DAWN_NETWORKSOCKET_H

#include <pthread.h>

/**
 * Init a socket using the runopts.
 * @param _ip - ip to use.
 * @param _port - port to use.
 * @param _multicast_socket - if socket should be multicast or broadcast.
 * @return the socket.
 */
int init_socket_runopts(const char *_ip, int _port, int _multicast_socket);

/**
 * Send message via network.
 * @param msg
 * @param is_enc
 * @return
 */
int send_string(char *msg, bool is_enc);

/**
 * Close socket.
 */
void close_socket();

// save connections
// struct sockaddr_in addr[100];

#endif
