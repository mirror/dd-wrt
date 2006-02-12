/*
 *	BIRD -- Table-to-Table Routing Protocol a.k.a Pipe
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_PIPE_H_
#define _BIRD_PIPE_H_

struct pipe_config {
  struct proto_config c;
  struct rtable_config *peer;		/* Table we're connected to */
};

struct pipe_proto {
  struct proto p;
  struct rtable *peer;
  struct pipe_proto *phantom;
};

#endif
