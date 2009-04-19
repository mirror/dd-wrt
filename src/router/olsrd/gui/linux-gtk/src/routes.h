
/*
 * OLSR ad-hoc routing table management protocol GUI front-end
 * Copyright (C) 2003 Andreas Tonnesen (andreto@ifi.uio.no)
 *
 * This file is part of olsr.org.
 *
 * uolsrGUI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * uolsrGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsr.org; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Route entries are registered
 * separatly from the node set
 *
 * This set is not used for anything in particular
 * intended for future use
 */

#ifndef _OLSRD_FRONTEND_ROUTE
#define _OLSRD_FRONTEND_ROUTE

#include "common.h"

#define MAX_IF_NAMESIZ  10

struct route_entry {
  union olsr_ip_addr gw;
  union olsr_ip_addr dst;
  olsr_u16_t hopcnt;
  char if_name[MAX_IF_NAMESIZ];
  struct route_entry *next;
  struct route_entry *prev;
};

/* List */
struct route_entry route_set;

/* Counter */
olsr_u32_t route_count;

/* Functions */

int init_route_entries();

int add_route_entry(struct route_entry *);

int del_route_entry(struct route_entry *);

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
