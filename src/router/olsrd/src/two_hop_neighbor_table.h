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

#ifndef _OLSR_TWO_HOP_TABLE
#define _OLSR_TWO_HOP_TABLE

#include "defs.h"
#include "hashing.h"
#include "lq_plugin.h"
#include "olsr_types.h"

#define	NB2S_COVERED 	0x1     /* node has been covered by a MPR */

struct neighbor_list_entry {
  struct neighbor_entry *neighbor;
  olsr_linkcost second_hop_linkcost;
  olsr_linkcost path_linkcost;
  olsr_linkcost saved_path_linkcost;
  struct neighbor_list_entry *next;
  struct neighbor_list_entry *prev;
};

struct neighbor_2_entry {
  union olsr_ip_addr neighbor_2_addr;
  uint8_t mpr_covered_count;           /*used in mpr calculation */
  uint8_t processed;                   /*used in mpr calculation */
  int16_t neighbor_2_pointer;          /* Neighbor count */
  struct neighbor_list_entry neighbor_2_nblist;
  struct neighbor_2_entry *prev;
  struct neighbor_2_entry *next;
};

extern struct neighbor_2_entry two_hop_neighbortable[HASHSIZE];

void olsr_init_two_hop_table(void);

void olsr_delete_neighbor_pointer(struct neighbor_2_entry *, struct neighbor_entry *);

void olsr_delete_two_hop_neighbor_table(struct neighbor_2_entry *);

void olsr_insert_two_hop_neighbor_table(struct neighbor_2_entry *);

struct neighbor_2_entry *olsr_lookup_two_hop_neighbor_table(const union olsr_ip_addr *);

struct neighbor_2_entry *olsr_lookup_two_hop_neighbor_table_mid(const union olsr_ip_addr *);

#ifndef NODEBUG
void olsr_print_two_hop_neighbor_table(void);
#else
#define olsr_print_two_hop_neighbor_table() do { } while(0)
#endif

#endif /* _OLSR_TWO_HOP_TABLE */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
