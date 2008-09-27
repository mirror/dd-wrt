/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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

#include "defs.h"
#include "interfaces.h"
#include "ifnet.h"
#include "scheduler.h"
#include "olsr.h"
#include "net_olsr.h"
#include "ipcalc.h"

/* The interface linked-list */
struct interface *ifnet;


/* Ifchange functions */
struct ifchgf
{
  int (*function)(struct interface *, int);
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
ifinit(void)
{
  struct olsr_if *tmp_if;

  /* Initial values */
  ifnet = NULL;

  /*
   * Get some cookies for getting stats to ease troubleshooting.
   */
  interface_poll_timer_cookie =
    olsr_alloc_cookie("Interface Polling", OLSR_COOKIE_TYPE_TIMER);

  hello_gen_timer_cookie =
    olsr_alloc_cookie("Hello Generation", OLSR_COOKIE_TYPE_TIMER);
  tc_gen_timer_cookie =
    olsr_alloc_cookie("TC Generation", OLSR_COOKIE_TYPE_TIMER);
  mid_gen_timer_cookie =
    olsr_alloc_cookie("MID Generation", OLSR_COOKIE_TYPE_TIMER);
  hna_gen_timer_cookie =
    olsr_alloc_cookie("HNA Generation", OLSR_COOKIE_TYPE_TIMER);

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
  olsr_start_timer((unsigned int)olsr_cnf->nic_chgs_pollrate * MSEC_PER_SEC, 5,
                   OLSR_TIMER_PERIODIC, &check_interface_updates, NULL,
                   interface_poll_timer_cookie->ci_id);

  return (ifnet == NULL) ? 0 : 1;
}

void
run_ifchg_cbs(struct interface *ifp, int flag)
{
  struct ifchgf *tmp_ifchgf_list = ifchgf_list;

  while(tmp_ifchgf_list != NULL)
    {
      tmp_ifchgf_list->function(ifp, flag);
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

  if(!addr)
    return NULL;

  for (ifp = ifnet; ifp; ifp = ifp->int_next)
    {
      if(olsr_cnf->ip_version == AF_INET)
	{
	  /* IPv4 */
	  //printf("Checking: %s == ", inet_ntoa(((struct sockaddr_in *)&ifp->int_addr)->sin_addr));
	  //printf("%s\n", olsr_ip_to_string(addr));

	  if (((struct sockaddr_in *)&ifp->int_addr)->sin_addr.s_addr == addr->v4.s_addr)
	      return ifp;
	}
      else
	{
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

  while (ifp) 
    {
      if (ifp->olsr_socket == fd)
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
  const struct interface * const ifp = if_ifwithindex(if_index);
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
queue_if(const char *name, int hemu)
{
  struct olsr_if *interf_n = olsr_cnf->interfaces;
  size_t name_size;

  //printf("Adding interface %s\n", name);

  /* check if the inerfaces already exists */
  while(interf_n != NULL)
    {
      if(memcmp(interf_n->name, name, strlen(name)) == 0)
	{
	  fprintf(stderr, "Duplicate interfaces defined... not adding %s\n", name);
	  return NULL;
	}
      interf_n = interf_n->next;
    }

  interf_n = olsr_malloc(sizeof(struct olsr_if), "queue interface");

  name_size = strlen(name) + 1;
  interf_n->name = olsr_malloc(name_size, "queue interface name");
  interf_n->cnf = NULL;
  interf_n->interf = NULL;
  interf_n->configured = 0;

  interf_n->host_emul = hemu ? OLSR_TRUE : OLSR_FALSE;

  strscpy(interf_n->name, name, name_size);
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
add_ifchgf(int (*f)(struct interface *, int))
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
del_ifchgf(int (*f)(struct interface *, int))
{
  struct ifchgf *tmp_ifchgf, *prev;

  tmp_ifchgf = ifchgf_list;
  prev = NULL;

  while(tmp_ifchgf)
    {
      if(tmp_ifchgf->function == f)
	{
	  /* Remove entry */
	  if(prev == NULL)
	    {
	      ifchgf_list = tmp_ifchgf->next;
	      free(tmp_ifchgf);
	    }
	  else
	    {
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
/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
