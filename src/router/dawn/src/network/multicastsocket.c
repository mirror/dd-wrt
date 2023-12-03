#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "multicastsocket.h"

// based on: http://openbook.rheinwerk-verlag.de/linux_unix_programmierung/Kap11-018.htm

static struct ip_mreq command;

int setup_multicast_socket(const char *_multicast_ip, unsigned short _multicast_port, struct sockaddr_in *addr) {
    int loop = 1;
    int sock;

    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(_multicast_ip);
    addr->sin_port = htons (_multicast_port);

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        dawnlog_perror("socket()");
        exit(EXIT_FAILURE);
    }

    // allow multiple processes to use the same port
    loop = 1;
    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &loop, sizeof(loop)) < 0) {
        dawnlog_perror("setsockopt:SO_REUSEADDR");
        // FIXME: Is close() required?
        close(sock);
        exit(EXIT_FAILURE);
    }
    if (bind(sock,
             (struct sockaddr *) addr,
             sizeof(*addr)) < 0) {
        dawnlog_perror("bind");
        // FIXME: Is close() required?
        close(sock);
        exit(EXIT_FAILURE);
    }

    loop = 0;
    if (setsockopt(sock,
                   IPPROTO_IP,
                   IP_MULTICAST_LOOP,
                   &loop, sizeof(loop)) < 0) {
        dawnlog_perror("setsockopt:IP_MULTICAST_LOOP");
        // FIXME: Is close() required?
        close(sock);
        exit(EXIT_FAILURE);
    }

    // join broadcast group
    command.imr_multiaddr.s_addr = inet_addr(_multicast_ip);
    command.imr_interface.s_addr = htonl (INADDR_ANY);
    if (command.imr_multiaddr.s_addr == -1) {
        dawnlog_perror("Wrong multicast address!\n");
        // FIXME: Is close() required?
        close(sock);
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sock,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   &command, sizeof(command)) < 0) {
        dawnlog_perror("setsockopt:IP_ADD_MEMBERSHIP");
        // FIXME: Is close() required?
        close(sock);
    }
    return sock;
}

int remove_multicast_socket(int socket) {
    if (setsockopt(socket,
                   IPPROTO_IP,
                   IP_DROP_MEMBERSHIP,
                   &command, sizeof(command)) < 0) {
        dawnlog_perror("setsockopt:IP_DROP_MEMBERSHIP");
        return -1;
    }
    return 0;
}
