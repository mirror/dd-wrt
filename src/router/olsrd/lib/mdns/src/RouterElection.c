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

/* System includes */
#include <stddef.h>             /* NULL */
#include <sys/types.h>          /* ssize_t */
#include <string.h>             /* strerror() */
#include <stdarg.h>             /* va_list, va_start, va_end */
#include <errno.h>              /* errno */
#include <assert.h>             /* assert() */
#include <linux/if_ether.h>     /* ETH_P_IP */
#include <linux/if_packet.h>    /* struct sockaddr_ll, PACKET_MULTICAST */
#include <signal.h>             /* sigset_t, sigfillset(), sigdelset(), SIGINT */
#include <netinet/ip.h>         /* struct ip */
#include <netinet/udp.h>        /* struct udphdr */
#include <unistd.h>             /* close() */

#include <netinet/in.h>
#include <netinet/ip6.h>

/* OLSRD includes */
#include "plugin_util.h"        /* set_plugin_int */
#include "defs.h"               /* olsr_cnf, //OLSR_PRINTF */
#include "ipcalc.h"
#include "olsr.h"               /* //OLSR_PRINTF */
#include "mid_set.h"            /* mid_lookup_main_addr() */
#include "link_set.h"           /* get_best_link_to_neighbor() */
#include "net_olsr.h"           /* ipequal */
#include "hna_set.h"

/* plugin includes */
#include "NetworkInterfaces.h"  /* TBmfInterface, CreateBmfNetworkInterfaces(), CloseBmfNetworkInterfaces() */
#include "Address.h"            /* IsMulticast() */
#include "Packet.h"             /* ENCAP_HDR_LEN, BMF_ENCAP_TYPE, BMF_ENCAP_LEN etc. */
#include "list_backport.h"
#include "RouterElection.h"
#include "mdns.h"

struct RtElHelloPkt *hello;
uint8_t NETWORK_ID;
union olsr_ip_addr ROUTER_ID;

//List for routers
struct list_entity ListOfRouter;
#define ROUTER_ELECTION_ENTRIES(nr, iterator) listbackport_for_each_element_safe(&ListOfRouter, nr, list, iterator)

int ParseElectionPacket (struct RtElHelloPkt *rcvPkt, struct RouterListEntry *listEntry, int skfd){
  OLSR_PRINTF(1, "parsing ipv4 packet \n");
  listEntry->ttl = ENTRYTTL;
  listEntry->network_id = rcvPkt->network_id;
  listbackport_init_node(&listEntry->list);
  listEntry->skfd = skfd;
  (void) memcpy(&listEntry->router_id, &rcvPkt->router_id.v4, sizeof(struct in_addr));  //Need to insert an address validity check?
  return 1;
}

int ParseElectionPacket6 (struct RtElHelloPkt *rcvPkt, struct RouterListEntry6 *listEntry6, int skfd){
  OLSR_PRINTF(1, "parsing ipv6 packet \n");
  listEntry6->ttl = ENTRYTTL;
  listEntry6->network_id = rcvPkt->network_id;
  listbackport_init_node(&listEntry6->list);
  listEntry6->skfd = skfd;
  (void) memcpy(&listEntry6->router_id, &rcvPkt->router_id.v6, sizeof(struct in6_addr));//Need to insert an address validity check?
  return 1;
}

int UpdateRouterList (struct RouterListEntry *listEntry){

  struct RouterListEntry *tmp, *iterator;
  int exist = 0, status = 0;

  if (olsr_cnf->ip_version == AF_INET6)		//mdns plugin is running in ipv4, discard ipv6
    return 0;

  ROUTER_ELECTION_ENTRIES(tmp, iterator) {
    OLSR_PRINTF(1,"inspecting entry");
    if((tmp->network_id == listEntry->network_id) && (tmp->skfd == listEntry->skfd) &&
		(memcmp(&listEntry->router_id, &tmp->router_id, sizeof(struct in_addr)) == 0)){
      exist = 1;
      tmp->ttl = listEntry->ttl;
      status = 1;
    }
  }
    if (exist == 0)
      listbackport_add_tail(&ListOfRouter, &(listEntry->list));
  return status;
}

int UpdateRouterList6 (struct RouterListEntry6 *listEntry6){

  struct RouterListEntry6 *tmp, *iterator;
  int exist = 0, status = 0;

  if (olsr_cnf->ip_version == AF_INET)		//mdns plugin is running in ipv6, discard ipv4
    return 0;
 
  ROUTER_ELECTION_ENTRIES(tmp, iterator) { 
    if((tmp->network_id == listEntry6->network_id) && (tmp->skfd == listEntry6->skfd)  &&
              (memcmp(&listEntry6->router_id, &tmp->router_id, sizeof(struct in6_addr))) == 0){
      exist = 1;
      tmp->ttl = listEntry6->ttl;
      status = 1;
    }
  }
    if (exist == 0)
      listbackport_add_tail(&ListOfRouter, &(listEntry6->list));
  return status;
}

void helloTimer (void *foo __attribute__ ((unused))){

  struct TBmfInterface *walker;
  struct sockaddr_in dest;
  struct sockaddr_in6 dest6;
  OLSR_PRINTF(1,"hello start \n");

  for (walker = BmfInterfaces; walker != NULL; walker = walker->next) {
    if (olsr_cnf->ip_version == AF_INET) {
      memset((char *) &dest, 0, sizeof(dest));
      dest.sin_family = AF_INET;
      dest.sin_addr.s_addr = inet_addr("224.0.0.2");
      dest.sin_port = htons(5354);

      OLSR_PRINTF(1,"hello running \n");

      if (sendto(walker->helloSkfd, (const char * ) hello,
			sizeof(struct RtElHelloPkt), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        BmfPError("Could not send to interface %s", walker->olsrIntf->int_name);
      }
    }
    else{
      memset((char *) &dest6, 0, sizeof(dest6));
      dest6.sin6_family = AF_INET6;
      (void) inet_pton(AF_INET6, "ff02::2", &dest6.sin6_addr);
      dest6.sin6_port = htons(5354);

      OLSR_PRINTF(1,"hello running \n");

      if (sendto(walker->helloSkfd, (const char * ) hello,
                        sizeof(struct RtElHelloPkt), 0, (struct sockaddr *)&dest6, sizeof(dest6)) < 0) {
        BmfPError("Could not send to interface %s", walker->olsrIntf->int_name);
      }
    }
  }
  return;
}

void electTimer (void *foo __attribute__ ((unused))){

  struct TBmfInterface *walker;
  struct RouterListEntry *tmp, *iterator;
  struct RouterListEntry6 *tmp6, *iterator6;

  OLSR_PRINTF(1,"elect start \n");

  for(walker = BmfInterfaces; walker != NULL; walker = walker->next){
    if (listbackport_is_empty(&ListOfRouter)){
      walker->isActive = 1;
      OLSR_PRINTF(1,"elect empty \n");
      continue;
    }

    walker->isActive = 1;
    if (olsr_cnf->ip_version == AF_INET) {
      ROUTER_ELECTION_ENTRIES(tmp, iterator){
        OLSR_PRINTF(1,"inspecting element \n");
        if(tmp->network_id == NETWORK_ID)
          if(tmp->skfd == walker->electionSkfd)
            if(memcmp(&tmp->router_id, &ROUTER_ID.v4, sizeof(struct in_addr)) < 0)
              walker->isActive = 0;
        OLSR_PRINTF(1,"confrontation done \n");
        tmp->ttl = ((tmp->ttl)- 1);
        if(tmp->ttl <= 0){
          listbackport_remove(&tmp->list);
          free(tmp);
        }
        OLSR_PRINTF(1,"inspect finish \n");
      }
    }
    else{
      ROUTER_ELECTION_ENTRIES(tmp6, iterator6){
        if(tmp6->network_id == NETWORK_ID)
          if(tmp6->skfd == walker->electionSkfd)
            if(memcmp(&tmp6->router_id, &ROUTER_ID.v6, sizeof(struct in6_addr)) < 0)
              walker->isActive = 0;
        tmp6->ttl = ((tmp6->ttl)- 1);
        if(tmp6->ttl <=  0){
          listbackport_remove(&tmp6->list);
          free(tmp6);
        }
      }
    }

    OLSR_PRINTF(1,"elect finish \n");
  }

  return;
}

void initTimer (void *foo __attribute__ ((unused))){
  listbackport_init_head(&ListOfRouter);

  NETWORK_ID = (uint8_t) 1;             //Default Network id

  OLSR_PRINTF(1,"Initialization \n");
  memcpy(&ROUTER_ID, &olsr_cnf->main_addr, sizeof(union olsr_ip_addr));
  hello = (struct RtElHelloPkt *) malloc(sizeof(struct RtElHelloPkt));
  OLSR_PRINTF(1,"initialization running step 1\n");
  memcpy(hello->head, "$REP", 4);
  if(olsr_cnf->ip_version == AF_INET)
    hello->ipFamily = AF_INET;
  else
    hello->ipFamily = AF_INET6;
  hello->network_id = NETWORK_ID;
  memcpy(&hello->router_id, &ROUTER_ID, sizeof(union olsr_ip_addr));
  OLSR_PRINTF(1,"initialization end\n");
  return;
}

int
set_Network_ID(const char *Network_ID, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  int temp;
  assert(Network_ID!= NULL);
  set_plugin_int(Network_ID, &temp, addon);
  NETWORK_ID = (uint8_t) temp;
  return 0;
} /* Set Network ID */


int InitRouterList(void *foo __attribute__ ((unused))){

  struct olsr_cookie_info *RouterElectionTimerCookie = NULL;
  struct olsr_cookie_info *HelloTimerCookie = NULL;
  struct olsr_cookie_info *InitCookie = NULL;

  RouterElectionTimerCookie = olsr_alloc_cookie("Router Election", OLSR_COOKIE_TYPE_TIMER);
  HelloTimerCookie = olsr_alloc_cookie("Hello Packet", OLSR_COOKIE_TYPE_TIMER);
  InitCookie = olsr_alloc_cookie("Init", OLSR_COOKIE_TYPE_TIMER);

  olsr_start_timer((unsigned int) INIT_TIMER * MSEC_PER_SEC, 0, OLSR_TIMER_ONESHOT, initTimer, NULL,
		   InitCookie);
  olsr_start_timer((unsigned int) HELLO_TIMER * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, helloTimer, NULL,
		   HelloTimerCookie);
  olsr_start_timer((unsigned int) ELECTION_TIMER * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, electTimer, NULL,
                   RouterElectionTimerCookie);

  return 0;
}
