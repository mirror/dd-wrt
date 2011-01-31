/* address.c - functions to query ESI and local ATM address from kernel */

/*
 * Marko Kiiskila carnil@cs.tut.fi 
 * 
 * Copyright (c) 1996
 * Tampere University of Technology - Telecommunications Laboratory
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * TUT ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 * 
 */

/*  Copyright (C) 1999  Heikki Vatiainen hessu@cs.tut.fi */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include <atm.h>
#include <linux/atmdev.h>
#include <atmd.h>

#include "address.h"

#define COMPONENT "address.c"

/* Gets End System Identifier (MAC address) from kernel
 * Returns < 0 for error
 */
int addr_getesi(unsigned char *mac_addr, int phys_itf)
{
        int fd, retval;
        struct atmif_sioc req;
        
        fd = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if (fd < 0) {
                diag(COMPONENT, DIAG_ERROR, "addr_getesi: socket: %s",
                     strerror(errno));
                return -1;
        }
        req.number = phys_itf;
        req.arg = mac_addr;
        req.length = ESI_LEN;
        retval = ioctl(fd, ATM_GETESI, &req);
        if (retval < 0) diag(COMPONENT, DIAG_ERROR, "ioctl ATM_GETESI: %s",
                             strerror(errno));
        close(fd);

        return retval;
}

/* Gets one of our ATM addresses from kernel. Useful for binding listen sockets.
 * Returns < 0 for error
 */
#define MAX_LOCAL_ADDRS 32
int get_listenaddr(unsigned char *atm_addr, int phys_itf)
{
        int fd, retval;
        struct atmif_sioc req;
        struct sockaddr_atmsvc listen_addr[MAX_LOCAL_ADDRS];
        
        fd = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if (fd < 0) {
                diag(COMPONENT, DIAG_ERROR, "get_listenaddr: socket: %s",
                     strerror(errno));
                return -1;
        }
        req.number = phys_itf;
        req.arg = listen_addr;
        req.length = sizeof(listen_addr);
        retval = ioctl(fd, ATM_GETADDR, &req);
        if (retval < 0) diag(COMPONENT, DIAG_ERROR, "ioctl ATM_GETADDR: %s",
                             strerror(errno));
        close(fd);

        memcpy(atm_addr, listen_addr[0].sas_addr.prv, ATM_ESA_LEN);

        return retval;
}
