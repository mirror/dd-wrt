/*
 *	BIRD -- Static Route Generator
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_STATIC_H_
#define _BIRD_STATIC_H_

struct static_config {
  struct proto_config c;
  list iface_routes;			/* Routes to search on interface events */
  list other_routes;			/* Routes hooked to neighbor cache and reject routes */
};

void static_init_config(struct static_config *);

struct static_route {
  node n;
  struct static_route *chain;		/* Next for the same neighbor */
  ip_addr net;				/* Network we route */
  int masklen;				/* Mask length */
  int dest;				/* Destination type (RTD_*) */
  ip_addr via;				/* Destination router */
  struct neighbor *neigh;
  byte *if_name;			/* Name for RTD_DEVICE routes */
  int installed;			/* Installed in master table */
};

void static_show(struct proto *);

#endif
