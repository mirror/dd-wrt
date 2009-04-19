
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "scheduler.h"
#include "net_olsr.h"
#include "tc_set.h"

struct hna_entry hna_set[HASHSIZE];
struct olsr_cookie_info *hna_net_timer_cookie = NULL;
struct olsr_cookie_info *hna_entry_mem_cookie = NULL;
struct olsr_cookie_info *hna_net_mem_cookie = NULL;

/**
 * Initialize the HNA set
 */
int
olsr_init_hna_set(void)
{
  int idx;

  for (idx = 0; idx < HASHSIZE; idx++) {
    hna_set[idx].next = &hna_set[idx];
    hna_set[idx].prev = &hna_set[idx];
  }

  hna_net_timer_cookie = olsr_alloc_cookie("HNA Network", OLSR_COOKIE_TYPE_TIMER);

  hna_net_mem_cookie = olsr_alloc_cookie("hna_net", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(hna_net_mem_cookie, sizeof(struct hna_net));

  hna_entry_mem_cookie = olsr_alloc_cookie("hna_entry", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(hna_entry_mem_cookie, sizeof(struct hna_entry));

  return 1;
}

/**
 * Lookup a network entry in a networkentry list.
 *
 * @param nets the network list to look in
 * @param net the network to look for
 * @param mask the netmask to look for
 *
 * @return the localized entry or NULL of not found
 */
struct hna_net *
olsr_lookup_hna_net(const struct hna_net *nets, const union olsr_ip_addr *net, uint8_t prefixlen)
{
  struct hna_net *tmp;

  /* Loop trough entrys */
  for (tmp = nets->next; tmp != nets; tmp = tmp->next) {
    if (tmp->prefixlen == prefixlen && ipequal(&tmp->A_network_addr, net)) {
      return tmp;
    }
  }

  /* Not found */
  return NULL;
}

/**
 * Lookup a gateway entry
 *
 * @param gw the address of the gateway
 * @return the located entry or NULL if not found
 */
struct hna_entry *
olsr_lookup_hna_gw(const union olsr_ip_addr *gw)
{
  struct hna_entry *tmp_hna;
  uint32_t hash = olsr_ip_hashing(gw);

#if 0
  OLSR_PRINTF(5, "HNA: lookup entry\n");
#endif
  /* Check for registered entry */

  for (tmp_hna = hna_set[hash].next; tmp_hna != &hna_set[hash]; tmp_hna = tmp_hna->next) {
    if (ipequal(&tmp_hna->A_gateway_addr, gw)) {
      return tmp_hna;
    }
  }

  /* Not found */
  return NULL;
}

/**
 *Add a gatewayentry to the HNA set
 *
 *@param addr the address of the gateway
 *
 *@return the created entry
 */
struct hna_entry *
olsr_add_hna_entry(const union olsr_ip_addr *addr)
{
  struct hna_entry *new_entry;
  uint32_t hash;

  new_entry = olsr_cookie_malloc(hna_entry_mem_cookie);

  /* Fill struct */
  new_entry->A_gateway_addr = *addr;

  /* Link nets */
  new_entry->networks.next = &new_entry->networks;
  new_entry->networks.prev = &new_entry->networks;

  /* queue */
  hash = olsr_ip_hashing(addr);

  hna_set[hash].next->prev = new_entry;
  new_entry->next = hna_set[hash].next;
  hna_set[hash].next = new_entry;
  new_entry->prev = &hna_set[hash];

  return new_entry;
}

/**
 * Adds a network entry to a HNA gateway.
 *
 * @param hna_gw the gateway entry to add the network to
 * @param net the networkaddress to add
 * @param mask the netmask
 *
 * @return the newly created entry
 */
struct hna_net *
olsr_add_hna_net(struct hna_entry *hna_gw, const union olsr_ip_addr *net, uint8_t prefixlen)
{
  /* Add the net */
  struct hna_net *new_net = olsr_cookie_malloc(hna_net_mem_cookie);

  /* Fill struct */
  memset(new_net, 0, sizeof(struct hna_net));
  new_net->A_network_addr = *net;
  new_net->prefixlen = prefixlen;

  /* Set backpointer */
  new_net->hna_gw = hna_gw;

  /* Queue */
  hna_gw->networks.next->prev = new_net;
  new_net->next = hna_gw->networks.next;
  hna_gw->networks.next = new_net;
  new_net->prev = &hna_gw->networks;

  return new_net;
}

/**
 * Callback for the hna_net timer.
 */
static void
olsr_expire_hna_net_entry(void *context)
{
#ifdef DEBUG
  struct ipaddr_str buf1, buf2;
#endif
  struct hna_net *net_to_delete;
  struct hna_entry *hna_gw;

  net_to_delete = (struct hna_net *)context;
  net_to_delete->hna_net_timer = NULL;  /* be pedandic */
  hna_gw = net_to_delete->hna_gw;

#ifdef DEBUG
  OLSR_PRINTF(5, "HNA: timeout %s/%u via hna-gw %s\n", olsr_ip_to_string(&buf1, &net_to_delete->A_network_addr),
              net_to_delete->prefixlen, olsr_ip_to_string(&buf2, &hna_gw->A_gateway_addr));
#endif

  /*
   * Delete the rt_path for the entry.
   */
  olsr_delete_routing_table(&net_to_delete->A_network_addr, net_to_delete->prefixlen, &hna_gw->A_gateway_addr);

  /* Delete hna_gw if empty */
  if (hna_gw->networks.next == &hna_gw->networks) {
    DEQUEUE_ELEM(hna_gw);
    olsr_cookie_free(hna_entry_mem_cookie, hna_gw);
  }

  DEQUEUE_ELEM(net_to_delete);
  olsr_cookie_free(hna_net_mem_cookie, net_to_delete);
}

/**
 * Update a HNA entry. If it does not exist it
 * is created.
 * This is the only function that should be called
 * from outside concerning creation of HNA entries.
 *
 *@param gw address of the gateway
 *@param net address of the network
 *@param mask the netmask
 *@param vtime the validitytime of the entry
 *
 *@return nada
 */
void
olsr_update_hna_entry(const union olsr_ip_addr *gw, const union olsr_ip_addr *net, uint8_t prefixlen, olsr_reltime vtime)
{
  struct hna_entry *gw_entry;
  struct hna_net *net_entry;

  gw_entry = olsr_lookup_hna_gw(gw);
  if (!gw_entry) {

    /* Need to add the entry */
    gw_entry = olsr_add_hna_entry(gw);
  }

  net_entry = olsr_lookup_hna_net(&gw_entry->networks, net, prefixlen);
  if (net_entry == NULL) {

    /* Need to add the net */
    net_entry = olsr_add_hna_net(gw_entry, net, prefixlen);
    changes_hna = true;
  }

  /*
   * Add the rt_path for the entry.
   */
  olsr_insert_routing_table(&net_entry->A_network_addr, net_entry->prefixlen, &gw_entry->A_gateway_addr, OLSR_RT_ORIGIN_HNA);

  /*
   * Start, or refresh the timer, whatever is appropriate.
   */
  olsr_set_timer(&net_entry->hna_net_timer, vtime, OLSR_HNA_NET_JITTER, OLSR_TIMER_ONESHOT, &olsr_expire_hna_net_entry, net_entry,
                 hna_net_timer_cookie->ci_id);
}

/**
 * Print all HNA entries.
 *
 *@return nada
 */
void
olsr_print_hna_set(void)
{
#ifdef NODEBUG
  /* The whole function doesn't do anything else. */
  int idx;

  OLSR_PRINTF(1, "\n--- %02d:%02d:%02d.%02d ------------------------------------------------- HNA SET\n\n", nowtm->tm_hour,
              nowtm->tm_min, nowtm->tm_sec, (int)now.tv_usec / 10000);

  if (olsr_cnf->ip_version == AF_INET)
    OLSR_PRINTF(1, "IP net          netmask         GW IP\n");
  else
    OLSR_PRINTF(1, "IP net/prefixlen               GW IP\n");

  for (idx = 0; idx < HASHSIZE; idx++) {
    struct hna_entry *tmp_hna = hna_set[idx].next;
    /* Check all entrys */
    while (tmp_hna != &hna_set[idx]) {
      /* Check all networks */
      struct hna_net *tmp_net = tmp_hna->networks.next;

      while (tmp_net != &tmp_hna->networks) {
        if (olsr_cnf->ip_version == AF_INET) {
          struct ipaddr_str buf;
          OLSR_PRINTF(1, "%-15s ", olsr_ip_to_string(&buf, &tmp_net->A_network_addr));
          OLSR_PRINTF(1, "%-15d ", tmp_net->prefix_len);
          OLSR_PRINTF(1, "%-15s\n", olsr_ip_to_string(&buf, &tmp_hna->A_gateway_addr));
        } else {
          struct ipaddr_str buf;
          OLSR_PRINTF(1, "%-27s/%d", olsr_ip_to_string(&buf, &tmp_net->A_network_addr), tmp_net->A_netmask.v6);
          OLSR_PRINTF(1, "%s\n", olsr_ip_to_string(&buf, &tmp_hna->A_gateway_addr));
        }

        tmp_net = tmp_net->next;
      }
      tmp_hna = tmp_hna->next;
    }
  }
#endif
}

/**
 *Process incoming HNA message.
 *Forwards the message if that is to be done.
 *
 *@param m the incoming OLSR message
 *the OLSR message.
 *@return 1 on success
 */

bool
olsr_input_hna(union olsr_message *m, struct interface *in_if __attribute__ ((unused)), union olsr_ip_addr *from_addr)
{

  uint8_t olsr_msgtype;
  olsr_reltime vtime;
  uint16_t olsr_msgsize;
  union olsr_ip_addr originator;
  uint8_t hop_count;
  uint16_t packet_seq_number;

  int hnasize;
  const uint8_t *curr, *curr_end;

#ifdef DEBUG
  OLSR_PRINTF(5, "Processing HNA\n");
#endif

  /* Check if everyting is ok */
  if (!m) {
    return false;
  }
  curr = (const uint8_t *)m;

  /* olsr_msgtype */
  pkt_get_u8(&curr, &olsr_msgtype);
  if (olsr_msgtype != HNA_MESSAGE) {
    OLSR_PRINTF(0, "not a HNA message!\n");
    return false;
  }
  /* Get vtime */
  pkt_get_reltime(&curr, &vtime);

  /* olsr_msgsize */
  pkt_get_u16(&curr, &olsr_msgsize);
  hnasize =
    olsr_msgsize - (olsr_cnf->ip_version == AF_INET ? offsetof(struct olsrmsg, message) : offsetof(struct olsrmsg6, message));
  if (hnasize < 0) {
    OLSR_PRINTF(0, "message size %d too small (at least %lu)!\n", olsr_msgsize,
                (unsigned long)(olsr_cnf->ip_version ==
                                AF_INET ? offsetof(struct olsrmsg, message) : offsetof(struct olsrmsg6, message)));
    return false;
  }
  if ((hnasize % (2 * olsr_cnf->ipsize)) != 0) {
    OLSR_PRINTF(0, "Illegal message size %d!\n", olsr_msgsize);
    return false;
  }
  curr_end = (const uint8_t *)m + olsr_msgsize;

  /* validate originator */
  pkt_get_ipaddress(&curr, &originator);
  /*printf("HNA from %s\n\n", olsr_ip_to_string(&buf, &originator)); */

  /* ttl */
  pkt_ignore_u8(&curr);

  /* hopcnt */
  pkt_get_u8(&curr, &hop_count);

  /* seqno */
  pkt_get_u16(&curr, &packet_seq_number);

  /*
   *      If the sender interface (NB: not originator) of this message
   *      is not in the symmetric 1-hop neighborhood of this node, the
   *      message MUST be discarded.
   */
  if (check_neighbor_link(from_addr) != SYM_LINK) {
    struct ipaddr_str buf;
    OLSR_PRINTF(2, "Received HNA from NON SYM neighbor %s\n", olsr_ip_to_string(&buf, from_addr));
    return false;
  }
#if 1
  while (curr < curr_end) {
    union olsr_ip_addr net;
    uint8_t prefixlen;
    struct ip_prefix_list *entry;

    pkt_get_ipaddress(&curr, &net);
    pkt_get_prefixlen(&curr, &prefixlen);
    entry = ip_prefix_list_find(olsr_cnf->hna_entries, &net, prefixlen);
    if (entry == NULL) {
      /* only update if it's not from us */
      olsr_update_hna_entry(&originator, &net, prefixlen, vtime);
    }
  }
#else
  while (hna_tmp) {
    /* Don't add an HNA entry that we are advertising ourselves. */
    if (!ip_prefix_list_find(olsr_cnf->hna_entries, &hna_tmp->net, hna_tmp->prefixlen)) {
      olsr_update_hna_entry(&message.originator, &hna_tmp->net, hna_tmp->prefixlen, message.vtime);
    }
  }
#endif
  /* Forward the message */
  return true;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
