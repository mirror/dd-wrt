/*
 * Logging function
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

#ifndef OSPF6_DUMP_H
#define OSPF6_DUMP_H

/* Strings for logging */
extern char   *ifs_name[];
extern char   *nbs_name[];

/* Function Prototypes */
void ospf6_log_init ();

/* new */

void ospf6_debug_init ();

enum ospf6_dump_type
{
  OSPF6_DUMP_HELLO,
  OSPF6_DUMP_DBDESC,
  OSPF6_DUMP_LSREQ,
  OSPF6_DUMP_LSUPDATE,
  OSPF6_DUMP_LSACK,
  OSPF6_DUMP_NEIGHBOR,
  OSPF6_DUMP_INTERFACE,
  OSPF6_DUMP_AREA,
  OSPF6_DUMP_LSA,
  OSPF6_DUMP_ZEBRA,
  OSPF6_DUMP_CONFIG,
  OSPF6_DUMP_DBEX,
  OSPF6_DUMP_SPF,
  OSPF6_DUMP_ROUTE,
  OSPF6_DUMP_LSDB,
  OSPF6_DUMP_REDISTRIBUTE,
  OSPF6_DUMP_MAX
};

struct _ospf6_dump
{
  int dump;
  char *string;
};

extern struct _ospf6_dump ospf6_dump[];

#define IS_OSPF6_DUMP_HELLO        (ospf6_dump[OSPF6_DUMP_HELLO].dump)
#define IS_OSPF6_DUMP_DBDESC       (ospf6_dump[OSPF6_DUMP_DBDESC].dump)
#define IS_OSPF6_DUMP_LSREQ        (ospf6_dump[OSPF6_DUMP_LSREQ].dump)
#define IS_OSPF6_DUMP_LSUPDATE     (ospf6_dump[OSPF6_DUMP_LSUPDATE].dump)
#define IS_OSPF6_DUMP_LSACK        (ospf6_dump[OSPF6_DUMP_LSACK].dump)
#define IS_OSPF6_DUMP_NEIGHBOR     (ospf6_dump[OSPF6_DUMP_NEIGHBOR].dump)
#define IS_OSPF6_DUMP_INTERFACE    (ospf6_dump[OSPF6_DUMP_INTERFACE].dump)
#define IS_OSPF6_DUMP_AREA         (ospf6_dump[OSPF6_DUMP_AREA].dump)
#define IS_OSPF6_DUMP_LSA          (ospf6_dump[OSPF6_DUMP_LSA].dump)
#define IS_OSPF6_DUMP_ZEBRA        (ospf6_dump[OSPF6_DUMP_ZEBRA].dump)
#define IS_OSPF6_DUMP_CONFIG       (ospf6_dump[OSPF6_DUMP_CONFIG].dump)
#define IS_OSPF6_DUMP_DBEX         (ospf6_dump[OSPF6_DUMP_DBEX].dump)
#define IS_OSPF6_DUMP_SPF          (ospf6_dump[OSPF6_DUMP_SPF].dump)
#define IS_OSPF6_DUMP_ROUTE        (ospf6_dump[OSPF6_DUMP_ROUTE].dump)
#define IS_OSPF6_DUMP_LSDB         (ospf6_dump[OSPF6_DUMP_LSDB].dump)
#define IS_OSPF6_DUMP_REDISTRIBUTE (ospf6_dump[OSPF6_DUMP_REDISTRIBUTE].dump)

#endif /* OSPF6_DUMP_H */

