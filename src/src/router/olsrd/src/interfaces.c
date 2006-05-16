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
 * $Id: interfaces.c,v 1.27 2005/06/03 08:00:55 kattemat Exp $
 */

#include "defs.h"
#include "interfaces.h"
#include "ifnet.h"
#include "scheduler.h"
#include "olsr.h"

static olsr_u32_t if_property_id;

/* The interface linked-list */
struct interface *ifnet;

/* Datastructures to use when creating new sockets */
struct sockaddr_in addrsock;
struct sockaddr_in6 addrsock6;


/* Ifchange functions */
struct ifchgf
{
  int (*function)(struct interface *, int);
  struct ifchgf *next;
};

static struct ifchgf *ifchgf_list;

/**
 *Do initialization of various data needed for
 *network interface management.
 *This function also tries to set up the given interfaces.
 *
 *@return the number of interfaces configured
 */
int
ifinit()
{
  struct olsr_if *tmp_if;


  /* Initial values */
  ifnet = NULL;

  /*
   *Initializing addrsock struct to be
   *used on all the sockets
   */
  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IP version 4 */
      memset(&addrsock, 0, sizeof (addrsock));
      addrsock.sin_family = AF_INET;
      addrsock.sin_port = olsr_udp_port;
      (addrsock.sin_addr).s_addr = INADDR_ANY;
    }
  else
    {
      /* IP version 6 */
      memset(&addrsock6, 0, sizeof (addrsock6));
      addrsock6.sin6_family = AF_INET6;
      addrsock6.sin6_port = olsr_udp_port;
      //(addrsock6.sin6_addr).s_addr = IN6ADDR_ANY_INIT;
    }

  OLSR_PRINTF(1, "\n ---- Interface configuration ---- \n\n")
    /* Run trough all interfaces immedeatly */
    for(tmp_if = olsr_cnf->interfaces; tmp_if != NULL; tmp_if = tmp_if->next)
      {
	if(!tmp_if->host_emul)
	  {
	    if(!olsr_cnf->host_emul) /* XXX: TEMPORARY! */
	      chk_if_up(tmp_if, 1);	
	  }
	else
	  add_hemu_if(tmp_if);
      }
  
  /* register network interface update function with scheduler */
  olsr_register_scheduler_event(&check_interface_updates, NULL, IFCHANGES_POLL_INT, 0, NULL);

  return (ifnet == NULL) ? 0 : 1;
}


olsr_u32_t
get_if_property_id()
{
  return if_property_id++;
}

olsr_bool
add_if_geninfo(struct interface *ifp, void *data, olsr_u32_t owner_id)
{
  struct if_gen_property *igp;

  if(get_if_geninfo(ifp, owner_id) != NULL)
    return OLSR_FALSE;

  igp = olsr_malloc(sizeof(struct if_gen_property), __func__);

  igp->owner_id = owner_id;
  igp->data = data;

  /* queue */
  igp->next = ifp->gen_properties;
  ifp->gen_properties = igp;

  return OLSR_TRUE;
}

void *
get_if_geninfo(struct interface *ifp, olsr_u32_t owner_id)
{
  struct if_gen_property *igp_list = ifp->gen_properties;


  while(igp_list)
    {
      if(igp_list->owner_id == owner_id)
	return igp_list->data;

      igp_list = igp_list->next;
    }

  return NULL;
}

void *
del_if_geninfo(struct interface *ifp, olsr_u32_t owner_id)
{
  void *data = NULL;
  struct if_gen_property *igp_list = ifp->gen_properties;
  struct if_gen_property *igp_prev = NULL;


  while(igp_list)
    {
      if(igp_list->owner_id == owner_id)
	break;

      igp_prev = igp_list;
      igp_list = igp_list->next;
    }

  /* Not found */
  if(igp_list == NULL)
    return NULL;

  /* Dequeue */
  if(igp_prev == NULL)
    {
      /* First elem */
      ifp->gen_properties = igp_list->next;
    }
  else
    {
      igp_prev->next = igp_list->next;
    }
  data = igp_list->data;
  free(igp_list);

  return data;
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
if_ifwithaddr(union olsr_ip_addr *addr)
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

	  if (COMP_IP(&((struct sockaddr_in *)&ifp->int_addr)->sin_addr, addr))
	      return ifp;
	}
      else
	{
	  /* IPv6 */
	  //printf("Checking %s ", olsr_ip_to_string((union olsr_ip_addr *)&ifp->int6_addr.sin6_addr));
	  //printf("== %s\n", olsr_ip_to_string((union olsr_ip_addr *)&((struct sockaddr_in6 *)addr)->sin6_addr));
	  if (COMP_IP(&ifp->int6_addr.sin6_addr, addr))
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
  struct interface *ifp;
  ifp = ifnet;

  while (ifp) 
    {
      /* good ol' strcmp should be sufficcient here */
      if (!strcmp(ifp->int_name, if_name))
	return ifp;
      ifp = ifp->int_next;
    }
  
  return NULL;
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
queue_if(char *name, int hemu)
{
  struct olsr_if *interf_n = olsr_cnf->interfaces;

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

  /* strlen () does not return length including terminating /0 */
  interf_n->name = olsr_malloc(strlen(name) + 1, "queue interface name");
  interf_n->cnf = NULL;
  interf_n->interf = NULL;
  interf_n->configured = 0;
  interf_n->index = olsr_cnf->ifcnt++;

  interf_n->host_emul = hemu ? OLSR_TRUE : OLSR_FALSE;

  strncpy(interf_n->name, name, strlen(name) + 1);
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
