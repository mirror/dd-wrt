
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

#include <signal.h>
#include <unistd.h>

#include "defs.h"
#include "interfaces.h"
#include "ifnet.h"
#include "scheduler.h"
#include "olsr.h"
#include "net_olsr.h"
#include "ipcalc.h"
#include "log.h"
#include "parser.h"

#ifdef WIN32
#include <winbase.h>
#define close(x) closesocket(x)
#endif

/* The interface linked-list */
struct interface *ifnet;

/* Ifchange functions */
struct ifchgf {
  void (*function) (int if_index, struct interface *, enum olsr_ifchg_flag);
  struct ifchgf *next;
};

static struct ifchgf *ifchgf_list;

/* Some cookies for stats keeping */
struct olsr_cookie_info *interface_poll_timer_cookie = NULL;
struct olsr_cookie_info *hello_gen_timer_cookie = NULL;
struct olsr_cookie_info *tc_gen_timer_cookie = NULL;
struct olsr_cookie_info *mid_gen_timer_cookie = NULL;
struct olsr_cookie_info *hna_gen_timer_cookie = NULL;

/**
 *Do initialization of various data needed for
 *network interface management.
 *This function also tries to set up the given interfaces.
 *
 *@return the number of interfaces configured
 */
int
olsr_init_interfacedb(void)
{
  struct olsr_if *tmp_if;

  /* Initial values */
  ifnet = NULL;

  /*
   * Get some cookies for getting stats to ease troubleshooting.
   */
  interface_poll_timer_cookie = olsr_alloc_cookie("Interface Polling", OLSR_COOKIE_TYPE_TIMER);

  hello_gen_timer_cookie = olsr_alloc_cookie("Hello Generation", OLSR_COOKIE_TYPE_TIMER);
  tc_gen_timer_cookie = olsr_alloc_cookie("TC Generation", OLSR_COOKIE_TYPE_TIMER);
  mid_gen_timer_cookie = olsr_alloc_cookie("MID Generation", OLSR_COOKIE_TYPE_TIMER);
  hna_gen_timer_cookie = olsr_alloc_cookie("HNA Generation", OLSR_COOKIE_TYPE_TIMER);

  OLSR_PRINTF(1, "\n ---- Interface configuration ---- \n\n");
  /* Run trough all interfaces immedeatly */
  for (tmp_if = olsr_cnf->interfaces; tmp_if != NULL; tmp_if = tmp_if->next) {
    if (!tmp_if->host_emul) {
      if (!olsr_cnf->host_emul) /* XXX: TEMPORARY! */
        chk_if_up(tmp_if, 1);
    } else {
      add_hemu_if(tmp_if);
    }
  }

  /* Kick a periodic timer for the network interface update function */
  olsr_start_timer((unsigned int)olsr_cnf->nic_chgs_pollrate * MSEC_PER_SEC, 5, OLSR_TIMER_PERIODIC, &check_interface_updates, NULL,
                   interface_poll_timer_cookie);

  return (ifnet == NULL) ? 0 : 1;
}

void
olsr_trigger_ifchange(int if_index, struct interface *ifp, enum olsr_ifchg_flag flag)
{
  struct ifchgf *tmp_ifchgf_list = ifchgf_list;

  while (tmp_ifchgf_list != NULL) {
    tmp_ifchgf_list->function(if_index, ifp, flag);
    tmp_ifchgf_list = tmp_ifchgf_list->next;
  }
}

/**
 *Find the local interface with a given address.
 *
 *@param addr the address to check.
 *
 *@return the interface struct representing the interface
 *that matched the address.
 */

struct interface *
if_ifwithaddr(const union olsr_ip_addr *addr)
{
  struct interface *ifp;

  if (!addr)
    return NULL;

  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if (olsr_cnf->ip_version == AF_INET) {
      /* IPv4 */
      //printf("Checking: %s == ", inet_ntoa(((struct sockaddr_in *)&ifp->int_addr)->sin_addr));
      //printf("%s\n", olsr_ip_to_string(addr));

      if (((struct sockaddr_in *)&ifp->int_addr)->sin_addr.s_addr == addr->v4.s_addr)
        return ifp;
    } else {
      /* IPv6 */
      //printf("Checking %s ", olsr_ip_to_string((union olsr_ip_addr *)&ifp->int6_addr.sin6_addr));
      //printf("== %s\n", olsr_ip_to_string((union olsr_ip_addr *)&((struct sockaddr_in6 *)addr)->sin6_addr));
      if (ip6equal(&ifp->int6_addr.sin6_addr, &addr->v6))
        return ifp;
    }
  }
  return NULL;
}

/**
 *Find the interface with a given number.
 *
 *@param nr the number of the interface to find.
 *
 *@return return the interface struct representing the interface
 *that matched the number.
 */
struct interface *
if_ifwithsock(int fd)
{
  struct interface *ifp;
  ifp = ifnet;

  while (ifp) {
    if (ifp->olsr_socket == fd || ifp->send_socket == fd)
      return ifp;
    ifp = ifp->int_next;
  }

  return NULL;
}

/**
 *Find the interface with a given label.
 *
 *@param if_name the label of the interface to find.
 *
 *@return return the interface struct representing the interface
 *that matched the label.
 */
struct interface *
if_ifwithname(const char *if_name)
{
  struct interface *ifp = ifnet;
  while (ifp) {
    /* good ol' strcmp should be sufficcient here */
    if (strcmp(ifp->int_name, if_name) == 0) {
      return ifp;
    }
    ifp = ifp->int_next;
  }
  return NULL;
}

/**
 *Find the olsr_if with a given label.
 *
 *@param if_name the label of the interface to find.
 *
 *@return return the interface struct representing the interface
 *that matched the label.
 */
struct olsr_if *
olsrif_ifwithname(const char *if_name)
{
  struct olsr_if *oifp = olsr_cnf->interfaces;
  while (oifp) {
    /* good ol' strcmp should be sufficcient here */
    if (strcmp(oifp->name, if_name) == 0) {
      return oifp;
    }
    oifp = oifp->next;
  }
  return NULL;
}

/**
 *Find the interface with a given interface index.
 *
 *@param iif_index of the interface to find.
 *
 *@return return the interface struct representing the interface
 *that matched the iif_index.
 */
struct interface *
if_ifwithindex(const int if_index)
{
  struct interface *ifp = ifnet;
  while (ifp != NULL) {
    if (ifp->if_index == if_index) {
      return ifp;
    }
    ifp = ifp->int_next;
  }
  return NULL;
}

/**
 *Get an interface name for a given interface index
 *
 *@param iif_index of the interface to find.
 *
 *@return "" or interface name.
 */
const char *
if_ifwithindex_name(const int if_index)
{
  const struct interface *const ifp = if_ifwithindex(if_index);
  return ifp == NULL ? "void" : ifp->int_name;
}

/**
 *Create a new interf_name struct using a given
 *name and insert it into the interface list.
 *
 *@param name the name of the interface.
 *
 *@return nada
 */
struct olsr_if *
olsr_create_olsrif(const char *name, int hemu)
{
  struct olsr_if *interf_n = olsr_cnf->interfaces;
  size_t name_size;

  //printf("Adding interface %s\n", name);

  /* check if the inerfaces already exists */
  while (interf_n != NULL) {
    if (strcmp(interf_n->name, name) == 0) {
      fprintf(stderr, "Duplicate interfaces defined... not adding %s\n", name);
      return NULL;
    }
    interf_n = interf_n->next;
  }

  interf_n = olsr_malloc(sizeof(struct olsr_if), "queue interface");

  name_size = strlen(name) + 1;
  interf_n->name = olsr_malloc(name_size, "queue interface name");
  strscpy(interf_n->name, name, name_size);

  interf_n->cnf = olsr_malloc(sizeof(*interf_n->cnf), "queue cnf");

  interf_n->cnfi = olsr_malloc(sizeof(*interf_n->cnfi), "queue cnfi");
  memset(interf_n->cnfi, 0xFF, sizeof(*interf_n->cnfi));
  interf_n->cnfi->orig_lq_mult_cnt=0;

  interf_n->host_emul = hemu ? true : false;

  interf_n->next = olsr_cnf->interfaces;
  olsr_cnf->interfaces = interf_n;

  return interf_n;
}

/**
 *Add an ifchange function. These functions are called on all (non-initial)
 *changes in the interface set.
 *
 *@param
 *
 *@return
 */
int
olsr_add_ifchange_handler(void (*f) (int if_index, struct interface *, enum olsr_ifchg_flag))
{

  struct ifchgf *new_ifchgf;

  new_ifchgf = olsr_malloc(sizeof(struct ifchgf), "Add ifchgfunction");

  new_ifchgf->next = ifchgf_list;
  new_ifchgf->function = f;

  ifchgf_list = new_ifchgf;

  return 1;
}

/*
 * Remove an ifchange function
 */
int
olsr_remove_ifchange_handler(void (*f) (int if_index, struct interface *, enum olsr_ifchg_flag))
{
  struct ifchgf *tmp_ifchgf, *prev;

  tmp_ifchgf = ifchgf_list;
  prev = NULL;

  while (tmp_ifchgf) {
    if (tmp_ifchgf->function == f) {
      /* Remove entry */
      if (prev == NULL) {
        ifchgf_list = tmp_ifchgf->next;
        free(tmp_ifchgf);
      } else {
        prev->next = tmp_ifchgf->next;
        free(tmp_ifchgf);
      }
      return 1;
    }
    prev = tmp_ifchgf;
    tmp_ifchgf = tmp_ifchgf->next;
  }

  return 0;
}

void
olsr_remove_interface(struct olsr_if * iface)
{
  struct interface *ifp, *tmp_ifp;
  ifp = iface->interf;

  OLSR_PRINTF(1, "Removing interface %s (%d)\n", iface->name, ifp->if_index);
  olsr_syslog(OLSR_LOG_INFO, "Removing interface %s\n", iface->name);

  olsr_delete_link_entry_by_ip(&ifp->ip_addr);

  /*
   *Call possible ifchange functions registered by plugins
   */
  olsr_trigger_ifchange(ifp->if_index, ifp, IFCHG_IF_REMOVE);

  /* cleanup routes over this interface */
  olsr_delete_interface_routes(ifp->if_index);

  /* Dequeue */
  if (ifp == ifnet) {
    ifnet = ifp->int_next;
  } else {
    tmp_ifp = ifnet;
    while (tmp_ifp->int_next != ifp) {
      tmp_ifp = tmp_ifp->int_next;
    }
    tmp_ifp->int_next = ifp->int_next;
  }

  /* Remove output buffer */
  net_remove_buffer(ifp);

  /* Check main addr */
  /* deactivated to prevent change of originator IP */
#if 0
  if (ipequal(&olsr_cnf->main_addr, &ifp->ip_addr)) {
    if (ifnet == NULL) {
      /* No more interfaces */
      memset(&olsr_cnf->main_addr, 0, olsr_cnf->ipsize);
      OLSR_PRINTF(1, "No more interfaces...\n");
    } else {
      struct ipaddr_str buf;
      olsr_cnf->main_addr = ifnet->ip_addr;
      OLSR_PRINTF(1, "New main address: %s\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));
      olsr_syslog(OLSR_LOG_INFO, "New main address: %s\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));
    }
  }
#endif
  /*
   * Deregister functions for periodic message generation
   */
  olsr_stop_timer(ifp->hello_gen_timer);
  olsr_stop_timer(ifp->tc_gen_timer);
  olsr_stop_timer(ifp->mid_gen_timer);
  olsr_stop_timer(ifp->hna_gen_timer);

  iface->configured = 0;
  iface->interf = NULL;

  /* Close olsr socket */
  remove_olsr_socket(ifp->olsr_socket, &olsr_input, NULL);
  close(ifp->olsr_socket);

  remove_olsr_socket(ifp->send_socket, &olsr_input, NULL);
  close(ifp->send_socket);

  /* Free memory */
  free(ifp->int_name);
  free(ifp);

  if ((ifnet == NULL) && (!olsr_cnf->allow_no_interfaces)) {
    olsr_syslog(OLSR_LOG_INFO, "No more active interfaces - exiting.\n");
    olsr_exit("No more active interfaces - exiting.\n", EXIT_FAILURE);
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
