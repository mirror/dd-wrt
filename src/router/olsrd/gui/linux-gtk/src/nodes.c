
/*
 * OLSR ad-hoc routing table management protocol GUI front-end
 * Copyright (C) 2003 Andreas Tonnesen (andreto@ifi.uio.no)
 *
 * This file is part of olsr.org.
 *
 * uolsrGUI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * uolsrGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsr.org; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "common.h"
#include "nodes.h"
#include <math.h>

void
init_nodes()
{

  nodes.next = &nodes;
  nodes.prev = &nodes;
}

/*
 *Insert a new node in the list
 *NB! The list is NOT checked for duplicates!!
 */
struct node *
insert_node(struct node *n, olsr_u8_t vtime)
{
  struct node *new_node;

  printf("Inserting node %s\n", ip_to_string((union olsr_ip_addr *)&n->addr));

  if ((new_node = malloc(sizeof(struct node))) == 0) {
    fprintf(stderr, "OUT OF MEMORY!\n");
    exit(1);
  }

  memcpy(new_node, n, sizeof(struct node));

  /* queue */
  nodes.next->prev = new_node;
  new_node->next = nodes.next;
  nodes.next = new_node;
  new_node->prev = &nodes;

  new_node->hna.next = &new_node->hna;
  new_node->hna.prev = &new_node->hna;
  new_node->mid.next = &new_node->mid;
  new_node->mid.prev = &new_node->mid;
  new_node->mpr.next = &new_node->mpr;
  new_node->mpr.prev = &new_node->mpr;

  update_timer_node(&n->addr, vtime);

  return new_node;
}

/**
 *Add a new node to the set
 */
int
add_node(union olsr_ip_addr *node, olsr_u8_t vtime)
{
  struct node new;
  struct node *tmp_nodes;
  struct timeval tmp_timer;
  double dbl_time;
  olsr_u32_t time_value;
  struct mid *tmp_mid;

  dbl_time = me_to_double(vtime);
  time_value = (olsr_u32_t) dbl_time *1000;

  tmp_timer.tv_sec = time_value / 1000;
  tmp_timer.tv_usec = (time_value - (tmp_timer.tv_sec * 1000)) * 1000;

  /* Check if node exists */
  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      //printf("updating timer for node %s\n", ip_to_string(node));
      //printf("Updatimng timer for: %s\n", ip_to_string(node));
      //printf("Secs: %d, usecs: %d\n", (int)tmp_timer.tv_sec, (int)tmp_timer.tv_usec);
      gettimeofday(&now, (struct timezone *)NULL);
      timeradd(&now, &tmp_timer, &tmp_nodes->timer);
      return 0;
    }
    /* Check MID */
    for (tmp_mid = tmp_nodes->mid.next; tmp_mid != &tmp_nodes->mid; tmp_mid = tmp_mid->next) {
      if (memcmp(&tmp_mid->alias, node, ipsize) == 0) {
        //printf("updating timer for node %s\n", ip_to_string(node));
        //printf("Updatimng timer for (MID): %s\n", ip_to_string(&tmp_nodes->addr));
        //printf("Secs: %d, usecs: %d\n", (int)tmp_timer.tv_sec, (int)tmp_timer.tv_usec);
        gettimeofday(&now, (struct timezone *)NULL);
        timeradd(&now, &tmp_timer, &tmp_nodes->timer);
        return 0;
      }
    }
  }

  /* New node */
  memset(&new, 0, sizeof(struct node));
  memcpy(&new.addr, node, ipsize);
  new.display = 1;
  printf("1:");
  insert_node(&new, vtime);
  update_nodes_list(&new);

  return 1;
}

int
update_timer_node(union olsr_ip_addr *node, olsr_u8_t vtime)
{
  struct node *tmp_nodes;
  struct timeval tmp_timer;
  double dbl_time;
  olsr_u32_t time_value;

  dbl_time = me_to_double(vtime);
  time_value = (olsr_u32_t) dbl_time *1000;

  tmp_timer.tv_sec = time_value / 1000;
  tmp_timer.tv_usec = (time_value - (tmp_timer.tv_sec * 1000)) * 1000;

  //printf("Updatimng timer for: %s\n", ip_to_string(node));
  //printf("Secs: %d, usecs: %d\n", (int)tmp_timer.tv_sec, (int)tmp_timer.tv_usec);

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      //printf("updating timer for node %s\n", ip_to_string(node));
      gettimeofday(&now, (struct timezone *)NULL);
      timeradd(&now, &tmp_timer, &tmp_nodes->timer);
      if (tmp_nodes->display)
        update_nodes_list(tmp_nodes);
      return 1;
    }
  }

  return 0;
}

/**
 *Updates the hold time for the mpr 'mpr' registered on
 *the node 'node'. Adds the mpr to the node if not already
 *registered.
 *@param node the node that has chosen the MPR
 *@param mpr the MPR chosen by the node
 *@return 0 if node was added, 1 if not
 */
int
update_timer_mpr(union olsr_ip_addr *node, union olsr_ip_addr *mpr, olsr_u8_t vtime)
{
  struct node *tmp_nodes;
  struct mpr *tmp_mpr;
  struct timeval tmp_timer;
  double dbl_time;
  olsr_u32_t time_value;

  dbl_time = me_to_double(vtime);
  time_value = (olsr_u32_t) dbl_time *1000;

  tmp_timer.tv_sec = time_value / 1000;
  tmp_timer.tv_usec = (time_value - (tmp_timer.tv_sec * 1000)) * 1000;

  //printf("Updatimng MPR timer for: %s\n", ip_to_string(node));
  //printf("Secs: %d, usecs: %d\n", (int)tmp_timer.tv_sec, (int)tmp_timer.tv_usec);

  //printf("Updatimng timer for: %s\n", ip_to_string(node));
  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      for (tmp_mpr = tmp_nodes->mpr.next; tmp_mpr != &tmp_nodes->mpr; tmp_mpr = tmp_mpr->next) {
        if (memcmp(&tmp_mpr->addr, mpr, ipsize) == 0) {
          //printf("updating timer for MPR %s ", ip_to_string(mpr));
          //printf("node %s\n", ip_to_string(node));
          gettimeofday(&now, (struct timezone *)NULL);
          timeradd(&now, &tmp_timer, &tmp_mpr->timer);
          return 1;
        }
      }
      /* Only add if parent is added */
      add_mpr(node, mpr, &tmp_timer);
      return 0;
    }
  }

  return 0;
}

int
add_mid_node(union olsr_ip_addr *node, union olsr_ip_addr *alias, olsr_u8_t vtime)
{

  struct node *tmp_nodes;
  struct mid *tmp_mid;
  struct node new, *inserted;

  //printf("MID_add: %s\n", ip_to_string(alias));

  //update_timer_node(node, vtime);

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      for (tmp_mid = tmp_nodes->mid.next; tmp_mid != &tmp_nodes->mid; tmp_mid = tmp_mid->next) {
        if (memcmp(&tmp_mid->alias, alias, ipsize) == 0)
          return 0;
      }

      /* we didn't find the address */
      printf("(1)NEW MID %s ", ip_to_string(alias));
      printf("ADDED FOR %s\n", ip_to_string(node));
      if ((tmp_mid = malloc(sizeof(struct mid))) == 0) {
        fprintf(stderr, "OUT OF MEMORY\n");
        exit(1);
      }

      memcpy(&tmp_mid->alias, alias, ipsize);

      tmp_nodes->mid.next->prev = tmp_mid;
      tmp_mid->next = tmp_nodes->mid.next;
      tmp_nodes->mid.next = tmp_mid;
      tmp_mid->prev = &tmp_nodes->mid;

      remove_node_addr(alias);  // Remove if already registered as a node

      update_nodes_list(tmp_nodes);
      return 1;

    }
  }

  /*New node */

  printf("ADDING NEW NODE %s FROM MID...\n", ip_to_string(node));
  /* We don't know wery much... */
  memset(&new, 0, sizeof(struct node));
  memcpy(&new.addr, node, ipsize);
  inserted = insert_node(&new, vtime);

  if ((tmp_mid = malloc(sizeof(struct mid))) == 0) {
    fprintf(stderr, "OUT OF MEMORY!\n");
    exit(1);
  }

  memcpy(&tmp_mid->alias, alias, ipsize);

  tmp_mid->next = &inserted->mid;
  tmp_mid->prev = &inserted->mid;
  inserted->mid.next = tmp_mid;
  inserted->mid.prev = tmp_mid;

  update_nodes_list(inserted);

  return 2;
}

int
add_hna_node(union olsr_ip_addr *node, union olsr_ip_addr *net, union olsr_ip_addr *mask, olsr_u8_t vtime)
{

  struct node *tmp_nodes;
  struct hna *tmp_hna;
  struct node new, *inserted;

  //printf("HNA: %s\n", ip_to_string(&net));

  update_timer_node(node, vtime);

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      for (tmp_hna = tmp_nodes->hna.next; tmp_hna != &tmp_nodes->hna; tmp_hna = tmp_hna->next) {
        if ((memcmp(&tmp_hna->net, net, ipsize) == 0) && (memcmp(&tmp_hna->mask, mask, ipsize) == 0))
          return 0;
      }

      //printf("NEW HNA ADDED FOR %s ", ip_to_string(node));
      //printf("net: %s \n", ip_to_string(&net));
      /* we didn't find the address */
      if ((tmp_hna = malloc(sizeof(struct hna))) == 0) {
        fprintf(stderr, "OUT OF MEMORY\n");
        exit(1);
      }

      memcpy(&tmp_hna->net, net, ipsize);
      memcpy(&tmp_hna->mask, mask, ipsize);

      /* queue */
      tmp_nodes->hna.next->prev = tmp_hna;
      tmp_hna->next = tmp_nodes->hna.next;
      tmp_nodes->hna.next = tmp_hna;
      tmp_hna->prev = &tmp_nodes->hna;

      update_nodes_list(tmp_nodes);
      return 1;
    }
  }

  printf("ADDING NEW NODE %s FROM HNA...\n", ip_to_string(node));
  /* We don't know wery much... */
  memset(&new, 0, sizeof(struct node));
  memcpy(&new.addr, node, ipsize);
  inserted = insert_node(&new, vtime);

  if ((tmp_hna = malloc(sizeof(struct hna))) == 0) {
    fprintf(stderr, "OUT OF MEMORY!\n");
    exit(1);
  }

  memcpy(&tmp_hna->net, net, ipsize);
  memcpy(&tmp_hna->mask, mask, ipsize);

  tmp_hna->next = &inserted->hna;
  tmp_hna->prev = &inserted->hna;
  inserted->hna.next = tmp_hna;
  inserted->hna.prev = tmp_hna;

  update_nodes_list(inserted);

  return 2;
}

/**
 *Add the MPR mpr to the node nodes selected MPRs.
 *Nodes are NOT added if they are not yet registered!
 *
 *@param node the node that has chosen an MPR
 *@param mpr the MPR choosen by node
 *@return negative if node already registered or node not found
 */
int
add_mpr(union olsr_ip_addr *node, union olsr_ip_addr *mpr, struct timeval *tmp_timer)
{

  struct node *tmp_nodes;
  struct mpr *mprs;
  struct mpr *tmp_mpr;

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      for (mprs = tmp_nodes->mpr.next; mprs != &tmp_nodes->mpr; mprs = mprs->next) {
        if (memcmp(&mprs->addr, mpr, ipsize) == 0)
          return 0;
      }

      //printf("Adding MPR %s to ", ip_to_string(mpr));
      //printf("%s\n", ip_to_string(node));
      /* Add mpr */

      if ((tmp_mpr = malloc(sizeof(struct mpr))) == 0) {
        fprintf(stderr, "OUT OF MEMORY\n");
        exit(1);
      }

      memcpy(&tmp_mpr->addr, mpr, ipsize);

      gettimeofday(&now, (struct timezone *)NULL);
      timeradd(&now, tmp_timer, &tmp_mpr->timer);

      /* queue */
      tmp_nodes->mpr.next->prev = tmp_mpr;
      tmp_mpr->next = tmp_nodes->mpr.next;
      tmp_nodes->mpr.next = tmp_mpr;
      tmp_mpr->prev = &tmp_nodes->mpr;

      update_nodes_list(tmp_nodes);
      return 1;

    }
  }

  return 1;
}

int
remove_node(struct node *node)
{
  struct hna *tmp_hna, *tmp_hna2;
  struct mid *tmp_mid, *tmp_mid2;
  struct mpr *tmp_mpr, *tmp_mpr2;

  printf("Remove node %s\n", ip_to_string(&node->addr));

  tmp_hna = node->hna.next;
  while (tmp_hna != &node->hna) {
    tmp_hna2 = tmp_hna;
    tmp_hna = tmp_hna->next;
    free(tmp_hna2);
  }
  tmp_mpr = node->mpr.next;
  while (tmp_mpr != &node->mpr) {
    tmp_mpr2 = tmp_mpr;
    tmp_mpr = tmp_mpr->next;
    free(tmp_mpr2);
  }
  tmp_mid = node->mid.next;
  while (tmp_mid != &node->mid) {
    tmp_mid2 = tmp_mid;
    tmp_mid = tmp_mid->next;
    free(tmp_mid2);
  }

  /* Gemove form GUI */
  remove_nodes_list(&node->addr);

  /* Dequeue */
  node->prev->next = node->next;
  node->next->prev = node->prev;

  free(node);

  return 1;
}

/*
 * Remove based on address
 */

int
remove_node_addr(union olsr_ip_addr *node)
{
  struct node *tmp_nodes;
  struct hna *tmp_hna, *tmp_hna2;
  struct mid *tmp_mid, *tmp_mid2;
  struct mpr *tmp_mpr, *tmp_mpr2;

  printf("Remove node %s\n", ip_to_string(node));

  tmp_nodes = nodes.next;

  while (tmp_nodes != &nodes) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      printf("(2)Deleting node %s\n", ip_to_string((union olsr_ip_addr *)&tmp_nodes->addr));

      tmp_hna = tmp_nodes->hna.next;
      while (tmp_hna != &tmp_nodes->hna) {
        tmp_hna2 = tmp_hna;
        tmp_hna = tmp_hna->next;
        free(tmp_hna2);
      }
      tmp_mpr = tmp_nodes->mpr.next;
      while (tmp_mpr != &tmp_nodes->mpr) {
        tmp_mpr2 = tmp_mpr;
        tmp_mpr = tmp_mpr->next;
        free(tmp_mpr2);
      }
      tmp_mid = tmp_nodes->mid.next;
      while (tmp_mid != &tmp_nodes->mid) {
        tmp_mid2 = tmp_mid;
        tmp_mid = tmp_mid->next;
        free(tmp_mid2);
      }

      /* Gemove form GUI */
      remove_nodes_list(&tmp_nodes->addr);

      /* Dequeue */
      tmp_nodes->prev->next = tmp_nodes->next;
      tmp_nodes->next->prev = tmp_nodes->prev;

      free(tmp_nodes);

      return 1;
    }

    tmp_nodes = tmp_nodes->next;
  }

  return 0;
}

struct node *
find_node(char *ip)
{
  struct node *tmp_nodes;

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (strcmp(ip_to_string((union olsr_ip_addr *)&tmp_nodes->addr), ip) == 0)
      return tmp_nodes;
  }

  return NULL;
}

struct node *
find_node_t(union olsr_ip_addr *ip)
{
  struct node *tmp_nodes;

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, ip, ipsize) == 0)
      return tmp_nodes;
  }

  return 0;
}

/*
 *Remove timed out nodes
 */
gint
time_out_nodes(gpointer data)
{
  struct node *tmp_nodes;
  struct node *node_to_delete;

  /* Wait before starting timing out */
  if (timeouts) {
    timeouts--;
    //printf("Waiting...\n");
    return 1;
  }
  //printf("Timing out nodes...\n");
  gettimeofday(&now, (struct timezone *)NULL);

  tmp_nodes = nodes.next;

  while (tmp_nodes != &nodes) {
    //printf("%s: %6d < %6d\n", ip_to_string(&tmp_nodes->addr), tmp_nodes->timer.tv_sec, now.tv_sec);
    if (timercmp(&tmp_nodes->timer, &now, <)) {
      printf("Node %s timed out...\n", ip_to_string((union olsr_ip_addr *)&tmp_nodes->addr));
      node_to_delete = tmp_nodes;

      tmp_nodes = tmp_nodes->next;

      remove_nodes_list(&node_to_delete->addr);
      remove_node(node_to_delete);
    } else
      tmp_nodes = tmp_nodes->next;
  }

  return 1;
}

/**
 *Timeout MPRs for a given node. Only called when user
 *is to see the registered MPRs of the node.
 *@param node the node whom MPRs should be timed out
 *@return negative if node not found
 */
int
time_out_mprs(union olsr_ip_addr *node)
{

  struct node *tmp_nodes;
  struct mpr *mpr_to_delete;
  struct mpr *tmp_mpr;

  gettimeofday(&now, (struct timezone *)NULL);

  /* W A R N I N G !
   *
   * THIS ALGORITHM HAS NOT BEEN TESTED PROPERLY!!!!!!
   * -Andreas
   */

  for (tmp_nodes = nodes.next; tmp_nodes != &nodes; tmp_nodes = tmp_nodes->next) {
    if (memcmp(&tmp_nodes->addr, node, ipsize) == 0) {
      tmp_mpr = tmp_nodes->mpr.next;

      while (tmp_mpr != &tmp_nodes->mpr) {
        if (timercmp(&tmp_mpr->timer, &now, <)) {
          printf("MPR %s OF NODE ", ip_to_string((union olsr_ip_addr *)&tmp_mpr->addr));
          printf("%s TIMIED OUT ", ip_to_string((union olsr_ip_addr *)&tmp_nodes->addr));
          fflush(stdout);

          mpr_to_delete = tmp_mpr;
          tmp_mpr = tmp_mpr->next;

          /* Dequeue */
          mpr_to_delete->next->prev = mpr_to_delete->prev;
          mpr_to_delete->prev->next = mpr_to_delete->next;
          /* Delete */
          free(mpr_to_delete);
        } else
          tmp_mpr = tmp_mpr->next;
      }

      return 1;
    }
  }

  return 0;
}

void
init_timer(olsr_u32_t time_value, struct timeval *hold_timer)
{
  olsr_u16_t time_value_sec = 0;
  olsr_u16_t time_value_msec = 0;

  time_value_sec = time_value / 1000;
  time_value_msec = time_value - (time_value_sec * 1000);

  hold_timer->tv_sec = time_value_sec;
  hold_timer->tv_usec = time_value_msec * 1000;

}

/**
 *Function that converts a mantissa/exponent 8bit value back
 *to double as described in RFC3626:
 *
 * value = C*(1+a/16)*2^b [in seconds]
 *
 *  where a is the integer represented by the four highest bits of the
 *  field and b the integer represented by the four lowest bits of the
 *  field.
 *
 *@param me the 8 bit mantissa/exponen value
 *
 *@return a double value
 */
double
me_to_double(olsr_u8_t me)
{
  int a = me >> 4;
  int b = me - a * 16;
  return (double)(VTIME_SCALE_FACTOR * (1 + (double)a / 16) * (double)pow(2, b));
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
