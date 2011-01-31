/* timer.c - Timer support */
 
/* Written 1995-1997 by Werner Almesberger, EPFL-LRC */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "atmd.h"


static TIMER *timers = NULL;
struct timeval now;

static void dump_list(const char *label)
{
#ifdef DEBUG_TIMERS
   TIMER *walk;

   if (!debug) return;
   fprintf(stderr,"Timer list %s\n",label);
   for (walk = timers; walk; walk = walk->next)
    fprintf(stderr,"  0x%p: Timer @0x%p, %ld.%06d (-> 0x%p)\n",
      walk,walk->callback,(unsigned long) walk->expiration.tv_sec,
      (int) walk->expiration.tv_usec,walk->next);
#endif
}


TIMER *start_timer(long usec,void (*callback)(void *user),void *user)
{
    TIMER *n,*walk,*last;

    n = alloc_t(TIMER);
    n->expiration.tv_usec = now.tv_usec+usec;
    n->expiration.tv_sec = now.tv_sec;
    n->callback = callback;
    n->user = user;
    while (n->expiration.tv_usec > 1000000) {
	n->expiration.tv_usec -= 1000000;
	n->expiration.tv_sec++;
	
    }
    last = NULL;
    for (walk = timers; walk; walk = walk->next)
	if (walk->expiration.tv_sec > n->expiration.tv_sec ||
	  (walk->expiration.tv_sec == n->expiration.tv_sec &&
	  walk->expiration.tv_usec > n->expiration.tv_usec)) break;
	else last = walk;
    if (walk) Q_INSERT_BEFORE(timers,n,walk);
    else Q_INSERT_AFTER(timers,n,last);
    dump_list("START_TIMER");
    return n;
}


void stop_timer(TIMER *timer)
{
    Q_REMOVE(timers,timer);
    free(timer);
}


void (*timer_handler(TIMER *timer))(void *user)
{
    return timer ? timer->callback : NULL;
}


struct timeval *next_timer(void)
{
    static struct timeval delta;

    if (!timers) return NULL;
    delta.tv_sec = timers->expiration.tv_sec-now.tv_sec;
    delta.tv_usec = timers->expiration.tv_usec-now.tv_usec;
    while (delta.tv_usec < 0) {
	delta.tv_usec += 1000000;
	delta.tv_sec--;
    }
    if (delta.tv_sec < 0) delta.tv_sec = delta.tv_usec = 0;
    return &delta;
}


void pop_timer(TIMER *timer)
{
    dump_list("POP_TIMER");
    Q_REMOVE(timers,timer);
    timer->callback(timer->user);
    free(timer);
}


void expire_timers(void)
{
    while (timers && (timers->expiration.tv_sec < now.tv_sec ||
      (timers->expiration.tv_sec == now.tv_sec && timers->expiration.tv_usec <
      now.tv_usec))) pop_timer(timers);
}
