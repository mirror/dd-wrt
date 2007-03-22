/***

promisc.c	- handles the promiscuous mode flag for the Ethernet/FDDI/
              Token Ring interfaces
		  
Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997, 2002

This module contains functions that manage the promiscuous states of
the interfaces.

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <curses.h>
#include <panel.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include "ifstats.h"
#include "ifaces.h"
#include "error.h"
#include "promisc.h"
#include "dirs.h"

#define PROMISC_MSG_MAX 80

extern int daemonized;
extern int accept_unsupported_interfaces;

void init_promisc_list(struct promisc_states **list)
{
    FILE *fd;
    int ifd;
    char buf[8];
    struct promisc_states *ptmp;
    struct promisc_states *tail = NULL;
    struct ifreq ifr;
    int istat;
    char err_msg[80];

    ifd = socket(PF_INET, SOCK_DGRAM, 0);

    *list = NULL;
    fd = open_procnetdev();

    do {
        get_next_iface(fd, buf);

        if (strcmp(buf, "") != 0) {
            ptmp = malloc(sizeof(struct promisc_states));
            strcpy(ptmp->params.ifname, buf);

            if (*list == NULL) {
                *list = ptmp;
            } else
                tail->next_entry = ptmp;

            tail = ptmp;
            ptmp->next_entry = NULL;

            /*
             * Retrieve and save interface flags
             */

            if ((strncmp(buf, "eth", 3) == 0) ||
                (strncmp(buf, "fddi", 4) == 0) ||
                (strncmp(buf, "tr", 2) == 0) ||
                (strncmp(ptmp->params.ifname, "wvlan", 4) == 0) ||
                (strncmp(ptmp->params.ifname, "lec", 3) == 0) ||
                (accept_unsupported_interfaces)) {
                strcpy(ifr.ifr_name, buf);

                istat = ioctl(ifd, SIOCGIFFLAGS, &ifr);

                if (istat < 0) {
                    sprintf(err_msg,
                            "Unable to obtain interface parameters for %s",
                            buf);
                    write_error(err_msg, daemonized);
                    ptmp->params.state_valid = 0;
                } else {
                    ptmp->params.saved_state = ifr.ifr_flags;
                    ptmp->params.state_valid = 1;
                }
            }
        }
    } while (strcmp(buf, "") != 0);
}

/*
 * Save interfaces and their states to a temporary file.  Used only by the
 * first IPTraf instance.  Needed in case there are subsequent, simultaneous 
 * instances of IPTraf, which may still need promiscuous mode even after
 * the first instance exits.  These subsequent instances will need to restore
 * the promiscuous state from this file.
 */

void save_promisc_list(struct promisc_states *list)
{
    int fd;
    struct promisc_states *ptmp = list;

    fd = open(PROMISCLISTFILE, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        write_error("Unable to save interface flags", daemonized);
        return;
    }

    while (ptmp != NULL) {
        write(fd, &(ptmp->params), sizeof(struct promisc_params));
        ptmp = ptmp->next_entry;
    }

    close(fd);
}

/*
 * Load promiscuous states into list
 */

void load_promisc_list(struct promisc_states **list)
{
    int fd;
    struct promisc_states *ptmp = NULL;
    struct promisc_states *tail = NULL;
    int br;

    fd = open(PROMISCLISTFILE, O_RDONLY);

    if (fd < 0) {
        write_error("Unable to retrieve saved interface flags",
                    daemonized);
        *list = NULL;
        return;
    }

    do {
        ptmp = malloc(sizeof(struct promisc_states));
        br = read(fd, &(ptmp->params), sizeof(struct promisc_params));

        if (br > 0) {
            if (tail != NULL)
                tail->next_entry = ptmp;
            else
                *list = ptmp;

            ptmp->next_entry = NULL;
            tail = ptmp;
        } else
            free(ptmp);
    } while (br > 0);

    close(fd);
}

/*
 * Set/restore interface promiscuous mode.
 */

void srpromisc(int mode, struct promisc_states *list)
{
    int fd;
    struct ifreq ifr;
    struct promisc_states *ptmp;
    int istat;
    char fullmsg[PROMISC_MSG_MAX];

    ptmp = list;

    fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        write_error("Unable to open socket for flag change", daemonized);
        return;
    }

    while (ptmp != NULL) {
        if (((strncmp(ptmp->params.ifname, "eth", 3) == 0) ||
             (strncmp(ptmp->params.ifname, "fddi", 4) == 0) ||
             (strncmp(ptmp->params.ifname, "tr", 2) == 0) ||
             (strncmp(ptmp->params.ifname, "wvlan", 4) == 0) ||
             (strncmp(ptmp->params.ifname, "lec", 3) == 0)) &&
            (ptmp->params.state_valid)) {

            strcpy(ifr.ifr_name, ptmp->params.ifname);

            if (mode)
                ifr.ifr_flags = ptmp->params.saved_state | IFF_PROMISC;
            else
                ifr.ifr_flags = ptmp->params.saved_state;

            istat = ioctl(fd, SIOCSIFFLAGS, &ifr);

            if (istat < 0) {
                sprintf(fullmsg, "Promisc change failed for %s",
                        ptmp->params.ifname);
                write_error(fullmsg, daemonized);
            }
        }
        ptmp = ptmp->next_entry;
    }

    close(fd);
}

void destroy_promisc_list(struct promisc_states **list)
{
    struct promisc_states *ptmp = *list;
    struct promisc_states *ctmp;

    if (ptmp != NULL)
        ctmp = ptmp->next_entry;

    while (ptmp != NULL) {
        free(ptmp);
        ptmp = ctmp;
        if (ctmp != NULL)
            ctmp = ctmp->next_entry;
    }
}
