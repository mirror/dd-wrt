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
#include "log.h"
#include "routing_table.h"
#include "olsr_cfg.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <net/route.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#ifndef _WIN32
#include <pthread.h>
#else /* _WIN32 */
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
#endif /* _WIN32 */

static int hna_check_interval	= DEFAULT_HNA_CHECK_INTERVAL;
/* set default interval, in case none is given in the config file */
static int ping_check_interval = DEFAULT_PING_CHECK_INTERVAL;

/* list to store the Ping IP addresses given in the config file */
struct ping_list {
  char *ping_address;
  struct ping_list *next;
};

static struct ping_list *add_to_ping_list(const char *, struct ping_list *);

struct hna_list {
  union olsr_ip_addr   hna_addr;
  uint8_t              hna_prefixlen;
  bool                 hna_added;
  bool                 checked;
  bool                 active;
  struct hna_list *    next;
};

static struct hna_list *add_to_hna_list(struct hna_list *, union olsr_ip_addr *hna_addr, uint8_t hna_prefixlen);

struct hna_group {
  struct hna_list *    hna_list;
  struct ping_list *   ping_hosts;
  bool                 probe_ok;
  struct hna_group *   next;
};

bool hna_ping_check	= false;
static struct hna_group * hna_groups = NULL;

static struct hna_group *add_to_hna_group(struct hna_group *);

static void looped_checks(void *) __attribute__ ((noreturn));

static bool check_gw(union olsr_ip_addr *, uint8_t, struct ping_list *);

static int ping_is_possible(struct ping_list *);

static char ping_cmd[PING_CMD_MAX_LEN] = { DEFAULT_PING_CMD };

/* Event function to register with the scheduler */
static void olsr_event_doing_hna(void *);

struct hna_list* find_hna(uint32_t src_addr, uint32_t src_mask);

char *get_ip_str(uint32_t address, char *s, size_t maxlen);
int update_routing(void);

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

  if (hna_groups == NULL) { 
    hna_groups = add_to_hna_group(hna_groups);
    if (hna_groups == NULL)
      return 1;
  } else {
    if (hna_groups->hna_list != NULL) {
      hna_groups = add_to_hna_group(hna_groups);
    }
  }

  hna_groups->ping_hosts = add_to_ping_list(value, hna_groups->ping_hosts);
  hna_ping_check = true;
  
  return 0;
}

static int
set_plugin_hna(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  union olsr_ip_addr temp_addr;
  union olsr_ip_addr temp_mask;
  char s_addr[128];
  char s_mask[128];
  
  //Example: 192.168.1.0  255.255.255.0
  int i = sscanf(value, "%127s %127s", s_addr, s_mask);
  if (i != 2) {
    OLSR_PRINTF(0, "Cannot get IP address and netmask from \"%s\"", value);
    return 1;
  }

  if (inet_pton(olsr_cnf->ip_version, s_addr, &temp_addr) <= 0) {
    OLSR_PRINTF(0, "Illegal IP address \"%s\"", s_addr);
    return 1;
  }

  if (inet_pton(olsr_cnf->ip_version, s_mask, &temp_mask) <= 0) {
    OLSR_PRINTF(0, "Illegal netmask \"%s\"", s_mask);
    return 1;
  }

  if (hna_groups == NULL)
  {
    hna_groups = add_to_hna_group(hna_groups);
    if (hna_groups == NULL) {
      return 1;
    }
  }
	
  hna_groups->hna_list = add_to_hna_list(hna_groups->hna_list, &temp_addr, olsr_netmask_to_prefix(&temp_mask));
  if (hna_groups->hna_list == NULL) {
    return 1;
  }
  return 0;
}

static int
set_plugin_cmd(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  size_t len = strlen(value);

  if (len < PING_CMD_MAX_LEN) {
    strncpy(ping_cmd, value, PING_CMD_MAX_LEN - 1);
    ping_cmd[PING_CMD_MAX_LEN - 1] = '\0';
    return 0;
  }

  return 1;
}

static const struct olsrd_plugin_parameters plugin_parameters[] = {
  {.name = "interval",      .set_plugin_parameter = &set_plugin_int,  .data = &ping_check_interval  },
  {.name = "pinginterval",  .set_plugin_parameter = &set_plugin_int,  .data = &ping_check_interval  },
  {.name = "checkinterval", .set_plugin_parameter = &set_plugin_int,  .data = &hna_check_interval   },
  {.name = "ping",          .set_plugin_parameter = &set_plugin_ping, .data = NULL                  },
  {.name = "hna",           .set_plugin_parameter = &set_plugin_hna,  .data = NULL                  },
  {.name = "pingcmd",       .set_plugin_parameter = &set_plugin_cmd,  .data = &ping_cmd             },
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

  if (hna_groups == NULL) {
    hna_groups = add_to_hna_group(hna_groups);
    if (hna_groups == NULL)
      return 1;
  }
	
  // Add a default gateway if the top entry was just a ping address
  if (hna_groups->hna_list == NULL) {
    union olsr_ip_addr temp_addr;
    union olsr_ip_addr temp_mask;
    
    temp_addr.v4.s_addr = INET_ADDR;
    temp_mask.v4.s_addr = INET_MASK;
    hna_groups->hna_list = add_to_hna_list(hna_groups->hna_list, &temp_addr, olsr_netmask_to_prefix(&temp_mask));
    if (hna_groups->hna_list == NULL) {
      return 1;
    }
  }
	
  // Prepare all routing information
  update_routing();
  
  if (hna_ping_check) {
    pthread_create(&ping_thread, NULL, (void *(*)(void *))looped_checks, NULL);
  } else {
    struct hna_group *grp;
    for (grp = hna_groups; grp; grp = grp->next) {
      grp->probe_ok = true;
    }
  }

  // Print the current configuration
  {
    struct hna_group *grp;
    int i = 0;
    for (grp = hna_groups; grp; grp = grp->next, ++i) {
      struct hna_list *lst;
      struct ping_list *png;
	    
      olsr_printf(1, "Group %d:\n", i);
      for (lst = grp->hna_list; lst; lst = lst->next) {
        char addr[INET_ADDRSTRLEN];
        olsr_printf(1, "  HNA %s\n", get_ip_str(lst->hna_addr.v4.s_addr, addr, INET_ADDRSTRLEN));
      }
      for (png = grp->ping_hosts; png; png = png->next) {
        olsr_printf(1, "  PING %s\n", png->ping_address);
      }
    }
  }

  /* Register the GW check */
  olsr_start_timer(hna_check_interval, 0, OLSR_TIMER_PERIODIC, &olsr_event_doing_hna, NULL, 0);
  return 1;
}

void olsrd_plugin_fini(void) {
  if (!hna_groups) {
    return;
  }

  while (hna_groups->ping_hosts) {
    struct ping_list* next = hna_groups->ping_hosts->next;
    free(hna_groups->ping_hosts->ping_address);
    free(hna_groups->ping_hosts);
    hna_groups->ping_hosts = next;
  }

  while (hna_groups->hna_list) {
    struct hna_list * next = hna_groups->hna_list->next;
    free(hna_groups->hna_list);
    hna_groups->hna_list = next;
  }

  free(hna_groups);
  hna_groups = NULL;
}


/**
 * Scheduled event to update the hna table,
 * called from olsrd main thread to keep the hna table thread-safe
 */
static void
olsr_event_doing_hna(void *foo __attribute__ ((unused)))
{
  struct hna_group* grp;
  struct hna_list *li;

  update_routing();
  
  for (grp = hna_groups; grp; grp = grp->next) {
    for (li = grp->hna_list; li; li = li->next) {
      if (!li->hna_added) {
        if (grp->probe_ok && li->active) {
          olsr_printf(1, "Adding OLSR local HNA entry\n");
          ip_prefix_list_add(&olsr_cnf->hna_entries, &li->hna_addr, li->hna_prefixlen);
          li->hna_added = true;
        }
      } else {
        if (!grp->probe_ok || !li->active) {
          while (ip_prefix_list_remove(&olsr_cnf->hna_entries, &li->hna_addr, li->hna_prefixlen)) {
            olsr_printf(1, "Removing OLSR local HNA entry\n");
          }
          li->hna_added = false;
        }
      }
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
    struct hna_group *grp;
    struct hna_list *li;
    struct timespec remainder_spec;
    /* the time to wait in "Interval" sec (see connfig), default=5sec */
    struct timespec sleeptime_spec = { ping_check_interval, 0L };

    for (grp = hna_groups; grp; grp = grp->next) {
      for (li = grp->hna_list; li; li = li->next) {
      
    		// If this HNA is not active skip to the next one
        if (!li->active)
          continue;
          
        /* check for gw in table entry and if Ping IPs are given also do pings */
        grp->probe_ok = check_gw(&li->hna_addr, li->hna_prefixlen, grp->ping_hosts);
        if (grp->probe_ok)
          break;	// Valid host found so we can bail out of the inner loop here
      }
    }

    while (nanosleep(&sleeptime_spec, &remainder_spec) < 0)
      sleeptime_spec = remainder_spec;
  }
  // return NULL;
}

/* -------------------------------------------------------------------------
 * Function   : find_hna
 * Description: Lookup an HNA that matches the specified parameters
 * Input      : src_addr - IP address of the HNA to find
 *              src_mask - Address mask of the HNA to find
 * Output     : none
 * Return     : The HNA specified or NULL when HNA not found
 * Data Used  : none
 * ------------------------------------------------------------------------- */
struct hna_list*
find_hna(uint32_t src_addr, uint32_t src_mask)
{
  struct hna_group * grp;
  struct hna_list *li;
  union olsr_ip_addr mask;

  for (grp = hna_groups; grp; grp = grp->next) {
    for (li = grp->hna_list; li; li = li->next) {
      olsr_prefix_to_netmask(&mask, li->hna_prefixlen);
      if (li->hna_addr.v4.s_addr == src_addr && mask.v4.s_addr == src_mask) {
        return li;
      }
    }
  }
  return NULL;
}

/* -------------------------------------------------------------------------
 * Function   : get_ip_str
 * Description: Convert the specified address to an IPv4 compatible string
 * Input      : address - IPv4 address to convert to string 
 *              s       - string buffer to contain the resulting string
 *              maxlen  - maximum length of the string buffer 
 * Output     : none
 * Return     : Pointer to the string buffer containing the result
 * Data Used  : none
 * ------------------------------------------------------------------------- */
char *
get_ip_str(uint32_t address, char *s, size_t maxlen)
{
  struct sockaddr_in v4;
  
  v4.sin_addr.s_addr = address;
  inet_ntop(AF_INET, &v4.sin_addr, s, maxlen);

  return s;
}

/* -------------------------------------------------------------------------
 * Function   : update_routing
 * Description: Mark the HNAs in the HNA list(s) corresponding to the results
 *              found in the routing table. HNAs that are found in the routing
 *              table will be marked as 'active', otherwise they'll remain
 *              inactive.    
 * Input      : nothing
 * Output     : none
 * Return     : -1 if an error occurred, 0 otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
int 
update_routing(void)
{
  char buf[1024], iface[16];
  uint32_t gate_addr, dest_addr, netmask;
  unsigned int iflags;
  int metric, refcnt, use;
  struct hna_group *grp;
  struct hna_list *li;
  
  FILE *fp = fopen(PROCENTRY_ROUTE, "r");
  if (!fp) {
    perror(PROCENTRY_ROUTE);
    olsr_printf(1, "INET (IPv4) not configured in this system.\n");
    return -1;
  }

  // Phase 1: reset the 'checked' flag, during the check of the routing table we 
  // will (re)discover whether the HNA is valid or not.
  for (grp = hna_groups; grp; grp = grp->next) {
    for (li = grp->hna_list; li; li = li->next) {
      li->checked = false;
    }
  }

  /*
     olsr_printf(1, "Genmask         Destination     Gateway         "
     "Flags Metric Ref    Use Iface\n");
   */
  while (fgets(buf, sizeof(buf), fp)) {
    struct hna_list *hna;
    char s_addr[INET_ADDRSTRLEN], s_mask[INET_ADDRSTRLEN];
    
    int num = sscanf(buf, 
                     "%15s %128X %128X %X %d %d %d %128X \n",
                     iface, 
                     &dest_addr,
                     &gate_addr,
                     &iflags, 
                     &refcnt,
                     &use,
                     &metric,
                     &netmask);
    if (num < 8)
      continue;

    get_ip_str(dest_addr, s_addr, INET_ADDRSTRLEN);
    get_ip_str(netmask, s_mask, INET_ADDRSTRLEN);
    
    hna = find_hna(dest_addr, netmask);
    if (hna == NULL) {	// Entry not found, try the next one
      continue;
    }
    
    if ((iflags & RTF_UP) && (metric != olsr_cnf->fib_metric_default)) {
      hna->checked = true;
    }
  }
  fclose(fp);
  
  // Phase 2: now copy the 'checked' flag to the 'active' flag.
  // The total check is a 2-phase process so the ping check loop won't be 
  // disturbed too badly.
  for (grp = hna_groups; grp; grp = grp->next) {
    for (li = grp->hna_list; li; li = li->next) {
      li->active = li->checked;
    }
  }
	
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : check_gw
 * Description: Check the specified gateway(s) by sending a ping
 * Input      : addr      - the address of the HNA to which the ping is related
 *              prefixlen - the length of the prefix for this HNA 
 *              the_ping_list - list with related ping hosts
 * Output     : none
 * Return     : true if the ping host could be reached, false otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static bool
check_gw(union olsr_ip_addr *addr, uint8_t prefixlen, struct ping_list *the_ping_list)
{
  bool retval = false;
  union olsr_ip_addr mask;

  olsr_prefix_to_netmask(&mask, prefixlen);
  
  /* don't ping, if there was no "Ping" IP addr in the config file */
  if (the_ping_list != NULL) {
    /*validate the found inet gw by pinging */
    if (ping_is_possible(the_ping_list)) {
      olsr_printf(1, "HNA[%08x/%08x](ping is possible) detected in routing table.\n", addr->v4.s_addr, mask.v4.s_addr);
      retval = true;
    }
  } else {
    olsr_printf(1, "HNA[%08x/%08x] detected in routing table.\n", addr->v4.s_addr, mask.v4.s_addr);
    retval = true;
  }

  if (retval == false) {
    /* And we cast here since we get warnings on Win32 */
    olsr_printf(1, "HNA[%08x/%08x] is invalid\n", (unsigned int)addr->v4.s_addr, (unsigned int)mask.v4.s_addr);
  }
  return retval;
}

/* -------------------------------------------------------------------------
 * Function   : ping_is_possible
 * Description: Ping the specified host(s)
 * Input      : the_ping_list - the list of hosts to ping
 * Output     : none
 * Return     : 1 if any host responded, 0 otherwise
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static int
ping_is_possible(struct ping_list *the_ping_list)
{
  struct ping_list *list;
  for (list = the_ping_list; list; list = list->next) {
    char ping_command[sizeof(ping_cmd) + INET6_ADDRSTRLEN];
    snprintf(ping_command, sizeof(ping_command), ping_cmd, list->ping_address);
    olsr_printf(1, "\nDo ping on (%s) %s ...\n", ping_cmd, list->ping_address);
    if (system(ping_command) == 0) {
      olsr_printf(1, "...OK\n\n");
      return 1;
    }
    olsr_printf(1, "...FAILED\n\n");
  }
  return 0;
}

/* -------------------------------------------------------------------------
 * Function   : add_to_ping_list
 * Description: Add a new ping host to the list of ping hosts
 * Input      : ping_address - the address of the ping host
 *              the_ping_list - the list of ping hosts 
 * Output     : none
 * Return     : a pointer to the newly added ping host, i.e. start of the list
 * Data Used  : none
 * ------------------------------------------------------------------------- */
/* add the valid IPs to the head of the list */
static struct ping_list *
add_to_ping_list(const char *ping_address, struct ping_list *the_ping_list)
{
  struct ping_list *new = calloc(1, sizeof(struct ping_list));
  if (!new) {
    olsr_exit("DYN GW: Out of memory", EXIT_FAILURE);
  }
  new->ping_address = strdup(ping_address);
  new->next = the_ping_list;
  return new;
}

/* -------------------------------------------------------------------------
 * Function   : add_to_hna_list
 * Description: Add a new HNA entry to the list of HNA entries
 * Input      : list_root - the start of the list with HNA entries
 *              hna_addr  - the address of the new HNA entry
 *              prefixlen - the prefix-length of the new HNA entry 
 * Output     : none
 * Return     : a pointer to the newly added HNA entry, i.e. start of the list
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static struct hna_list *
add_to_hna_list(struct hna_list *list_root, union olsr_ip_addr *hna_addr, uint8_t hna_prefixlen)
{
  struct hna_list *new = calloc(1, sizeof(struct hna_list));
  if (new == NULL) {
    olsr_exit("DYN GW: Out of memory", EXIT_FAILURE);
  }

  new->hna_addr.v4 = hna_addr->v4;
  new->hna_prefixlen = hna_prefixlen;
  new->hna_added = false;
  new->next = list_root;
  return new;
}

/* -------------------------------------------------------------------------
 * Function   : add_to_hna_group
 * Description: Add a new HNA group to the list of HNA groups
 * Input      : list_root - the start of the list with HNA groups
 * Output     : none
 * Return     : a pointer to the newly added HNA group, i.e. start of the list
 * Data Used  : none
 * ------------------------------------------------------------------------- */
static struct hna_group *
add_to_hna_group(struct hna_group *list_root)
{
  struct hna_group *new = calloc(1, sizeof(struct hna_group));
  if (new == NULL) {
    olsr_exit("DYN GW: Out of memory", EXIT_FAILURE);
  }
	
  new->next =  list_root;
  return new;
}


#ifdef _WIN32

/*
 * Windows pthread compat stuff
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

#endif /* _WIN32 */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
