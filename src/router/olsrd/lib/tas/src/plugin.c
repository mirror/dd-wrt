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
 * $Id: plugin.c,v 1.6 2005/11/10 19:50:42 kattemat Exp $
 */

#include <string.h>
#include <time.h> // clock_t required by olsrd includes

#include "link.h"
#include "plugin.h"
#include "lib.h"
#include "os_unix.h"
#include "http.h"
#include "glua.h"
#include "glua_ext.h"

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
#include <lq_route.h>
#include <mpr_selector_set.h>
#include <duplicate_set.h>

#define MESSAGE_TYPE 129

int olsrd_plugin_interface_version(void);
int olsrd_plugin_register_param(char *name, char *value);
int olsrd_plugin_init(void);

static int ipAddrLen;
static union olsr_ip_addr *mainAddr;

static struct interface *intTab = NULL;
static struct neighbor_entry *neighTab = NULL;
static struct mid_entry *midTab = NULL;
static struct tc_entry *tcTab = NULL;
static struct hna_entry *hnaTab = NULL;
static struct rt_entry *routeTab = NULL;
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

static void __attribute__((constructor)) banner(void)
{
  printf("Tiny Application Server 0.1 by olsr.org\n");
}

static double lqToEtx(double lq, double nlq)
{
  if (lq < MIN_LINK_QUALITY || nlq < MIN_LINK_QUALITY)
    return 0.0;

  else
    return 1.0 / (lq * nlq);
}

int iterLinkTabNext(char *buff, int len)
{
  double etx;

  if (iterLinkTab == NULL)
    return -1;

  etx = lqToEtx(iterLinkTab->loss_link_quality,
                iterLinkTab->neigh_link_quality);

  snprintf(buff, len, "local~%s~remote~%s~main~%s~hysteresis~%f~lq~%f~nlq~%f~etx~%f~",
           rawIpAddrToString(&iterLinkTab->local_iface_addr, ipAddrLen),
           rawIpAddrToString(&iterLinkTab->neighbor_iface_addr, ipAddrLen),
           rawIpAddrToString(&iterLinkTab->neighbor->neighbor_main_addr, ipAddrLen),
           iterLinkTab->L_link_quality, iterLinkTab->loss_link_quality,
           iterLinkTab->neigh_link_quality, etx);

  iterLinkTab = iterLinkTab->next;

  return 0;
}

void iterLinkTabInit(void)
{
  iterLinkTab = get_link_set();
}

int iterNeighTabNext(char *buff, int len)
{
  int res;
  int i;
  struct neighbor_2_list_entry *neigh2;
  
  if (iterNeighTab == NULL)
    return -1;

  res = snprintf(buff, len,
                 "main~%s~symmetric~%s~mpr~%s~mprs~%s~willingness~%d~[~neighbors2~",
                 rawIpAddrToString(&iterNeighTab->neighbor_main_addr, ipAddrLen),
                 iterNeighTab->status == SYM ? "true" : "false",
                 iterNeighTab->is_mpr != 0 ? "true" : "false",
                 olsr_lookup_mprs_set(&iterNeighTab->neighbor_main_addr) != NULL ?
                 "true" : "false",
                 iterNeighTab->willingness);

  i = 0;

  len -= res;
  buff += res;

  len -= 2;

  for (neigh2 = iterNeighTab->neighbor_2_list.next;
       neigh2 != &iterNeighTab->neighbor_2_list;
       neigh2 = neigh2->next)
  {
    res = snprintf(buff, len, "%d~%s~", i,
                   rawIpAddrToString(&neigh2->neighbor_2->neighbor_2_addr,
                                     ipAddrLen));

    if (res < len)
      buff += res;

    len -= res;

    if (len <= 0)
      break;

    i++;
  }

  strcpy(buff, "]~");

  iterNeighTab = iterNeighTab->next;

  if (iterNeighTab == &neighTab[iterIndex])
  {
    iterNeighTab = NULL;
    
    while (++iterIndex < HASHSIZE)
      if (neighTab[iterIndex].next != &neighTab[iterIndex])
      {
        iterNeighTab = neighTab[iterIndex].next;
        break;
      }
  }

  return 0;
}

void iterNeighTabInit(void)
{
  iterNeighTab = NULL;

  if (neighTab == NULL)
    return;

  for (iterIndex = 0; iterIndex < HASHSIZE; iterIndex++)
    if (neighTab[iterIndex].next != &neighTab[iterIndex])
    {
      iterNeighTab = neighTab[iterIndex].next;
      break;
    }
}

int iterRouteTabNext(char *buff, int len)
{
  if (iterRouteTab == NULL)
    return -1;

  snprintf(buff, len, "destination~%s~gateway~%s~interface~%s~metric~%d~",
           rawIpAddrToString(&iterRouteTab->rt_dst, ipAddrLen),
           rawIpAddrToString(&iterRouteTab->rt_router, ipAddrLen),
           iterRouteTab->rt_if->int_name, iterRouteTab->rt_metric);

  iterRouteTab = iterRouteTab->next;

  if (iterRouteTab == &routeTab[iterIndex])
  {
    iterRouteTab = NULL;
    
    while (++iterIndex < HASHSIZE)
      if (routeTab[iterIndex].next != &routeTab[iterIndex])
      {
        iterRouteTab = routeTab[iterIndex].next;
        break;
      }
  }

  return 0;
}

void iterRouteTabInit(void)
{
  iterRouteTab = NULL;

  if (routeTab == NULL)
    return;

  for (iterIndex = 0; iterIndex < HASHSIZE; iterIndex++)
    if (routeTab[iterIndex].next != &routeTab[iterIndex])
    {
      iterRouteTab = routeTab[iterIndex].next;
      break;
    }
}

int iterTcTabNext(char *buff, int len)
{
  int res;
  int i;
  struct topo_dst *dest;
  
  if (iterTcTab == NULL)
    return -1;

  res = snprintf(buff, len,
                 "main~%s~[~destinations~",
                 rawIpAddrToString(&iterTcTab->T_last_addr, ipAddrLen));

  i = 0;

  len -= res;
  buff += res;

  len -= 2;

  for (dest = iterTcTab->destinations.next; dest != &iterTcTab->destinations;
       dest = dest->next)
  {
    res = snprintf(buff, len, "[~%d~address~%s~etx~%f~]~", i,
                   rawIpAddrToString(&dest->T_dest_addr, ipAddrLen),
                   lqToEtx(dest->link_quality, dest->inverse_link_quality));

    if (res < len)
      buff += res;

    len -= res;

    if (len <= 0)
      break;

    i++;
  }

  strcpy(buff, "]~");

  iterTcTab = iterTcTab->next;

  if (iterTcTab == &tcTab[iterIndex])
  {
    iterTcTab = NULL;
    
    while (++iterIndex < HASHSIZE)
      if (tcTab[iterIndex].next != &tcTab[iterIndex])
      {
        iterTcTab = tcTab[iterIndex].next;
        break;
      }
  }

  return 0;
}

void iterTcTabInit(void)
{
  iterTcTab = NULL;

  if (tcTab == NULL)
    return;

  for (iterIndex = 0; iterIndex < HASHSIZE; iterIndex++)
    if (tcTab[iterIndex].next != &tcTab[iterIndex])
    {
      iterTcTab = tcTab[iterIndex].next;
      break;
    }
}

static void parserFunc(union olsr_message *msg, struct interface *inInt,
                       union olsr_ip_addr *neighIntAddr)
{
  char *mess = (char *)msg;
  union olsr_ip_addr *orig = (union olsr_ip_addr *)(mess + 4);
  unsigned short seqNo = (mess[ipAddrLen + 6] << 8) | mess[ipAddrLen + 7];
  int len = (mess[2] << 8) | mess[3];
  char *service, *string;
  int i;

  if (memcmp(orig, mainAddr, ipAddrLen) == 0)
    return;

  if (check_neighbor_link(neighIntAddr) != SYM_LINK)
  {
    error("TAS message not from symmetric neighbour\n");
    return;
  }

  if (len < ipAddrLen + 8 + 2)
  {
    error("short TAS message received (%d bytes)\n", len);
    return;
  }

  if (olsr_check_dup_table_proc(orig, seqNo) != 0)
  {
    len -= ipAddrLen + 8;
    service = mess + ipAddrLen + 8;

    for (i = 0; i < len && service[i] != 0; i++);

    if (i++ == len)
    {
      error("TAS message has unterminated service string\n");
      return;
    }

    if (i == len)
    {
      error("TAS message lacks payload string\n");
      return;
    }

    string = service + i;
    len -= i;

    for (i = 0; i < len && string[i] != 0; i++);

    if (i == len)
    {
      error("TAS message has unterminated payload string\n");
      return;
    }

    httpAddTasMessage(service, string, rawIpAddrToString(orig, ipAddrLen));
  }

  olsr_forward_message(msg, orig, seqNo, inInt, neighIntAddr);
}

void sendMessage(const char *service, const char *string)
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

  for (inter = intTab; inter != NULL; inter = inter->int_next)
  {
    if (net_outbuffer_push(inter, mess, len) != len)
    {
      net_output(inter);
      net_outbuffer_push(inter, mess, len);
    }
  }
}

static void serviceFunc(void)
{
  static int up = 0;

  if (up == 0)
  {
    if (httpSetup() < 0)
      return;

    up = 1;
  }

  if (up != 0)
    httpService((int)(1.0 / config->pollrate));
}

int olsrd_plugin_interface_version(void)
{
  return 4;
}

int olsrd_plugin_init()
{
  ipAddrLen = ipsize;
  mainAddr = &main_addr;

  intTab = ifnet;
  neighTab = neighbortable;
  midTab = mid_set;
  tcTab = tc_table;
  hnaTab = hna_set;
  routeTab = routingtable;
  config = olsr_cnf;

  httpInit();
  
  olsr_register_timeout_function(serviceFunc);
  olsr_parser_add_function(parserFunc, MESSAGE_TYPE, 1);

  return 0;
}

int olsrd_plugin_register_param(char *name, char *value)
{
  if (strcmp(name, "address") == 0)
  {
    if (httpSetAddress(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "port") == 0)
  {
    if (httpSetPort(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "rootdir") == 0)
  {
    if (httpSetRootDir(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "workdir") == 0)
  {
    if (httpSetWorkDir(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "indexfile") == 0)
  {
    httpSetIndexFile(value);
    return 1;
  }

  if (strcmp(name, "user") == 0)
  {
    httpSetUser(value);
    return 1;
  }

  if (strcmp(name, "password") == 0)
  {
    httpSetPassword(value);
    return 1;
  }

  if (strcmp(name, "sesstime") == 0)
  {
    if (httpSetSessTime(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "pubdir") == 0)
  {
    httpSetPubDir(value);
    return 1;
  }

  if (strcmp(name, "quantum") == 0)
  {
    if (httpSetQuantum(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "messtime") == 0)
  {
    if (httpSetMessTime(value) < 0)
      return 0;

    return 1;
  }

  if (strcmp(name, "messlimit") == 0)
  {
    if (httpSetMessLimit(value) < 0)
      return 0;

    return 1;
  }

  return 0;
}
