/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * RIB implementation (c) 2007, Hannes Gredler (hannes@gredler.at)
 * All rights reserved.
 *
 * export_route_entry interface added by Immo 'FaUl Wehrenberg 
 * <immo@chaostreff-dortmund.de>
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
 * $Id: process_routes.c,v 1.37 2007/09/16 21:14:25 bernd67 Exp $
 */

#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "kernel_routes.h"
#include <assert.h>
#include "lq_avl.h"

#ifdef WIN32
#undef strerror
#define strerror(x) StrError(x)
#endif

struct export_route_entry
{
  olsr_u8_t type;       /* AF_INET/AF_INET6 */
  int (*function)(struct rt_entry*);
  struct export_route_entry *next;
};


static struct export_route_entry *add_routes;
static struct export_route_entry *del_routes;

struct list_node add_kernel_list;
struct list_node chg_kernel_list;
struct list_node del_kernel_list;

/**
 *
 * Calculate the kernel route flags.
 * Called before enqueuing a change/delete operation
 *
 */
olsr_u8_t
olsr_rt_flags(const struct rt_entry *rt)
{
  const struct rt_nexthop *nh;
  olsr_u8_t flags;

  flags = (RTF_UP);

  if (rt->rt_dst.prefix_len == olsr_cnf->maxplen) {
    flags |= RTF_HOST;
  }

  nh = olsr_get_nh(rt);

  if(!COMP_IP(&rt->rt_dst.prefix, &nh->gateway)) {
    flags |= RTF_GATEWAY;
  }

  return flags;
}

void 
olsr_addroute_add_function(int (*function)(struct rt_entry*), olsr_u8_t type) 
{
  struct export_route_entry *tmp;
  tmp = olsr_malloc(sizeof *tmp, "olsr_addroute_add_function");
  tmp->type = type;
  tmp->function = function;
  tmp->next = add_routes;
  add_routes = tmp;
}

void 
olsr_delroute_add_function(int (*function) (struct rt_entry*), olsr_u8_t type)
{
  struct export_route_entry *tmp;
  tmp = olsr_malloc(sizeof *tmp, "olsr_delroute_add_function");
  tmp->type = type;
  tmp->function = function;
  tmp->next = del_routes;
  del_routes = tmp;
}


int 
olsr_addroute_remove_function(int (*function) (struct rt_entry*), olsr_u8_t type)
{
  struct export_route_entry *tmp, *prev = NULL /* Make compiler happy */; 
  tmp = add_routes;
  while (tmp) 
    {
      if (function == tmp->function && type == tmp->type) 
	{
	  if (tmp == add_routes) 
	    {
	      add_routes = add_routes->next;
	      free (tmp);
	      return 1;
	    }
	  else 
	    {
	      prev->next = tmp->next;
	      free (tmp);
	      return 1;
	    }
	}
      prev = tmp;
      tmp = tmp->next;
    }
  return 0;
}

int
olsr_delroute_remove_function(int (*function) (struct rt_entry*), olsr_u8_t type)
{
  struct export_route_entry *tmp, *prev = NULL /* Make compiler happy */;
  tmp = del_routes;
  while (tmp) 
    {
      if (function == tmp->function && type == tmp->type) 
	{
	  if (tmp == del_routes) 
	    {
	      del_routes = del_routes->next;
	      free (tmp);
	      return 1;
	    }
	  else 
	    {
	      prev->next = tmp->next;
	      free (tmp);
	      return 1; 
	    }
	}
      prev = tmp;
      tmp = tmp->next;
    }
  return 0;
}

void 
olsr_init_export_route(void)
{
  olsr_addroute_add_function(&olsr_ioctl_add_route, AF_INET);
  olsr_addroute_add_function(&olsr_ioctl_add_route6, AF_INET6);
  olsr_delroute_add_function(&olsr_ioctl_del_route, AF_INET);
  olsr_delroute_add_function(&olsr_ioctl_del_route6, AF_INET6);
}

int
olsr_export_add_route (struct rt_entry *rt) 
{
  int retval = 0;
  struct export_route_entry *tmp;
  for (tmp = add_routes; tmp; tmp = tmp->next)
    {
      if (tmp->type == AF_INET)
	retval = tmp->function(rt);
    }
  return retval;
}

int
olsr_export_add_route6 (struct rt_entry *rt) 
{
  int retval = 0;
  struct export_route_entry *tmp;
  for (tmp = add_routes; tmp; tmp = tmp->next)
    {
      if (tmp->type == AF_INET6)
	retval = tmp->function(rt);
    }
  return retval;
}

int
olsr_export_del_route (struct rt_entry *rt) 
{
  int retval = 0;
  struct export_route_entry *tmp;
  for (tmp = del_routes; tmp; tmp = tmp->next)
    {
      if (tmp->type == AF_INET)
	retval = tmp->function(rt);
    }
  return retval;
}

int
olsr_export_del_route6 (struct rt_entry *rt) 
{
  int retval = 0;
  struct export_route_entry *tmp;
  for (tmp = del_routes; tmp; tmp = tmp->next)
    {
      if (tmp->type == AF_INET6)
	retval = tmp->function(rt);
    }
  return retval;
}

/**
 *Deletes all OLSR routes
 *
 * This is extremely simple - Just increment the version of the
 * tree and then olsr_update_kernel_routes() will see
 * all routes in the tree as outdated and flush it.
 *
 *@return 1
 */
int
olsr_delete_all_kernel_routes(void)
{ 
  OLSR_PRINTF(1, "Deleting all routes...\n");

  olsr_bump_routingtree_version();
  olsr_update_kernel_routes();

  return 1;
}

/**
 * Enqueue a route on a kernel add/chg/del queue.
 */
static void
olsr_enqueue_rt(struct list_node *head_node, struct rt_entry *rt)
{
  const struct rt_nexthop *nh;

  /* if this node is already on some changelist we are done */
  if (list_node_on_list(&rt->rt_change_node)) {
    return;
  }

  rt->rt_change_node.data = rt;

  /*
   * For easier route dependency tracking we enqueue nexthop routes
   * at the head of the queue and non-nexthop routes at the tail of the queue.
   */
  nh = olsr_get_nh(rt);

  if (COMP_IP(&rt->rt_dst.prefix, &nh->gateway)) {
    list_add_after(head_node, &rt->rt_change_node);
  } else {
    list_add_before(head_node, &rt->rt_change_node);
  }
}

/**
 * Process a route from the kernel deletion list.
 *
 *@return nada
 */
static void
olsr_delete_kernel_route(struct rt_entry *rt)
{
  olsr_16_t error;		  

  if(!olsr_cnf->host_emul) {

    if(olsr_cnf->ip_version == AF_INET) {
      error = olsr_export_del_route(rt);
    } else {
      error = olsr_export_del_route6(rt);
    }

    if(error < 0) {
      const char * const err_msg = strerror(errno);

      OLSR_PRINTF(1, "KERN: ERROR deleting %s: %s\n",
                  olsr_rt_to_string(rt), err_msg);

      olsr_syslog(OLSR_LOG_ERR, "Delete route: %s", err_msg);

    }
  }
}

/**
 * Process a route from the kernel addition list.
 *
 *@return nada
 */
static void
olsr_add_kernel_route(struct rt_entry *rt)
{
  olsr_16_t error;		  

  if(!olsr_cnf->host_emul) {

    if(olsr_cnf->ip_version == AF_INET) {
      error = olsr_export_add_route(rt);
    } else {
      error = olsr_export_add_route6(rt);
    }
    
    if(error < 0) {
      const char * const err_msg = strerror(errno);
      OLSR_PRINTF(1, "KERN: ERROR adding %s: %s\n",
                  olsr_rtp_to_string(rt->rt_best), err_msg);

      olsr_syslog(OLSR_LOG_ERR, "Add route: %s", err_msg);
    } else {

      /* route addition has suceeded */

      /* save the nexthop in the route entry */
      rt->rt_nexthop = rt->rt_best->rtp_nexthop;
    }
  }
}

/**
 * process the kernel add list.
 * the routes are already ordered such that nexthop routes
 * are on the head of the queue.
 * nexthop routes need to be added first and therefore
 * the queue needs to be traversed from head to tail.
 */
static void
olsr_add_kernel_routes(struct list_node *head_node)
{
  struct rt_entry *rt;

  while (!list_is_empty(head_node)) {

    rt = head_node->next->data;
    olsr_add_kernel_route(rt);

    list_remove(&rt->rt_change_node);
  }
}

/**
 * process the kernel change list.
 * the routes are already ordered such that nexthop routes
 * are on the head of the queue.
 * non-nexthop routes need to be changed first and therefore
 * the queue needs to be traversed from tail to head.
 */
static void
olsr_chg_kernel_routes(struct list_node *head_node)
{
  struct rt_entry *rt;
  struct list_node *node;

  if (list_is_empty(head_node)) {
    return;
  }

  /*
   * First pass.
   * traverse from the end to the beginning of the list,
   * such that nexthop routes are deleted last.
   */
  for (node = head_node->prev; head_node != node; node = node->prev) {

    rt = node->data;
    olsr_delete_kernel_route(rt);
  }

  /*
   * Second pass.
   * Traverse from the beginning to the end of the list,
   * such that nexthop routes are added first.
   */
  while (!list_is_empty(head_node)) {

    rt = head_node->next->data;
    olsr_add_kernel_route(rt);

    list_remove(&rt->rt_change_node);
  }
}

/**
 * process the kernel delete list.
 * the routes are already ordered such that nexthop routes
 * are on the head of the queue.
 * non-nexthop routes need to be deleted first and therefore
 * the queue needs to be traversed from tail to head.
 */
static void
olsr_del_kernel_routes(struct list_node *head_node)
{
  struct rt_entry *rt;

  while (!list_is_empty(head_node)) {

    rt = head_node->prev->data;
    olsr_delete_kernel_route(rt);

    list_remove(&rt->rt_change_node);
    free(rt);
  }
}

/**
 * Check the version number of all route paths hanging off a route entry.
 * If a route does not match the current routing tree number, delete it.
 * Reset the best route pointer.
 */
static void
olsr_delete_outdated_routes(struct rt_entry *rt)
{
  struct rt_path *rtp;
  struct avl_node *rtp_tree_node, *next_rtp_tree_node;

  for (rtp_tree_node = avl_walk_first(&rt->rt_path_tree);
       rtp_tree_node;
       rtp_tree_node = next_rtp_tree_node) {

    /*
     * pre-fetch the next node before loosing context.
     */
    next_rtp_tree_node = avl_walk_next(rtp_tree_node);

    rtp = rtp_tree_node->data;

    /*
     * check the version number which gets incremented on every SPF run.
     * comparing for unequalness avoids handling version number wraps.
     */
    if (routingtree_version != rtp->rtp_version) {

      /* remove from the originator tree */
      avl_delete(&rt->rt_path_tree, rtp_tree_node);
      free(rtp);
    }
  }

  /* safety measure against dangling pointers */
  rt->rt_best = NULL;
}

/**
 * Walk all the routes, remove outdated routes and run
 * best path selection on the remaining set.
 * Finally compare the nexthop of the route head and the best
 * path and enqueue a add/chg operation.
 */
void
olsr_update_kernel_routes(void)
{
  struct rt_entry *rt;

  OLSR_PRINTF(3, "Updating kernel routes...\n");

  /* walk all routes in the RIB. */

  OLSR_FOR_ALL_RT_ENTRIES(rt) {

    /* eliminate first unused routes */
    olsr_delete_outdated_routes(rt);

    if (!rt->rt_path_tree.count) {

      /* oops, all routes are gone - flush the route head */
      avl_delete(&routingtree, rt_tree_node);

      olsr_enqueue_rt(&del_kernel_list, rt);
      continue;
    }

    /* run best route election */
    olsr_rt_best(rt);

    /* nexthop change ? */
    if (olsr_nh_change(&rt->rt_best->rtp_nexthop, &rt->rt_nexthop)) {

      if (0 > rt->rt_nexthop.iif_index) {

        /* fresh routes do have an interface index of -1. */
        olsr_enqueue_rt(&add_kernel_list, rt);
      } else { 

        /* this is a route change. */
        olsr_enqueue_rt(&chg_kernel_list, rt);
      }
    }
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);

  /* delete unreachable routes */
  olsr_del_kernel_routes(&del_kernel_list);

  /* route changes */
  olsr_chg_kernel_routes(&chg_kernel_list);

  /* route additions */
  olsr_add_kernel_routes(&add_kernel_list);

#if DEBUG
  olsr_print_routing_table(&routingtree);
#endif
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
