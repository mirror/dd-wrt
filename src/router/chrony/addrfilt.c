/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997,1998,1999,2000,2001,2002,2005
 * Copyright (C) Miroslav Lichvar  2009, 2015
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  This module provides a set of routines for checking IP addresses
  against a set of rules and deciding whether they are allowed or
  disallowed.

  */

#include "config.h"

#include "sysincl.h"

#include "addrfilt.h"
#include "memory.h"

/* Define the number of bits which are stripped off per level of
   indirection in the tables */
#define NBITS 4

/* Define the table size */
#define TABLE_SIZE (1UL<<NBITS)

typedef enum {DENY, ALLOW, AS_PARENT} State;

typedef struct _TableNode {
  State state;
  struct _TableNode *extended;
} TableNode;

struct ADF_AuthTableInst {
  TableNode base4;      /* IPv4 node */
  TableNode base6;      /* IPv6 node */
};

/* ================================================== */

static void
split_ip6(IPAddr *ip, uint32_t *dst)
{
  int i;

  for (i = 0; i < 4; i++)
    dst[i] = (uint32_t)ip->addr.in6[i * 4 + 0] << 24 |
             ip->addr.in6[i * 4 + 1] << 16 |
             ip->addr.in6[i * 4 + 2] << 8 |
             ip->addr.in6[i * 4 + 3];
}

/* ================================================== */

inline static uint32_t
get_subnet(uint32_t *addr, unsigned int where)
{
  int off;

  off = where / 32;
  where %= 32;

  return (addr[off] >> (32 - NBITS - where)) & ((1UL << NBITS) - 1);
}

/* ================================================== */

ADF_AuthTable
ADF_CreateTable(void)
{
  ADF_AuthTable result;
  result = MallocNew(struct ADF_AuthTableInst);

  /* Default is that nothing is allowed */
  result->base4.state = DENY;
  result->base4.extended = NULL;
  result->base6.state = DENY;
  result->base6.extended = NULL;

  return result;
}

/* ================================================== */
/* This function deletes all definitions of child nodes, in effect
   pruning a whole subnet definition back to a single parent
   record. */
static void
close_node(TableNode *node)
{
  int i;
  TableNode *child_node;

  if (node->extended != NULL) {
    for (i=0; i<TABLE_SIZE; i++) {
      child_node = &(node->extended[i]);
      close_node(child_node);
    }
    Free(node->extended);
    node->extended = NULL;
  }
}


/* ================================================== */
/* Allocate the extension field in a node, and set all the children's
   states to default to that of the node being extended */

static void
open_node(TableNode *node)
{
  int i;
  TableNode *child_node;

  if (node->extended == NULL) {

    node->extended = MallocArray(struct _TableNode, TABLE_SIZE);

    for (i=0; i<TABLE_SIZE; i++) {
      child_node = &(node->extended[i]);
      child_node->state = AS_PARENT;
      child_node->extended = NULL;
    }
  }
}

/* ================================================== */

static ADF_Status
set_subnet(TableNode *start_node,
           uint32_t *ip,
           int ip_len,
           int subnet_bits,
           State new_state,
           int delete_children)
{
  int bits_to_go, bits_consumed;
  uint32_t subnet;
  TableNode *node;

  bits_consumed = 0;
  bits_to_go = subnet_bits;
  node = start_node;

  if ((subnet_bits < 0) ||
      (subnet_bits > 32 * ip_len)) {

    return ADF_BADSUBNET;

  } else {

    if ((bits_to_go & (NBITS-1)) == 0) {
    
      while (bits_to_go > 0) {
        subnet = get_subnet(ip, bits_consumed);
        if (!(node->extended)) {
          open_node(node);
        }
        node = &(node->extended[subnet]);
        bits_to_go -= NBITS;
        bits_consumed += NBITS;
      }

      if (delete_children) {
        close_node(node);
      }
      node->state = new_state;

    } else { /* Have to set multiple entries */
      int N, i, j;
      TableNode *this_node;

      while (bits_to_go >= NBITS) {
        subnet = get_subnet(ip, bits_consumed);
        if (!(node->extended)) {
          open_node(node);
        }
        node = &(node->extended[subnet]);
        bits_to_go -= NBITS;
        bits_consumed += NBITS;
      }

      /* How many subnet entries to set : 1->8, 2->4, 3->2 */
      N = 1 << (NBITS-bits_to_go);

      subnet = get_subnet(ip, bits_consumed) & ~(N - 1);
      assert(subnet + N <= TABLE_SIZE);

      if (!(node->extended)) {
        open_node(node);
      }
      
      for (i=subnet, j=0; j<N; i++, j++) {
        this_node = &(node->extended[i]);
        if (delete_children) {
          close_node(this_node);
        }
        this_node->state = new_state;
      }
    }
    
    return ADF_SUCCESS;
  }
  
}

/* ================================================== */

static ADF_Status
set_subnet_(ADF_AuthTable table,
           IPAddr *ip_addr,
           int subnet_bits,
           State new_state,
           int delete_children)
{
  uint32_t ip6[4];

  switch (ip_addr->family) {
    case IPADDR_INET4:
      return set_subnet(&table->base4, &ip_addr->addr.in4, 1, subnet_bits, new_state, delete_children);
    case IPADDR_INET6:
      split_ip6(ip_addr, ip6);
      return set_subnet(&table->base6, ip6, 4, subnet_bits, new_state, delete_children);
    case IPADDR_UNSPEC:
      /* Apply to both, subnet_bits has to be 0 */
      if (subnet_bits != 0)
        return ADF_BADSUBNET;
      memset(ip6, 0, sizeof (ip6));
      if (set_subnet(&table->base4, ip6, 1, 0, new_state, delete_children) == ADF_SUCCESS &&
          set_subnet(&table->base6, ip6, 4, 0, new_state, delete_children) == ADF_SUCCESS)
        return ADF_SUCCESS;
      break;
  }

  return ADF_BADSUBNET;
}

ADF_Status
ADF_Allow(ADF_AuthTable table,
          IPAddr *ip,
          int subnet_bits)
{
  return set_subnet_(table, ip, subnet_bits, ALLOW, 0);
}

/* ================================================== */


ADF_Status
ADF_AllowAll(ADF_AuthTable table,
             IPAddr *ip,
             int subnet_bits)
{
  return set_subnet_(table, ip, subnet_bits, ALLOW, 1);
}

/* ================================================== */

ADF_Status
ADF_Deny(ADF_AuthTable table,
         IPAddr *ip,
         int subnet_bits)
{
  return set_subnet_(table, ip, subnet_bits, DENY, 0);
}

/* ================================================== */

ADF_Status
ADF_DenyAll(ADF_AuthTable table,
            IPAddr *ip,
            int subnet_bits)
{
  return set_subnet_(table, ip, subnet_bits, DENY, 1);
}

/* ================================================== */

void
ADF_DestroyTable(ADF_AuthTable table)
{
  close_node(&table->base4);
  close_node(&table->base6);
  Free(table);
}

/* ================================================== */

static int
check_ip_in_node(TableNode *start_node, uint32_t *ip)
{
  uint32_t subnet;
  int bits_consumed = 0;
  int result = 0;
  int finished = 0;
  TableNode *node;
  State state=DENY;

  node = start_node;

  do {
    if (node->state != AS_PARENT) {
      state = node->state;
    }
    if (node->extended) {
      subnet = get_subnet(ip, bits_consumed);
      node = &(node->extended[subnet]);
      bits_consumed += NBITS;
    } else {
      /* Make decision on this node */
      finished = 1;
    }
  } while (!finished);

  switch (state) {
    case ALLOW:
      result = 1;
      break;
    case DENY:
      result = 0;
      break;
    case AS_PARENT:
      assert(0);
      break;
  }

  return result;
}


/* ================================================== */

int
ADF_IsAllowed(ADF_AuthTable table,
              IPAddr *ip_addr)
{
  uint32_t ip6[4];

  switch (ip_addr->family) {
    case IPADDR_INET4:
      return check_ip_in_node(&table->base4, &ip_addr->addr.in4);
    case IPADDR_INET6:
      split_ip6(ip_addr, ip6);
      return check_ip_in_node(&table->base6, ip6);
  }

  return 0;
}

/* ================================================== */

static int
is_any_allowed(TableNode *node, State parent)
{
  State state;
  int i;

  state = node->state != AS_PARENT ? node->state : parent;
  assert(state != AS_PARENT);

  if (node->extended) {
    for (i = 0; i < TABLE_SIZE; i++) {
      if (is_any_allowed(&node->extended[i], state))
        return 1;
    }
  } else if (state == ALLOW) {
    return 1;
  }

  return 0;
}

/* ================================================== */

int
ADF_IsAnyAllowed(ADF_AuthTable table, int family)
{
  switch (family) {
    case IPADDR_INET4:
      return is_any_allowed(&table->base4, AS_PARENT);
    case IPADDR_INET6:
      return is_any_allowed(&table->base6, AS_PARENT);
    default:
      return 0;
  }
}
