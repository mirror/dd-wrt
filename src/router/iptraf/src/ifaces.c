/***

ifaces.c - routine that determines whether a given interface is supported
		by IPTraf
		
Copyright (c) Gerard Paul Java 1998, 2003

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <fcntl.h>
#include <string.h>
#include <linux/if_packet.h>
#include "links.h"
#include "error.h"

extern int accept_unsupported_interfaces;
#define NUM_SUPPORTED_IFACES 26

extern int daemonized;

char ifaces[][6] =
    { "lo", "eth", "sl", "ppp", "ippp", "plip", "fddi", "isdn", "dvb",
    "pvc", "hdlc", "ipsec", "sbni", "tr", "wvlan", "wlan", "sm2", "sm3",
    "pent", "lec", "brg", "tun", "tap", "cipcb", "tunl", "vlan"
};

char *ltrim(char *buf)
{
    char *tmp = buf;

    while ((*tmp == ' ') || (*tmp == '\t'))
        tmp++;

    strcpy(buf, tmp);
    return buf;
}

/*
 * Open /proc/net/dev and move file pointer past the two table header lines
 * at the top of the file.
 */

FILE *open_procnetdev(void)
{
    FILE *fd;
    char buf[161];

    fd = fopen("/proc/net/dev", "r");

    /*
     * Read and discard the table header lines in the file
     */

    if (fd != NULL) {
        fgets(buf, 160, fd);
        fgets(buf, 160, fd);
    }

    return fd;
}

/*
 * Get the next interface from /proc/net/dev.
 */
void get_next_iface(FILE * fd, char *ifname)
{
    char buf[161];

    if (!feof(fd)) {
        strcpy(buf, "");
        fgets(buf, 160, fd);
        if (strcmp(buf, "") != 0)
            strcpy(ifname, ltrim(strtok(buf, ":")));
        else
            strcpy(ifname, "");
    } else
        strcpy(ifname, "");
}

/*
 * Determine if supplied interface is supported.
 */

int iface_supported(char *iface)
{
    int i;

    if (accept_unsupported_interfaces)
        return 1;

    for (i = 0; i <= NUM_SUPPORTED_IFACES - 1; i++) {
        if (strncmp(ifaces[i], iface, strlen(ifaces[i])) == 0)
            return 1;
    }

    return 0;
}

int iface_up(char *iface)
{
    int fd;
    int ir;
    struct ifreq ifr;

    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    strcpy(ifr.ifr_name, iface);
    ir = ioctl(fd, SIOCGIFFLAGS, &ifr);

    close(fd);

    if ((ir != 0) || (!(ifr.ifr_flags & IFF_UP)))
        return 0;

    return 1;
}

void err_iface_unsupported(void)
{
    write_error("Specified interface not supported", daemonized);
}

void err_iface_down(void)
{
    write_error("Specified interface not active", daemonized);
}

void isdn_iface_check(int *fd, char *ifname)
{
    if (*fd == -1) {
        if (strncmp(ifname, "isdn", 4) == 0)
            *fd = open("/dev/isdnctrl", O_RDWR);
    }
}

char *gen_iface_msg(char *ifptr)
{
    static char if_msg[20];

    if (ifptr == NULL)
        strcpy(if_msg, "all interfaces");
    else
        strncpy(if_msg, ifptr, 20);

    return if_msg;
}
