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

#ifndef _OLSR_HNA
#define _OLSR_HNA

#include "hashing.h"
#include "olsr_types.h"
#include "olsr_protocol.h"
#include "mantissa.h"

#include <time.h>

/* hna_netmask declared in packet.h */

struct hna_net {
  struct olsr_ip_prefix hna_prefix;
  struct timer_entry *hna_net_timer;
  struct hna_entry *hna_gw;            /* backpointer to the owning HNA entry */
  struct hna_net *next;
  struct hna_net *prev;
};

#define OLSR_HNA_NET_JITTER 5   /* percent */

struct hna_entry {
  union olsr_ip_addr A_gateway_addr;
  struct hna_net networks;
  struct hna_entry *next;
  struct hna_entry *prev;
};

#define OLSR_FOR_ALL_HNA_ENTRIES(hna) \
{ \
  int _idx; \
  for (_idx = 0; _idx < HASHSIZE; _idx++) { \
    struct hna_entry *_next; \
    for(hna = hna_set[_idx].next; \
        hna != &hna_set[_idx]; \
        hna = _next) { \
      _next = hna->next;
#define OLSR_FOR_ALL_HNA_ENTRIES_END(hna) }}}

extern struct hna_entry hna_set[HASHSIZE];

int olsr_init_hna_set(void);
void olsr_cleanup_hna(union olsr_ip_addr *orig);

struct hna_net *olsr_lookup_hna_net(const struct hna_net *, const union olsr_ip_addr *, uint8_t);

struct hna_entry *olsr_lookup_hna_gw(const union olsr_ip_addr *);

struct hna_entry *olsr_add_hna_entry(const union olsr_ip_addr *);

struct hna_net *olsr_add_hna_net(struct hna_entry *, const union olsr_ip_addr *, uint8_t);

void olsr_update_hna_entry(const union olsr_ip_addr *, const union olsr_ip_addr *, uint8_t, olsr_reltime);

#ifndef NODEBUG
void olsr_print_hna_set(void);
#else
#define olsr_print_hna_set() do { } while(0)
#endif

bool olsr_input_hna(union olsr_message *, struct interface_olsr *, union olsr_ip_addr *);

#endif /* _OLSR_HNA */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
