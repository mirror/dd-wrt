/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
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
get_packet(int idx)
{
  int i = 0;
  struct packnode *tmp;

  if (idx > MAXPACKS)
    return 0;

  if (idx == 0)
    return packets->packet;

  tmp = packets;

  while (i != idx) {
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
