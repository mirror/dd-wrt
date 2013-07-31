/*
 *	BIRD Library -- Event Processing
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_EVENT_H_
#define _BIRD_EVENT_H_

#include "lib/resource.h"

typedef struct event {
  resource r;
  void (*hook)(void *);
  void *data;
  node n;				/* Internal link */
} event;

typedef list event_list;

extern event_list global_event_list;

event *ev_new(pool *);
void ev_run(event *);
#define ev_init_list(el) init_list(el)
void ev_enqueue(event_list *, event *);
void ev_schedule(event *);
void ev_postpone(event *);
int ev_run_list(event_list *);

#endif
