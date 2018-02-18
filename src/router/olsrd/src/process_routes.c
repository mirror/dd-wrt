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

#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "kernel_routes.h"
#include "common/avl.h"
#include "net_olsr.h"
#include "tc_set.h"
#include "olsr_cookie.h"
#include "olsr_niit.h"

#ifdef _WIN32
char *StrError(unsigned int ErrNo);
#undef strerror
#define strerror(x) StrError(x)
#endif /* _WIN32 */

static struct list_node chg_kernel_list;

/**
 *
 * Calculate the kernel route flags.
 * Called before enqueuing a change/delete operation
 *
 */
uint8_t
olsr_rt_flags(const struct rt_entry *rt, int add)
{
  const struct rt_nexthop *nh;
  uint8_t flags = RTF_UP;

  /* destination is host */
  if (rt->rt_dst.prefix_len == olsr_cnf->maxplen) {
    flags |= RTF_HOST;
  }

  if (add) nh = olsr_get_nh(rt);
  else nh = &rt->rt_nexthop;

  if (!ipequal(&rt->rt_dst.prefix, &nh->gateway)) {
    flags |= RTF_GATEWAY;
  }

  return flags;
}

export_route_function olsr_addroute_function;
export_route_function olsr_addroute6_function;
export_route_function olsr_delroute_function;
export_route_function olsr_delroute6_function;

void
olsr_init_export_route(void)
{
  /* the add/chg/del kernel queues */
  //list_head_init(&add_kernel_list);
  list_head_init(&chg_kernel_list);

  olsr_addroute_function = olsr_ioctl_add_route;
  olsr_addroute6_function = olsr_ioctl_add_route6;
  olsr_delroute_function = olsr_ioctl_del_route;
  olsr_delroute6_function = olsr_ioctl_del_route6;
}

/**
 * Delete all OLSR routes.
 *
 * This is extremely simple - Just increment the version of the
 * tree and then olsr_update_rib_routes() will see all routes in the tree
 * as outdated and olsr_update_kernel_routes() will finally flush it.
 *
 */
void
olsr_delete_all_kernel_routes(void)
{
  OLSR_PRINTF(1, "Deleting all routes...\n");

  olsr_bump_routingtree_version();
  olsr_update_rib_routes();
  olsr_update_kernel_routes();
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

  /*
   * For easier route dependency tracking we enqueue nexthop routes
   * at the head of the queue and non-nexthop routes at the tail of the queue.
   */
  nh = olsr_get_nh(rt);

  if (ipequal(&rt->rt_dst.prefix, &nh->gateway)) {
    list_add_after(head_node, &rt->rt_change_node);
  } else {
    list_add_before(head_node, &rt->rt_change_node);
  }
}

/**
 * Process a route from the kernel deletion list.
 *
 *@return -1 on error, else 0
 */
static int
olsr_delete_kernel_route(struct rt_entry *rt)
{
  if (rt->rt_metric.hops > 1) {
    /* multihop route */
    if (ip_is_linklocal(&rt->rt_dst.prefix)) {
      /* do not delete a route with a LL IP as a destination */
      return 0;
    }
  }

  if (!olsr_cnf->host_emul) {
    int16_t error = olsr_cnf->ip_version == AF_INET ? olsr_delroute_function(rt) : olsr_delroute6_function(rt);

    if (error != 0) {
      const char *const err_msg = strerror(errno);
      const char *const routestr = olsr_rt_to_string(rt);
      OLSR_PRINTF(1, "KERN: ERROR deleting %s: %s\n", routestr, err_msg);

      olsr_syslog(OLSR_LOG_ERR, "Delete route %s: %s", routestr, err_msg);
      return -1;
    }
#ifdef __linux__
    /* call NIIT handler (always)*/
    if (olsr_cnf->use_niit) {
      olsr_niit_handle_route(rt, false);
    }
#endif /* __linux__ */
  }
  return 0;
}

/**
 * Process a route from the kernel addition list.
 *
 *@return nada
 */
static void
olsr_add_kernel_route(struct rt_entry *rt)
{
  if (rt->rt_best->rtp_metric.hops > 1) {
    /* multihop route */
    if (ip_is_linklocal(&rt->rt_best->rtp_dst.prefix)) {
      /* do not create a route with a LL IP as a destination */
      return;
    }
  }
  if (!olsr_cnf->host_emul) {
    int16_t error = (olsr_cnf->ip_version == AF_INET) ? olsr_addroute_function(rt) : olsr_addroute6_function(rt);

    if (error != 0) {
      const char *const err_msg = strerror(errno);
      const char *const routestr = olsr_rtp_to_string(rt->rt_best);
      OLSR_PRINTF(1, "KERN: ERROR adding %s: %s\n", routestr, err_msg);

      olsr_syslog(OLSR_LOG_ERR, "Add route %s: %s", routestr, err_msg);
    } else {
      /* route addition has suceeded */

      /* save the nexthop and metric in the route entry */
      rt->rt_nexthop = rt->rt_best->rtp_nexthop;
      rt->rt_metric = rt->rt_best->rtp_metric;

#ifdef __linux__
      /* call NIIT handler */
      if (olsr_cnf->use_niit) {
        olsr_niit_handle_route(rt, true);
      }
#endif /* __linux__ */
    }
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

  if (list_is_empty(head_node)) {
    return;
  }

  /*
   * Traverse from the beginning to the end of the list,
   * such that nexthop routes are added first.
   */
  while (!list_is_empty(head_node)) {
    rt = changelist2rt(head_node->next);

#ifdef __linux__
    /*
    *   actively deleting routes is not necessary as we use (NLM_F_CREATE | NLM_F_REPLACE) with linux
    *        (i.e. new routes simply overwrite the old ones in kernel)
    *   BUT: We still have to actively delete routes if fib_metric != FLAT or we run on ipv6.
    *        As NLM_F_REPLACE is not supported with IPv6, or simply of no use with varying route metrics.
    *        We also actively delete routes if custom route functions are in place. (e.g. quagga plugin)
    */
    if (((olsr_cnf->ip_version != AF_INET ) || (olsr_cnf->fib_metric != FIBM_FLAT)
         || (olsr_addroute_function != olsr_ioctl_add_route) || (olsr_addroute6_function != olsr_ioctl_add_route6)
         || (olsr_delroute_function != olsr_ioctl_del_route) || (olsr_delroute6_function != olsr_ioctl_del_route6))
        && (rt->rt_nexthop.iif_index > -1)) {
      olsr_delete_kernel_route(rt);
    }
#else /* __linux__ */
    /*no rtnetlink we have to delete routes*/
    if (rt->rt_nexthop.iif_index > -1) olsr_delete_kernel_route(rt);
#endif /* __linux__ */

    olsr_add_kernel_route(rt);

    list_remove(&rt->rt_change_node);
  }
}

/**
 * Check the version number of all route paths hanging off a route entry.
 * If a route does not match the current routing tree number, remove it
 * from the global originator tree for that rt_entry.
 * Reset the best route pointer.
 */
static void
olsr_delete_outdated_routes(struct rt_entry *rt)
{
  struct rt_path *rtp;
  struct avl_node *rtp_tree_node, *next_rtp_tree_node;

  for (rtp_tree_node = avl_walk_first(&rt->rt_path_tree); rtp_tree_node != NULL; rtp_tree_node = next_rtp_tree_node) {
    /*
     * pre-fetch the next node before loosing context.
     */
    next_rtp_tree_node = avl_walk_next(rtp_tree_node);

    rtp = rtp_tree2rtp(rtp_tree_node);

    /*
     * check the version number which gets incremented on every SPF run.
     * comparing for unequalness avoids handling version number wraps.
     */
    if (routingtree_version != rtp->rtp_version) {
      /* remove from the originator tree */
      avl_delete(&rt->rt_path_tree, rtp_tree_node);
      rtp->rtp_rt = NULL;

      if (rt->rt_best == rtp) {
        rt->rt_best = NULL;
      }
    }
  }
}

/**
 * Walk all the routes, remove outdated routes and run
 * best path selection on the remaining set.
 * Finally compare the nexthop of the route head and the best
 * path and enqueue an add/chg operation.
 */
void
olsr_update_rib_routes(void)
{
  struct rt_entry *rt;

  OLSR_PRINTF(3, "Updating kernel routes...\n");

  /* walk all routes in the RIB. */

  OLSR_FOR_ALL_RT_ENTRIES(rt) {

    /* eliminate first unused routes */
    olsr_delete_outdated_routes(rt);

    if (!rt->rt_path_tree.count) {

      /* oops, all routes are gone - flush the route head */
  
      if (olsr_delete_kernel_route(rt) == 0) {
        /*only remove if deletion was successful*/
        avl_delete(&routingtree, &rt->rt_tree_node);
        olsr_cookie_free(rt_mem_cookie, rt);
      }

      continue;
    }

    /* run best route election */
    olsr_rt_best(rt);

    /* nexthop or hopcount change ? */
    if (olsr_nh_change(&rt->rt_best->rtp_nexthop, &rt->rt_nexthop)
        || (FIBM_CORRECT == olsr_cnf->fib_metric && olsr_hopcount_change(&rt->rt_best->rtp_metric, &rt->rt_metric))) {

        /* this is a route add or change. */
        olsr_enqueue_rt(&chg_kernel_list, rt);
    }
  }
  OLSR_FOR_ALL_RT_ENTRIES_END(rt);
}

void
olsr_delete_interface_routes(int if_index) {
  struct rt_entry *rt;
  bool triggerUpdate = false;

  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    bool mightTrigger = false;
    struct rt_path *rtp;
    struct avl_node *rtp_tree_node, *next_rtp_tree_node;

    /* run through all routing paths of route */
    for (rtp_tree_node = avl_walk_first(&rt->rt_path_tree); rtp_tree_node != NULL; rtp_tree_node = next_rtp_tree_node) {
      /*
       * pre-fetch the next node before loosing context.
       */
      next_rtp_tree_node = avl_walk_next(rtp_tree_node);

      rtp = rtp_tree2rtp(rtp_tree_node);

      /* nexthop use lost interface ? */
      if (rtp->rtp_nexthop.iif_index == if_index) {
        /* remove from the originator tree */
        avl_delete(&rt->rt_path_tree, rtp_tree_node);
        rtp->rtp_rt = NULL;

        if (rt->rt_best == rtp) {
          rt->rt_best = NULL;
          mightTrigger = true;
        }
      }
    }

    if (mightTrigger) {
      if (!rt->rt_path_tree.count) {
        /* oops, all routes are gone - flush the route head */
        avl_delete(&routingtree, rt_tree_node);

        /* do not dequeue route because they are already gone */
      }
      triggerUpdate = true;
    }
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt)

  /* trigger route update if necessary */
  if (triggerUpdate) {
    olsr_update_rib_routes();
    olsr_update_kernel_routes();
  }
}

/**
 * Propagate the accumulated changes from the last rib update to the kernel.
 */
void
olsr_update_kernel_routes(void)
{
  /* route changes */
  olsr_chg_kernel_routes(&chg_kernel_list);

#if defined DEBUG && DEBUG
  olsr_print_routing_table(&routingtree);
#endif /* defined DEBUG && DEBUG */
}

void
olsr_force_kernelroutes_refresh(void) {
  struct rt_entry *rt;

  /* enqueue all existing routes for a rewrite */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    olsr_enqueue_rt(&chg_kernel_list, rt);
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt)

  /* trigger kernel route refresh */
  olsr_chg_kernel_routes(&chg_kernel_list);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
