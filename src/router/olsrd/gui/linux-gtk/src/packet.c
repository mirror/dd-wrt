
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
 *Andreas Tonnesen (andreto@ifi.uio.no)
 *
 *The list of cached packets
 *
 */

#include "common.h"
#include "packet.h"

struct packnode *packets = NULL;
int no_packets = 0;

/*
 *Add the contets of a packet to
 *the cached packets
 */
int
add_packet_to_buffer(union olsr_message *p, int size)
{
  struct packnode *tmp;
  /* Should be the same for v4 and v6 */

  /* If this is the first packet */
  if (!packets) {
    //printf("Adding first packet\n");
    packets = malloc(sizeof(struct packnode));
    packets->packet = malloc(size);
    memcpy(packets->packet, p, size);
    packets->next = NULL;
    no_packets++;
    return 1;
  } else {
    /* If the buffer is not full */
    if (no_packets < MAXPACKS) {
      //printf("Buffer not filled yet..\n");
      tmp = packets;
      packets = malloc(sizeof(struct packnode));
      packets->packet = malloc(size);
      memcpy(packets->packet, p, size);
      packets->next = tmp;
      no_packets++;
      return 1;
    }
    /* If buffer is full */
    else {
      //printf("Buffer full - deleting...\n");
      tmp = packets;
      /* Find second last packet */
      while (tmp->next->next) {
        tmp = tmp->next;
      }
      /* Delete last packet */
      free(tmp->next->packet);
      free(tmp->next);
      tmp->next = NULL;

      /*Add the new packet */
      tmp = packets;
      packets = malloc(sizeof(struct packnode));
      packets->packet = malloc(size);
      memcpy(packets->packet, p, size);
      packets->next = tmp;
      return 1;
    }

  }
  return 0;
}

/*
 *Get the packet with index 'index'
 */
union olsr_message *
get_packet(int index)
{
  int i = 0;
  struct packnode *tmp;

  if (index > MAXPACKS)
    return 0;

  if (index == 0)
    return packets->packet;

  tmp = packets;

  while (i != index) {
    tmp = tmp->next;
    i++;
  }
  return tmp->packet;

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
