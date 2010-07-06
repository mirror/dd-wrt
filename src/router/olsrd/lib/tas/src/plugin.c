
/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * Copyright (c) 2004, Thomas Lopatic (thomas@olsr.org)
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

#include <string.h>
#include <time.h>               // clock_t required by olsrd includes

#include "link.h"
#include "plugin.h"
#include "lib.h"
#include "os_unix.h"
#include "http.h"
#include "glua.h"
#include "glua_ext.h"
#include "olsrd_plugin.h"
#include "net_olsr.h"

#include <defs.h>
#include <olsr.h>
#include <scheduler.h>
#include <parser.h>
#include <link_set.h>
#include <neighbor_table.h>
#include <two_hop_neighbor_table.h>
#include <mid_set.h>
#include <tc_set.h>
#include <hna_set.h>
#include <routing_table.h>
#include <olsr_protocol.h>
#include <mpr_selector_set.h>
#include <duplicate_set.h>
#include <lq_plugin.h>

#define PLUGIN_INTERFACE_VERSION 5

#define MESSAGE_TYPE 129

int olsrd_plugin_interface_version(void);
int olsrd_plugin_register_param(char *name, char *value);
int olsrd_plugin_init(void);

static int ipAddrLen;
static union olsr_ip_addr *mainAddr;

static struct interface *intTab = NULL;
static struct neighbor_entry *neighTab = NULL;
static struct mid_entry *midTab = NULL;
static struct hna_entry *hnaTab = NULL;
static struct olsrd_config *config = NULL;

static int iterIndex;
#if 0

/* not used */
static struct interface *iterIntTab = NULL;
static struct mid_entry *iterMidTab = NULL;
static struct hna_entry *iterHnaTab = NULL;
#endif

static struct link_entry *iterLinkTab = NULL;
static struct neighbor_entry *iterNeighTab = NULL;
static struct tc_entry *iterTcTab = NULL;
static struct rt_entry *iterRouteTab = NULL;

static void __attribute__ ((constructor)) banner(void)
{
  printf("Tiny Application Server 0.1 by olsr.org\n");
}

int
iterLinkTabNext(char *buff, int len)
{
  struct list_node *link_node;
  struct lqtextbuffer lqbuffer;

  if (iterLinkTab == NULL)
    return -1;

  snprintf(buff, len, "local~%s~remote~%s~main~%s~hysteresis~%f~cost~%s~",
           rawIpAddrToString(&iterLinkTab->local_iface_addr, ipAddrLen), rawIpAddrToString(&iterLinkTab->neighbor_iface_addr,
                                                                                           ipAddrLen),
           rawIpAddrToString(&iterLinkTab->neighbor->neighbor_main_addr, ipAddrLen), iterLinkTab->L_link_quality,
           get_linkcost_text(iterLinkTab->linkcost, false, &lqbuffer));

  link_node = iterLinkTab->link_list.next;
  if (link_node != &link_entry_head) {
    iterLinkTab = list2link(link_node);
  } else {
    iterLinkTab = NULL;
  }

  return 0;
}

void
iterLinkTabInit(void)
{
  struct list_node *link_node;

  link_node = link_entry_head.next;
  if (link_node != &link_entry_head) {
    iterLinkTab = list2link(link_node);
  } else {
    iterLinkTab = NULL;
  }
}

int
iterNeighTabNext(char *buff, int len)
{
  int res;
  int i;
  struct neighbor_2_list_entry *neigh2;

  if (iterNeighTab == NULL)
    return -1;

  res =
    snprintf(buff, len, "main~%s~symmetric~%s~mpr~%s~mprs~%s~willingness~%d~[~neighbors2~",
             rawIpAddrToString(&iterNeighTab->neighbor_main_addr, ipAddrLen), iterNeighTab->status == SYM ? "true" : "false",
             iterNeighTab->is_mpr != 0 ? "true" : "false",
             olsr_lookup_mprs_set(&iterNeighTab->neighbor_main_addr) != NULL ? "true" : "false", iterNeighTab->willingness);

  i = 0;

  len -= res;
  buff += res;

  len -= 2;

  for (neigh2 = iterNeighTab->neighbor_2_list.next; neigh2 != &iterNeighTab->neighbor_2_list; neigh2 = neigh2->next) {
    res = snprintf(buff, len, "%d~%s~", i, rawIpAddrToString(&neigh2->neighbor_2->neighbor_2_addr, ipAddrLen));

    if (res < len)
      buff += res;

    len -= res;

    if (len <= 0)
      break;

    i++;
  }

  strcpy(buff, "]~");

  iterNeighTab = iterNeighTab->next;

  if (iterNeighTab == &neighTab[iterIndex]) {
    iterNeighTab = NULL;

    while (++iterIndex < HASHSIZE)
      if (neighTab[iterIndex].next != &neighTab[iterIndex]) {
        iterNeighTab = neighTab[iterIndex].next;
        break;
      }
  }

  return 0;
}

void
iterNeighTabInit(void)
{
  iterNeighTab = NULL;

  if (neighTab == NULL)
    return;

  for (iterIndex = 0; iterIndex < HASHSIZE; iterIndex++)
    if (neighTab[iterIndex].next != &neighTab[iterIndex]) {
      iterNeighTab = neighTab[iterIndex].next;
      break;
    }
}

int
iterRouteTabNext(char *buff, int len)
{
  struct avl_node *rt_tree_node;

  if (iterRouteTab == NULL)
    return -1;

  snprintf(buff, len, "destination~%s~gateway~%s~interface~%s~metric~%d~",
           rawIpAddrToString(&iterRouteTab->rt_dst.prefix, ipAddrLen),
           rawIpAddrToString(&iterRouteTab->rt_best->rtp_nexthop.gateway, ipAddrLen),
           if_ifwithindex_name(iterRouteTab->rt_best->rtp_nexthop.iif_index), iterRouteTab->rt_best->rtp_metric.hops);

  rt_tree_node = avl_walk_next(&iterRouteTab->rt_tree_node);
  if (rt_tree_node) {
    iterRouteTab = rt_tree2rt(rt_tree_node);
  } else {
    iterRouteTab = NULL;
  }

  return 0;
}

void
iterRouteTabInit(void)
{
  struct avl_node *node;

  avl_init(&routingtree, avl_comp_prefix_default);
  routingtree_version = 0;

  node = avl_walk_first(&routingtree);
  iterRouteTab = node ? rt_tree2rt(node) : NULL;

}

/**
 * Grab the next topology entry.
 *
 * @param adr the address to look for
 * @return the entry found or NULL
 */
static struct tc_entry *
tas_getnext_tc_entry(struct tc_entry *tc)
{
  struct avl_node *node = avl_walk_next(&tc->vertex_node);

  return (node ? vertex_tree2tc(node) : NULL);
}

int
iterTcTabNext(char *buff, int len)
{
  int res;
  int i;
  struct tc_edge_entry *tc_edge;
  struct lqtextbuffer lqbuffer;

  if (iterTcTab == NULL)
    return -1;

  res = snprintf(buff, len, "main~%s~[~destinations~", rawIpAddrToString(&iterTcTab->addr, ipAddrLen));

  len -= res;
  buff += res;

  len -= 2;
  i = 0;

  OLSR_FOR_ALL_TC_EDGE_ENTRIES(iterTcTab, tc_edge) {

    res =
      snprintf(buff, len, "[~%d~address~%s~cost~%s~]~", i, rawIpAddrToString(&tc_edge->T_dest_addr, ipAddrLen),
               get_linkcost_text(tc_edge->cost, false, &lqbuffer));

    if (res < len)
      buff += res;

    len -= res;

    if (len <= 0)
      break;

    i++;
  }
  OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(iterTcTab, tc_edge);

  strcpy(buff, "]~");

  iterTcTab = tas_getnext_tc_entry(iterTcTab);

  return 0;
}

void
iterTcTabInit(void)
{
  struct avl_node *node;

  avl_init(&tc_tree, avl_comp_default);

  node = avl_walk_first(&tc_tree);
  iterTcTab = (node ? vertex_tree2tc(node) : NULL);
}

static bool
parserFunc(union olsr_message *msg, struct interface *inInt __attribute__ ((unused)), union olsr_ip_addr *neighIntAddr)
{
  char *mess = (char *)msg;
  union olsr_ip_addr *orig = (union olsr_ip_addr *)ARM_NOWARN_ALIGN(mess + 4);
  int len = (mess[2] << 8) | mess[3];
  char *service, *string;
  int i;

  if (memcmp(orig, mainAddr, ipAddrLen) == 0)
    return false;

  if (check_neighbor_link(neighIntAddr) != SYM_LINK) {
    error("TAS message not from symmetric neighbour\n");
    return false;
  }

  if (len < ipAddrLen + 8 + 2) {
    error("short TAS message received (%d bytes)\n", len);
    return false;
  }

  len -= ipAddrLen + 8;
  service = mess + ipAddrLen + 8;

  for (i = 0; i < len && service[i] != 0; i++);

  if (i++ == len) {
    error("TAS message has unterminated service string\n");
    return false;
  }

  if (i == len) {
    error("TAS message lacks payload string\n");
    return false;
  }

  string = service + i;
  len -= i;

  for (i = 0; i < len && string[i] != 0; i++);

  if (i == len) {
    error("TAS message has unterminated payload string\n");
    return false;
  }

  httpAddTasMessage(service, string, rawIpAddrToString(orig, ipAddrLen));

  /* Forward the message */
  return true;
}

void
sendMessage(const char *service, const char *string)
{
  unsigned char *mess, *walker;
  int len, pad;
  unsigned short seqNo;
  struct interface *inter;

  pad = len = ipAddrLen + 8 + strlen(service) + 1 + strlen(string) + 1;

  len = 1 + ((len - 1) | 3);

  pad = len - pad;

  walker = mess = allocMem(len);

  seqNo = get_msg_seqno();

  *walker++ = MESSAGE_TYPE;
  *walker++ = 0;
  *walker++ = (unsigned char)(len >> 8);
  *walker++ = (unsigned char)len;

  memcpy(walker, mainAddr, ipAddrLen);
  walker += ipAddrLen;

  *walker++ = 255;
  *walker++ = 0;
  *walker++ = (unsigned char)(seqNo >> 8);
  *walker++ = (unsigned char)seqNo;

  while (*service != 0)
    *walker++ = *service++;

  *walker++ = 0;

  while (*string != 0)
    *walker++ = *string++;

  *walker++ = 0;

  while (pad-- > 0)
    *walker++ = 0;

  for (inter = intTab; inter != NULL; inter = inter->int_next) {
    if (net_outbuffer_push(inter, mess, len) != len) {
      net_output(inter);
      net_outbuffer_push(inter, mess, len);
    }
  }
}

static void
serviceFunc(void *context __attribute__ ((unused)))
{
  static int up = 0;

  if (up == 0) {
    if (httpSetup() < 0)
      return;

    up = 1;
  }

  if (up != 0)
    httpService((int)(1.0 / config->pollrate));
}

int
olsrd_plugin_interface_version(void)
{
  return PLUGIN_INTERFACE_VERSION;
}

int
olsrd_plugin_init(void)
{
  ipAddrLen = olsr_cnf->ipsize;
  mainAddr = &olsr_cnf->main_addr;

  intTab = ifnet;
  neighTab = neighbortable;
  midTab = mid_set;
  hnaTab = hna_set;
  config = olsr_cnf;

  httpInit();

  olsr_start_timer(OLSR_TAS_SERVICE_INT, 0, OLSR_TIMER_PERIODIC, &serviceFunc, NULL, 0);

  olsr_parser_add_function(parserFunc, MESSAGE_TYPE);

  return 0;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "address",.set_plugin_parameter = &httpSetAddress,.data = NULL},
  {.name = "port",.set_plugin_parameter = &httpSetPort,.data = NULL},
  {.name = "rootdir",.set_plugin_parameter = &httpSetRootDir,.data = NULL},
  {.name = "workdir",.set_plugin_parameter = &httpSetWorkDir,.data = NULL},
  {.name = "indexfile",.set_plugin_parameter = &httpSetIndexFile,.data = NULL},
  {.name = "user",.set_plugin_parameter = &httpSetUser,.data = NULL},
  {.name = "password",.set_plugin_parameter = &httpSetPassword,.data = NULL},
  {.name = "sesstime",.set_plugin_parameter = &httpSetSessTime,.data = NULL},
  {.name = "pubdir",.set_plugin_parameter = &httpSetPubDir,.data = NULL},
  {.name = "quantum",.set_plugin_parameter = &httpSetQuantum,.data = NULL},
  {.name = "messtime",.set_plugin_parameter = &httpSetMessTime,.data = NULL},
  {.name = "messlimit",.set_plugin_parameter = &httpSetMessLimit,.data = NULL},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
