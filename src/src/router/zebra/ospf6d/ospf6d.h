/*
 * Copyright (C) 1999 Yasuhiro Ohara
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */

#ifndef OSPF6D_H
#define OSPF6D_H

#include <zebra.h>
#include "linklist.h"

#ifndef HEADER_DEPENDENCY
/* Include other stuffs */
#include "version.h"
#include "log.h"
#include "getopt.h"
#include "thread.h"
#include "command.h"
#include "memory.h"
#include "sockunion.h"
#include "if.h"
#include "prefix.h"
#include "stream.h"
#include "thread.h"
#include "filter.h"
#include "zclient.h"
#include "table.h"
#include "plist.h"

/* OSPF stuffs */
#include "ospf6_types.h"
#include "ospf6_prefix.h"
#include "ospf6_lsa.h"
#include "ospf6_lsdb.h"

#include "ospf6_message.h"
#include "ospf6_proto.h"
#include "ospf6_spf.h"
#include "ospf6_top.h"
#include "ospf6_area.h"
#include "ospf6_interface.h"
#include "ospf6_neighbor.h"
#include "ospf6_ism.h"
#include "ospf6_nsm.h"
#include "ospf6_redistribute.h"
#include "ospf6_route.h"
#include "ospf6_dbex.h"
#include "ospf6_network.h"
#include "ospf6_zebra.h"
#include "ospf6_dump.h"
#include "ospf6_routemap.h"
#endif /*HEADER_DEPENDENCY*/

#define HASHVAL 64
#define MAXIOVLIST 1024

#define OSPF6_DAEMON_VERSION    "0.9.2"

/* global variables */
extern char *progname;
extern int errno;
extern int daemon_mode;
extern struct thread_master *master;
extern list iflist;
extern list nexthoplist;
extern struct sockaddr_in6 allspfrouters6;
extern struct sockaddr_in6 alldrouters6;
extern int ospf6_sock;
extern struct ospf6 *ospf6;
extern char *recent_reason;

/* Default configuration file name for ospfd. */
#define OSPF6_DEFAULT_CONFIG       "ospf6d.conf"

/* Default port values. */
#define OSPF6_VTY_PORT             2606
#define OSPF6_VTYSH_PATH           "/tmp/.ospf6d"

#ifdef INRIA_IPV6
#ifndef IPV6_PKTINFO
#define IPV6_PKTINFO IPV6_RECVPKTINFO
#endif /* IPV6_PKTINFO */
#endif /* INRIA_IPV6 */

/* historycal for KAME */
#ifndef IPV6_JOIN_GROUP
#ifdef IPV6_ADD_MEMBERSHIP
#define IPV6_JOIN_GROUP IPV6_ADD_MEMBERSHIP
#endif /* IPV6_ADD_MEMBERSHIP. */
#ifdef IPV6_JOIN_MEMBERSHIP  /* I'm not sure this really exist. -- kunihiro. */
#define IPV6_JOIN_GROUP  IPV6_JOIN_MEMBERSHIP
#endif /* IPV6_JOIN_MEMBERSHIP. */
#endif

#ifndef IPV6_LEAVE_GROUP
#ifdef  IPV6_DROP_MEMBERSHIP
#define IPV6_LEAVE_GROUP IPV6_DROP_MEMBERSHIP
#endif
#endif

#define OSPF6_CMD_CHECK_RUNNING() \
  if (ospf6 == NULL) \
    { \
      vty_out (vty, "OSPFv3 is not running%s", VTY_NEWLINE); \
      return CMD_SUCCESS; \
    }

#define OSPF6_LEVEL_NONE      0
#define OSPF6_LEVEL_NEIGHBOR  1
#define OSPF6_LEVEL_INTERFACE 2
#define OSPF6_LEVEL_AREA      3
#define OSPF6_LEVEL_TOP       4
#define OSPF6_LEVEL_MAX       5


/* Function Prototypes */
void
ospf6_timeval_add (const struct timeval *t1, const struct timeval *t2,
                   struct timeval *result);
void
ospf6_timeval_sub (const struct timeval *t1, const struct timeval *t2,
                   struct timeval *result);
void
ospf6_timeval_div (const struct timeval *t1, u_int by,
                   struct timeval *result);
int
ospf6_timeval_cmp (const struct timeval *t1, const struct timeval *t2);
void
ospf6_timeval_add_equal (const struct timeval *t, struct timeval *result);
void
ospf6_timeval_sub_equal (const struct timeval *t, struct timeval *result);
void
ospf6_timeval_decode (const struct timeval *t, long *dayp, long *hourp,
                      long *minp, long *secp, long *msecp, long *usecp);
void
ospf6_timeval_string (struct timeval *tv, char *buf, int size);

void
ospf6_count_state (void *arg, int val, void *obj);

void ospf6_init ();
void ospf6_terminate ();

void ospf6_maxage_remover ();

#endif /* OSPF6D_H */

