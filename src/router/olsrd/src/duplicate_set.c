/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2008 Henning Rogge <rogge@fgan.de>
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

#include "duplicate_set.h"
#include "ipcalc.h"
#include "common/avl.h"
#include "olsr.h"
#include "mid_set.h"
#include "scheduler.h"
#include "mantissa.h"

static void olsr_cleanup_duplicate_entry(void *unused);

struct avl_tree duplicate_set;
struct timer_entry *duplicate_cleanup_timer;


void olsr_init_duplicate_set(void) {
  avl_init(&duplicate_set, olsr_cnf->ip_version == AF_INET ? &avl_comp_ipv4 : &avl_comp_ipv6);

  olsr_set_timer(&duplicate_cleanup_timer, DUPLICATE_CLEANUP_INTERVAL,
     DUPLICATE_CLEANUP_JITTER, OLSR_TIMER_PERIODIC,
     &olsr_cleanup_duplicate_entry, NULL, 0);
}

struct dup_entry *olsr_create_duplicate_entry(void *ip, olsr_u16_t seqnr) {
  struct dup_entry *entry;
  entry = olsr_malloc(sizeof(struct dup_entry), "New duplicate entry");
  if (entry != NULL) {
    memcpy (&entry->ip, ip, olsr_cnf->ip_version == AF_INET ? sizeof(entry->ip.v4) : sizeof(entry->ip.v6));
    entry->seqnr = seqnr;
    entry->too_low_counter = 0;
    entry->avl.key = &entry->ip;
  }
  return entry;
}

static void olsr_cleanup_duplicate_entry(void __attribute__ ((unused)) *unused) {
  struct dup_entry *entry;

  OLSR_FOR_ALL_DUP_ENTRIES(entry) {
    if (TIMED_OUT(entry->valid_until)) {
      avl_delete(&duplicate_set, &entry->avl);
      free (entry);
    }
  } OLSR_FOR_ALL_DUP_ENTRIES_END(entry);
}

int olsr_shall_process_message(void *ip, olsr_u16_t seqnr) {
  struct dup_entry *entry;
  int diff;
  void *mainIp;
  clock_t valid_until;

  struct ipaddr_str buf;
  // get main address
  mainIp = mid_lookup_main_addr(ip);
  if (mainIp == NULL) {
    mainIp = ip;
  }

  valid_until = GET_TIMESTAMP(DUPLICATE_VTIME);

  entry = (struct dup_entry *)avl_find(&duplicate_set, ip);
  if (entry == NULL) {
    entry = olsr_create_duplicate_entry(ip, seqnr);
    if (entry != NULL) {
      avl_insert(&duplicate_set, &entry->avl, 0);
      entry->valid_until = valid_until;
    }
    return 1; // okay, we process this package
  }

  diff = (int)seqnr - (int)(entry->seqnr);

  // update timestamp
  if (valid_until > entry->valid_until) {
    entry->valid_until = valid_until;
  }

  // overflow ?
  if (diff > (1<<15)) {
    diff -= (1<<16);
  }

  if (diff < -31) {
    entry->too_low_counter ++;

    // client did restart with a lower number ?
    if (entry->too_low_counter > 16) {
      entry->too_low_counter = 0;
      entry->seqnr = seqnr;
      entry->array = 1;
      return 1;
    }
    OLSR_PRINTF(9, "blocked %x from %s\n", seqnr, olsr_ip_to_string(&buf, mainIp));
    return 0;
  }

  entry->too_low_counter = 0;
  if (diff <= 0) {
    olsr_u32_t bitmask = 1 << ((olsr_u32_t) (-diff));

    if ((entry->array & bitmask) != 0) {
      OLSR_PRINTF(9, "blocked %x (diff=%d,mask=%08x) from %s\n", seqnr, diff, entry->array, olsr_ip_to_string(&buf, mainIp));
      return 0;
    }
    entry->array |= bitmask;
    OLSR_PRINTF(9, "processed %x from %s\n", seqnr, olsr_ip_to_string(&buf, mainIp));
    return 1;
  }
  else if (diff < 32) {
    entry->array <<= (olsr_u32_t)diff;
  }
  else {
    entry->array = 0;
  }
  entry->array |= 1;
  entry->seqnr = seqnr;
  OLSR_PRINTF(9, "processed %x from %s\n", seqnr, olsr_ip_to_string(&buf, mainIp));
  return 1;
}

void olsr_print_duplicate_table(void) {
#ifndef NODEBUG
  /* The whole function makes no sense without it. */
  struct dup_entry *entry;
  const int ipwidth = olsr_cnf->ip_version == AF_INET ? 15 : 30;
  struct ipaddr_str addrbuf;

  OLSR_PRINTF(1,
        "\n--- %s ------------------------------------------------- DUPLICATE SET\n\n"
        "%-*s %8s %s\n", olsr_wallclock_string(), ipwidth,
        "Node IP", "DupArray", "VTime");

  OLSR_FOR_ALL_DUP_ENTRIES(entry) {
    OLSR_PRINTF(1, "%-*s %08x %s\n",
        ipwidth, olsr_ip_to_string(&addrbuf, (union olsr_ip_addr *)(entry->avl.key)),
        entry->array,
        olsr_clock_string(entry->valid_until));
  } OLSR_FOR_ALL_DUP_ENTRIES_END(entry);
#endif
}
