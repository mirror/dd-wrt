
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

/*
 * -Threaded ping code added by Jens Nachtigall
 * -HNA4 checking by bjoern riemer
 */

#include <arpa/inet.h>

#include "olsr_types.h"
#include "olsrd_dyn_gw.h"
#include "olsr.h"
#include "defs.h"
#include "ipcalc.h"
#include "scheduler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <net/route.h>
#ifdef linux
#include <linux/in_route.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <time.h>
#ifndef WIN32
#include <pthread.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef interface

#define close(x) closesocket(x)

typedef HANDLE pthread_mutex_t;
typedef HANDLE pthread_t;

int pthread_create(HANDLE * Hand, void *Attr, void *(*Func) (void *), void *Arg);
int pthread_kill(HANDLE Hand, int Sig);
int pthread_mutex_init(HANDLE * Hand, void *Attr);
int pthread_mutex_lock(HANDLE * Hand);
int pthread_mutex_unlock(HANDLE * Hand);

struct ThreadPara {
  void *(*Func) (void *);
  void *Arg;
};
#endif

/* set default interval, in case none is given in the config file */
static int check_interval = 5;

/* list to store the Ping IP addresses given in the config file */
struct ping_list {
  char *ping_address;
  struct ping_list *next;
};

static struct ping_list *add_to_ping_list(const char *, struct ping_list *);

struct hna_list {
  union olsr_ip_addr hna_net;
  uint8_t hna_prefixlen;
  struct ping_list *ping_hosts;
  int hna_added;
  int probe_ok;
  struct hna_list *next;
};

static struct hna_list *add_to_hna_list(struct hna_list *, union olsr_ip_addr *hna_net, uint8_t hna_prefixlen);

struct hna_list *the_hna_list = NULL;

static void looped_checks(void *) __attribute__ ((noreturn));

static int check_gw(union olsr_ip_addr *, uint8_t, struct ping_list *);

static int ping_is_possible(struct ping_list *);

/* Event function to register with the scheduler */
static void olsr_event_doing_hna(void *);

/**
 * read config file parameters
 */
static int
set_plugin_ping(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  union olsr_ip_addr foo_addr;

  if (inet_pton(olsr_cnf->ip_version, value, &foo_addr) <= 0) {
    OLSR_PRINTF(0, "Illegal IP address \"%s\"", value);
    return 1;
  }
  /*if first ping without hna then assume inet gateway */
  if (the_hna_list == NULL) {
    union olsr_ip_addr temp_net;
    union olsr_ip_addr temp_netmask;
    temp_net.v4.s_addr = INET_NET;
    temp_netmask.v4.s_addr = INET_PREFIX;
    the_hna_list = add_to_hna_list(the_hna_list, &temp_net, olsr_netmask_to_prefix(&temp_netmask));
    if (the_hna_list == NULL) {
      return 1;
    }
  }
  the_hna_list->ping_hosts = add_to_ping_list(value, the_hna_list->ping_hosts);
  return 0;
}

static int
set_plugin_hna(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  union olsr_ip_addr temp_net;
  union olsr_ip_addr temp_netmask;
  char s_netaddr[128];
  char s_mask[128];

  //192.168.1.0  255.255.255.0
  int i = sscanf(value, "%127s %127s", s_netaddr, s_mask);
  if (i != 2) {
    OLSR_PRINTF(0, "Cannot get IP address and netmask from \"%s\"", value);
    return 1;
  }
  //printf("%s():i:%i; net:%s; mask:%s\n",__func__,i,s_netaddr,s_mask);
  if (inet_pton(olsr_cnf->ip_version, s_netaddr, &temp_net) <= 0) {
    OLSR_PRINTF(0, "Illegal IP address \"%s\"", s_netaddr);
    return 1;
  }
  //printf("GOT: %s(%08x)",inet_ntoa(foo_addr),foo_addr.s_addr);
  if (inet_pton(olsr_cnf->ip_version, s_netaddr, &temp_netmask) <= 0) {
    OLSR_PRINTF(0, "Illegal netmask \"%s\"", s_netaddr);
    return 1;
  }
  //printf("/%s(%08x)\n",inet_ntoa(foo_addr),foo_addr.s_addr);
  //printf("%s():got->%s/%s\n",__func__,olsr_ip_to_string((union olsr_ip_addr *)&));
  the_hna_list = add_to_hna_list(the_hna_list, &temp_net, olsr_netmask_to_prefix(&temp_netmask));
  if (the_hna_list != NULL) {
    return 1;
  }
  return 0;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "interval",.set_plugin_parameter = &set_plugin_int,.data = &check_interval},
  {.name = "ping",.set_plugin_parameter = &set_plugin_ping,.data = NULL},
  {.name = "hna",.set_plugin_parameter = &set_plugin_hna,.data = NULL},
};

void
olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size)
{
  *params = plugin_parameters;
  *size = sizeof(plugin_parameters) / sizeof(*plugin_parameters);
}

/**
 *Do initialization here
 *
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 *It is ran _after_ register_olsr_param
 */
int
olsrd_plugin_init(void)
{
  pthread_t ping_thread;

  //gw_net.v4 = INET_NET;
  //gw_netmask.v4 = INET_PREFIX;

  //gw_already_added = 0;
  //has_available_gw = 0;

  /* Remove all local Inet HNA entries */
  /*while(remove_local_hna4_entry(&gw_net, &gw_netmask))
     {
     olsr_printf(1, "HNA Internet gateway deleted\n");
     } */

  pthread_create(&ping_thread, NULL, (void *(*)(void *))looped_checks, NULL);

  /* Register the GW check */
  olsr_start_timer(3 * MSEC_PER_SEC, 0, OLSR_TIMER_PERIODIC, &olsr_event_doing_hna, NULL, 0);

  return 1;
}

/**
 * Scheduled event to update the hna table,
 * called from olsrd main thread to keep the hna table thread-safe
 */
static void
olsr_event_doing_hna(void *foo __attribute__ ((unused)))
{
  struct hna_list *li;
  /*
     if (has_available_gw == 1 && gw_already_added == 0) {
     olsr_printf(1, "Adding OLSR local HNA entry for Internet\n");
     add_local_hna_entry(&gw_net, &gw_netmask);
     gw_already_added = 1;
     } else if ((has_available_gw == 0) && (gw_already_added == 1)) {
     // Remove all local Inet HNA entries /
     while(remove_local_hna4_entry(&gw_net, &gw_netmask)) {
     olsr_printf(1, "Removing OLSR local HNA entry for Internet\n");
     }
     gw_already_added = 0;
     }
   */
  for (li = the_hna_list; li; li = li->next) {
    if ((li->probe_ok == 1) && (li->hna_added == 0)) {
      olsr_printf(1, "Adding OLSR local HNA entry\n");
      ip_prefix_list_add(&olsr_cnf->hna_entries, &li->hna_net, li->hna_prefixlen);
      li->hna_added = 1;
    } else if ((li->probe_ok == 0) && (li->hna_added == 1)) {
      while (ip_prefix_list_remove(&olsr_cnf->hna_entries, &li->hna_net, li->hna_prefixlen)) {
        olsr_printf(1, "Removing OLSR local HNA entry\n");
      }
      li->hna_added = 0;
    }
  }
}

/**
 * the threaded function which happens within an endless loop,
 * reiterated every "Interval" sec (as given in the config or
 * the default value)
 */
static void
looped_checks(void *foo __attribute__ ((unused)))
{
  for (;;) {
    struct hna_list *li;
    struct timespec remainder_spec;
    /* the time to wait in "Interval" sec (see connfig), default=5sec */
    struct timespec sleeptime_spec = { check_interval, 0L };

    for (li = the_hna_list; li; li = li->next) {
      /* check for gw in table entry and if Ping IPs are given also do pings */
      li->probe_ok = check_gw(&li->hna_net, li->hna_prefixlen, li->ping_hosts);
      //has_available_gw = check_gw(&gw_net, &gw_netmask);
    }

    while (nanosleep(&sleeptime_spec, &remainder_spec) < 0)
      sleeptime_spec = remainder_spec;
  }
  // return NULL;
}

static int
check_gw(union olsr_ip_addr *net, uint8_t prefixlen, struct ping_list *the_ping_list)
{
  char buf[1024], iface[16];
  uint32_t gate_addr, dest_addr, netmask;
  unsigned int iflags;
  int metric, refcnt, use;
  int retval = 0;
  union olsr_ip_addr mask;

  FILE *fp = fopen(PROCENTRY_ROUTE, "r");
  if (!fp) {
    perror(PROCENTRY_ROUTE);
    olsr_printf(1, "INET (IPv4) not configured in this system.\n");
    return -1;
  }

  olsr_prefix_to_netmask(&mask, prefixlen);
  /*
     olsr_printf(1, "Genmask         Destination     Gateway         "
     "Flags Metric Ref    Use Iface\n");
   */
  while (fgets(buf, sizeof(buf), fp)) {
    int num = sscanf(buf, "%15s %128X %128X %X %d %d %d %128X \n",
                     iface, &dest_addr, &gate_addr,
                     &iflags, &refcnt, &use, &metric, &netmask);
    if (num < 8)
      continue;

    /*
       olsr_printf(1, "%-15s ", olsr_ip_to_string((union olsr_ip_addr *)&netmask));

       olsr_printf(1, "%-15s ", olsr_ip_to_string((union olsr_ip_addr *)&dest_addr));

       olsr_printf(1, "%-15s %-6d %-2d %7d %s\n",
       olsr_ip_to_string((union olsr_ip_addr *)&gate_addr),
       metric, refcnt, use, iface);
     */

    if ((iflags & RTF_UP) && (metric == 0) && (netmask == mask.v4.s_addr) && (dest_addr == net->v4.s_addr)) {
      if (((mask.v4.s_addr == INET_PREFIX) && (net->v4.s_addr == INET_NET)) && (!(iflags & RTF_GATEWAY))) {
        fclose(fp);
        return retval;
      }
      /* don't ping, if there was no "Ping" IP addr in the config file */
      if (the_ping_list != NULL) {
        /*validate the found inet gw by pinging */
        if (ping_is_possible(the_ping_list)) {
          olsr_printf(1, "HNA[%08x/%08x](ping is possible) VIA %s detected in routing table.\n", dest_addr, netmask, iface);
          retval = 1;
        }
      } else {
        olsr_printf(1, "HNA[%08x/%08x] VIA %s detected in routing table.\n", dest_addr, netmask, iface);
        retval = 1;
      }
    }
  }

  fclose(fp);
  if (retval == 0) {
    /* And we cast here since we get warnings on Win32 */
    olsr_printf(1, "HNA[%08x/%08x] is invalid\n", (unsigned int)net->v4.s_addr, (unsigned int)mask.v4.s_addr);
  }
  return retval;
}

static int
ping_is_possible(struct ping_list *the_ping_list)
{
  struct ping_list *list;
  for (list = the_ping_list; list; list = list->next) {
    char ping_command[50];
    snprintf(ping_command, sizeof(ping_command), "ping -c 1 -q %s", list->ping_address);
    olsr_printf(1, "\nDo ping on %s ...\n", list->ping_address);
    if (system(ping_command) == 0) {
      olsr_printf(1, "...OK\n\n");
      return 1;
    }
    olsr_printf(1, "...FAILED\n\n");
  }
  return 0;
}

/* add the valid IPs to the head of the list */
static struct ping_list *
add_to_ping_list(const char *ping_address, struct ping_list *the_ping_list)
{
  struct ping_list *new = malloc(sizeof(struct ping_list));
  if (!new) {
    fprintf(stderr, "DYN GW: Out of memory!\n");
    exit(0);
  }
  new->ping_address = strdup(ping_address);
  new->next = the_ping_list;
  return new;
}

static struct hna_list *
add_to_hna_list(struct hna_list *list_root, union olsr_ip_addr *hna_net, uint8_t hna_prefixlen)
{
  struct hna_list *new = malloc(sizeof(struct hna_list));
  if (new == NULL) {
    fprintf(stderr, "DYN GW: Out of memory!\n");
    exit(0);
  }
  //memcpy(&new->hna_net,hna_net,sizeof(union hna_net));
  //memcpy(&new->hna_netmask,hna_netmask,sizeof(union hna_netmask));
  new->hna_net.v4 = hna_net->v4;
  new->hna_prefixlen = hna_prefixlen;
  new->hna_added = 0;
  new->probe_ok = 0;
  new->ping_hosts = NULL;
  new->next = list_root;
  return new;
}

#ifdef WIN32

/*
 * Windows ptread compat stuff
 */
static unsigned long __stdcall
ThreadWrapper(void *Para)
{
  struct ThreadPara *Cast;
  void *(*Func) (void *);
  void *Arg;

  Cast = (struct ThreadPara *)Para;

  Func = Cast->Func;
  Arg = Cast->Arg;

  HeapFree(GetProcessHeap(), 0, Para);

  Func(Arg);

  return 0;
}

int
pthread_create(HANDLE * Hand, void *Attr __attribute__ ((unused)), void *(*Func) (void *), void *Arg)
{
  struct ThreadPara *Para;
  unsigned long ThreadId;

  Para = HeapAlloc(GetProcessHeap(), 0, sizeof(struct ThreadPara));

  if (Para == NULL)
    return -1;

  Para->Func = Func;
  Para->Arg = Arg;

  *Hand = CreateThread(NULL, 0, ThreadWrapper, Para, 0, &ThreadId);

  if (*Hand == NULL)
    return -1;

  return 0;
}

int
pthread_kill(HANDLE Hand, int Sig __attribute__ ((unused)))
{
  if (!TerminateThread(Hand, 0))
    return -1;

  return 0;
}

int
pthread_mutex_init(HANDLE * Hand, void *Attr __attribute__ ((unused)))
{
  *Hand = CreateMutex(NULL, FALSE, NULL);

  if (*Hand == NULL)
    return -1;

  return 0;
}

int
pthread_mutex_lock(HANDLE * Hand)
{
  if (WaitForSingleObject(*Hand, INFINITE) == WAIT_FAILED)
    return -1;

  return 0;
}

int
pthread_mutex_unlock(HANDLE * Hand)
{
  if (!ReleaseMutex(*Hand))
    return -1;

  return 0;
}

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
