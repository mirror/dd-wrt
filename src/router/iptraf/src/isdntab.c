/***

isdntab.c	- a set of simple routines that collect detected ISDN
		  interfaces and record their link encapsulation.
		  
Copyright (c) Gerard Paul Java, 1998

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
#include <string.h>
#include <sys/ioctl.h>
#include <linux/isdn.h>
#include "isdntab.h"


void add_isdn_entry(struct isdntab *list, char *ifname, int isdn_fd)
{
    struct isdntabent *new_entry;
    isdn_net_ioctl_cfg isdn_cfg;

    new_entry = malloc(sizeof(struct isdntabent));

    strcpy(new_entry->ifname, ifname);
    new_entry->next_entry = NULL;

    if (list->head == NULL)
        list->head = new_entry;

    if (list->tail != NULL)
        list->tail->next_entry = new_entry;

    list->tail = new_entry;

    strcpy(isdn_cfg.name, ifname);
    ioctl(isdn_fd, IIOCNETGCF, &isdn_cfg);
    new_entry->encap = isdn_cfg.p_encap;
}

struct isdntabent *isdn_table_lookup(struct isdntab *list, char *ifname,
                                     int isdn_fd)
{
    struct isdntabent *ptmp = list->head;

    while (ptmp != NULL) {
        if (strcmp(ptmp->ifname, ifname) == 0)
            break;

        ptmp = ptmp->next_entry;
    }

    if (ptmp == NULL) {
        add_isdn_entry(list, ifname, isdn_fd);
        ptmp = list->tail;
    }
    return ptmp;
}

void destroy_isdn_table(struct isdntab *list)
{
    struct isdntabent *ptmp = list->head;
    struct isdntabent *ctemp = NULL;

    if (ptmp != NULL)
        ctemp = ptmp->next_entry;

    while (ptmp != NULL) {
        free(ptmp);

        ptmp = ctemp;

        if (ctemp != NULL)
            ctemp = ctemp->next_entry;
    }
}
