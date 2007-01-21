/* ----------------------------------------------------------------------------
    NSTX -- tunneling network-packets over DNS

     (C) 2000 by Florian Heinz and Julien Oster

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "nstxfun.h"
#include "nstx_pstack.h"

static struct nstx_item * get_item_by_id(unsigned int id);
static struct nstx_item * alloc_item(unsigned int id);
static char * dealloc_item(struct nstx_item *ptr, int *l);
static int add_data(struct nstx_item *item, struct nstxhdr *pkt, int datalen);
static void check_timeouts(void);

static struct nstx_item *nstx_list = NULL;

void
nstx_handlepacket(const char *ptr, size_t len,
    void (*nstx_usepacket)(const char*, size_t))
{
   struct nstxhdr *nstxpkt = (struct nstxhdr *) ptr;
   struct nstx_item *nstxitem;
   char *netpacket;
   int netpacketlen;
   
   if ((!ptr) || (signed int) len < (signed int) sizeof(struct nstxhdr))
     return;

   if (!nstxpkt->id)
     return;
   
   nstxitem = get_item_by_id(nstxpkt->id);
   
   if (!nstxitem)
     nstxitem = alloc_item(nstxpkt->id);
   
   if (add_data(nstxitem, nstxpkt, len)) {
      netpacket = dealloc_item(nstxitem, &netpacketlen);
      nstx_usepacket(netpacket, netpacketlen);
   }
   check_timeouts();
}

static struct nstx_item * get_item_by_id(unsigned int id) {
   struct nstx_item *ptr = nstx_list;
   
   if (!ptr)
     return NULL;
   
   while (ptr) {
      if (ptr->id == id)
	return ptr;
      ptr = ptr->next;
   }
   return NULL;
}

static struct nstx_item * alloc_item(unsigned int id) {
   struct nstx_item *ptr;
   
   ptr = malloc(sizeof(struct nstx_item));
   memset(ptr, 0, sizeof(struct nstx_item));
   ptr->next = nstx_list;
   if (ptr->next)
     ptr->next->prev = ptr;
   nstx_list = ptr;
   
   ptr->id = id;
   return ptr;
}

static char * dealloc_item(struct nstx_item *ptr, int *l) {
   static char *data = NULL;
   int len = 0;
   struct clist *c, *tmp;
   
   if (ptr->prev)
     ptr->prev->next = ptr->next;
   else
     nstx_list = ptr->next;
   if (ptr->next)
     ptr->next->prev = ptr->prev;
   
   c = ptr->chunks;
   while (c) {
      data = realloc(data, len+c->len);
      memcpy(data+len, c->data, c->len);
      len += c->len;
      free(c->data);
      tmp = c->next;
      free(c);
      c = tmp;
   }
   free(ptr);
   
   if (l)
     *l = len;
   return data;
}

static void add_data_chunk(struct nstx_item *item, int seq,
			   char *data, int len) {
   struct clist *next, *prev, *ptr;
   
   prev = next = NULL;
   if (!item->chunks)
     ptr = item->chunks = malloc(sizeof(struct clist));
   else if (item->chunks->seq == seq)
     return;
   else if (item->chunks->seq > seq) {
      next = item->chunks;
      ptr = item->chunks = malloc(sizeof(struct clist));
   } else {
      prev = item->chunks;
      while (prev->next && (prev->next->seq < seq))
	prev = prev->next;
      next = prev->next;
      if (next && (next->seq == seq))
	return;
      ptr = malloc(sizeof(struct clist));
   }
   memset(ptr, 0, sizeof(struct clist));
   if (prev)
     prev->next = ptr;
   ptr->next = next;
   ptr->seq = seq;
   ptr->len = len;
   ptr->data = malloc(len);
   memcpy(ptr->data, data, len);
}

static int find_sequence (struct nstx_item *item) {
   struct clist *list;
   int i;
   
   for (i = 0, list = item->chunks; list; i++, list = list->next)
     if (list->seq != i)
       break;
   
   return i;
}  

static int add_data(struct nstx_item *item, struct nstxhdr *pkt, int datalen) {
   char *payload;
   
   payload = ((char *) pkt) + sizeof(struct nstxhdr);
   item->timestamp = time(NULL);
   if (pkt->flags & NSTX_LF) {
      item->frc = pkt->seq+1;
   }
   
   add_data_chunk(item, pkt->seq, payload, datalen-sizeof(struct nstxhdr));
   if (item->frc && (find_sequence(item) == item->frc)) {
      return 1;
   }
   return 0;
}

static void check_timeouts(void) {
   unsigned int now;
   struct nstx_item *ptr = nstx_list, *ptr2;
   
   now = time(NULL);

   while (ptr) {
      ptr2 = ptr;
      ptr = ptr->next;
      if (now > (ptr2->timestamp + NSTX_TIMEOUT)) {
	 dealloc_item(ptr2, NULL);
      }
   }
}
