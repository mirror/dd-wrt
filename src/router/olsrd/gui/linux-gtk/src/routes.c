
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

#include "routes.h"

int
init_route_entries()
{
  route_count = 0;

  route_set.next = &route_set;
  route_set.prev = &route_set;

  return 1;
}

int
add_route_entry(struct route_entry *entry)
{
  struct route_entry *new_entry;

  if ((new_entry = malloc(sizeof(struct route_entry))) == 0) {
    fprintf(stderr, "OUT OF MEMORY!\n");
    exit(1);
  }

  /* queue */
  new_entry->next = route_set.next;
  route_set.next->prev = new_entry;
  route_set.next = new_entry;
  new_entry->prev = &route_set;

  route_count++;

  return 1;

}

int
del_route_entry(struct route_entry *entry)
{

  route_count--;

  return 1;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
