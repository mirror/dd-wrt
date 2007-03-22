/***

revname.c - reverse DNS resolution module for IPTraf.  As of IPTraf 1.1,
this module now communicates with the rvnamed process to resolve in the
background while allowing the foreground process to continue with the
interim IP addresses in the meantime.

Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1998

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <curses.h>
#include "deskman.h"
#include "getpath.h"
#include "rvnamed.h"

char revname_socket[80];

char *gen_unix_sockname(void)
{
    static char scratch[80];

    srandom(time(NULL));
    snprintf(scratch, 80, "%s-%lu%d%ld", SOCKET_PREFIX,
             time(NULL), getpid(), random());

    return scratch;
}

int rvnamedactive(void)
{
    int fd;
    fd_set sockset;
    struct rvn rpkt;
    struct sockaddr_un su;
    int sstat;
    struct timeval tv;
    int fr;
    int br;
    char unix_socket[80];

    strncpy(unix_socket, get_path(T_WORKDIR, gen_unix_sockname()), 80);
    unlink(unix_socket);

    fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    su.sun_family = AF_UNIX;
    strcpy(su.sun_path, unix_socket);
    bind(fd, (struct sockaddr *) &su,
         sizeof(su.sun_family) + strlen(su.sun_path));

    su.sun_family = AF_UNIX;
    strcpy(su.sun_path, IPTSOCKNAME);

    rpkt.type = RVN_HELLO;

    sendto(fd, &rpkt, sizeof(struct rvn), 0, (struct sockaddr *) &su,
           sizeof(su.sun_family) + strlen(su.sun_path));

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&sockset);
    FD_SET(fd, &sockset);

    do {
        sstat = select(fd + 1, &sockset, NULL, NULL, &tv);
    } while ((sstat < 0) && (errno != ENOMEM) && (errno == EINTR));

    if (sstat == 1) {
        fr = sizeof(su.sun_family) + strlen(su.sun_path);
        do {
            br = recvfrom(fd, &rpkt, sizeof(struct rvn), 0,
                          (struct sockaddr *) &su, &fr);
        } while ((br < 0) && (errno == EINTR));

        if (br < 0)
            printipcerr();
    }

    close(fd);
    unlink(unix_socket);

    if (sstat == 0)
        return 0;
    else
        return 1;
}

/*
 * Terminate rvnamed process
 */

void killrvnamed()
{
    int fd;
    struct sockaddr_un su;
    struct rvn rvnpkt;

    fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    su.sun_family = AF_UNIX;
    strcpy(su.sun_path, IPTSOCKNAME);

    rvnpkt.type = RVN_QUIT;

    sendto(fd, &rvnpkt, sizeof(struct rvn), 0, (struct sockaddr *) &su,
           sizeof(su.sun_family) + strlen(su.sun_path));

    close(fd);
}

void open_rvn_socket(int *fd)
{
    struct sockaddr_un su;

    strncpy(revname_socket, get_path(T_WORKDIR, gen_unix_sockname()), 80);
    unlink(revname_socket);

    *fd = socket(PF_UNIX, SOCK_DGRAM, 0);
    su.sun_family = AF_UNIX;
    strcpy(su.sun_path, revname_socket);
    bind(*fd, (struct sockaddr *) &su,
         sizeof(su.sun_family) + strlen(su.sun_path));
}

void close_rvn_socket(int fd)
{
    if (fd > 0) {
        close(fd);
        unlink(revname_socket);
    }
}

int revname(int *lookup, struct in_addr *saddr, char *target, int rvnfd)
{
    struct hostent *he;
    struct rvn rpkt;
    int br;
    struct sockaddr_un su;
    int fl;
    fd_set sockset;
    struct timeval tv;
    int sstat = 0;

    bzero(target, 45);
    if (*lookup) {
        if (rvnfd > 0) {
            su.sun_family = AF_UNIX;
            strcpy(su.sun_path, IPTSOCKNAME);

            rpkt.type = RVN_REQUEST;
            rpkt.saddr.s_addr = saddr->s_addr;

            sendto(rvnfd, &rpkt, sizeof(struct rvn), 0,
                   (struct sockaddr *) &su,
                   sizeof(su.sun_family) + strlen(su.sun_path));

            fl = sizeof(su.sun_family) + strlen(su.sun_path);
            do {
                tv.tv_sec = 10;
                tv.tv_usec = 0;

                FD_ZERO(&sockset);
                FD_SET(rvnfd, &sockset);

                do {
                    sstat = select(rvnfd + 1, &sockset, NULL, NULL, &tv);
                } while ((sstat < 0) && (errno == EINTR));

                if (FD_ISSET(rvnfd, &sockset))
                    br = recvfrom(rvnfd, &rpkt, sizeof(struct rvn), 0,
                                  (struct sockaddr *) &su, &fl);
                else
                    br = -1;
            } while ((br < 0) && (errno == EINTR));

            if (br < 0) {
                strcpy(target, inet_ntoa(*saddr));
                printipcerr();
                *lookup = 0;
                return RESOLVED;
            }
            strncpy(target, rpkt.fqdn, 44);
            return (rpkt.ready);
        } else {
            he = gethostbyaddr((char *) saddr,
                               sizeof(struct in_addr), AF_INET);

            if (he == NULL)
                strcpy(target, inet_ntoa(*saddr));
            else
                strncpy(target, he->h_name, 44);

            return RESOLVED;
        }
    } else {
        strcpy(target, inet_ntoa(*saddr));
        return RESOLVED;
    }
}
