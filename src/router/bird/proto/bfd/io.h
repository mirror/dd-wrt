/*
 *	BIRD -- I/O and event loop
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_BFD_IO_H_
#define _BIRD_BFD_IO_H_

#include "nest/bird.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/event.h"
#include "lib/socket.h"
// #include "lib/timer.h"


typedef struct timer2
{
  resource r;
  void (*hook)(struct timer2 *);
  void *data;

  btime expires;			/* 0=inactive */
  uint randomize;			/* Amount of randomization */
  uint recurrent;			/* Timer recurrence */

  int index;
} timer2;


btime current_time(void);

void ev2_schedule(event *e);


timer2 *tm2_new(pool *p);
void tm2_set(timer2 *t, btime when);
void tm2_start(timer2 *t, btime after);
void tm2_stop(timer2 *t);

static inline int
tm2_active(timer2 *t)
{
  return t->expires != 0;
}

static inline btime 
tm2_remains(timer2 *t)
{
  btime now = current_time();
  return (t->expires > now) ? (t->expires - now) : 0;
}

static inline timer2 *
tm2_new_init(pool *p, void (*hook)(struct timer2 *), void *data, uint rec, uint rand)
{
  timer2 *t = tm2_new(p);
  t->hook = hook;
  t->data = data;
  t->recurrent = rec;
  t->randomize = rand;
  return t;
}

static inline void
tm2_set_max(timer2 *t, btime when)
{
  if (when > t->expires)
    tm2_set(t, when);
}

/*
static inline void
tm2_start_max(timer2 *t, btime after)
{
  btime rem = tm2_remains(t);
  tm2_start(t, MAX_(rem, after));
}
*/


void sk_start(sock *s);
void sk_stop(sock *s);



struct birdloop *birdloop_new(void);
void birdloop_start(struct birdloop *loop);
void birdloop_stop(struct birdloop *loop);
void birdloop_free(struct birdloop *loop);

void birdloop_enter(struct birdloop *loop);
void birdloop_leave(struct birdloop *loop);
void birdloop_mask_wakeups(struct birdloop *loop);
void birdloop_unmask_wakeups(struct birdloop *loop);


#endif /* _BIRD_BFD_IO_H_ */
