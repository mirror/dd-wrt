/*
 *	BIRD -- I/O and event loop
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "nest/bird.h"
#include "proto/bfd/io.h"

#include "lib/buffer.h"
#include "lib/heap.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/event.h"
#include "lib/socket.h"


struct birdloop
{
  pool *pool;
  pthread_t thread;
  pthread_mutex_t mutex;

  btime last_time;
  btime real_time;
  u8 use_monotonic_clock;

  u8 stop_called;
  u8 poll_active;
  u8 wakeup_masked;
  int wakeup_fds[2];

  BUFFER(timer2 *) timers;
  list event_list;
  list sock_list;
  uint sock_num;

  BUFFER(sock *) poll_sk;
  BUFFER(struct pollfd) poll_fd;
  u8 poll_changed;
  u8 close_scheduled;
};


/*
 *	Current thread context
 */

static pthread_key_t current_loop_key;

static inline struct birdloop *
birdloop_current(void)
{
  return pthread_getspecific(current_loop_key);
}

static inline void
birdloop_set_current(struct birdloop *loop)
{
  pthread_setspecific(current_loop_key, loop);
}

static inline void
birdloop_init_current(void)
{
  pthread_key_create(&current_loop_key, NULL);
}


/*
 *	Time clock
 */

static void times_update_alt(struct birdloop *loop);

static void
times_init(struct birdloop *loop)
{
  struct timespec ts;
  int rv;

  rv = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (rv < 0)
  {
    log(L_WARN "Monotonic clock is missing");

    loop->use_monotonic_clock = 0;
    loop->last_time = 0;
    loop->real_time = 0;
    times_update_alt(loop);
    return;
  }

  if ((ts.tv_sec < 0) || (((s64) ts.tv_sec) > ((s64) 1 << 40)))
    log(L_WARN "Monotonic clock is crazy");

  loop->use_monotonic_clock = 1;
  loop->last_time = ((s64) ts.tv_sec S) + (ts.tv_nsec / 1000);
  loop->real_time = 0;
}

static void
times_update_pri(struct birdloop *loop)
{
  struct timespec ts;
  int rv;

  rv = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (rv < 0)
    die("clock_gettime: %m");

  btime new_time = ((s64) ts.tv_sec S) + (ts.tv_nsec / 1000);

  if (new_time < loop->last_time)
    log(L_ERR "Monotonic clock is broken");

  loop->last_time = new_time;
  loop->real_time = 0;
}

static void
times_update_alt(struct birdloop *loop)
{
  struct timeval tv;
  int rv;

  rv = gettimeofday(&tv, NULL);
  if (rv < 0)
    die("gettimeofday: %m");

  btime new_time = ((s64) tv.tv_sec S) + tv.tv_usec;
  btime delta = new_time - loop->real_time;

  if ((delta < 0) || (delta > (60 S)))
  {
    if (loop->real_time)
      log(L_WARN "Time jump, delta %d us", (int) delta);

    delta = 100 MS;
  }

  loop->last_time += delta;
  loop->real_time = new_time;
}

static void
times_update(struct birdloop *loop)
{
  if (loop->use_monotonic_clock)
    times_update_pri(loop);
  else
    times_update_alt(loop);
}

btime
current_time(void)
{
  return birdloop_current()->last_time;
}


/*
 *	Wakeup code for birdloop
 */

static void
pipe_new(int *pfds)
{
  int rv = pipe(pfds);
  if (rv < 0)
    die("pipe: %m");

  if (fcntl(pfds[0], F_SETFL, O_NONBLOCK) < 0)
    die("fcntl(O_NONBLOCK): %m");

  if (fcntl(pfds[1], F_SETFL, O_NONBLOCK) < 0)
    die("fcntl(O_NONBLOCK): %m");
}

void
pipe_drain(int fd)
{
  char buf[64];
  int rv;
  
 try:
  rv = read(fd, buf, 64);
  if (rv < 0)
  {
    if (errno == EINTR)
      goto try;
    if (errno == EAGAIN)
      return;
    die("wakeup read: %m");
  }
  if (rv == 64)
    goto try;
}

void
pipe_kick(int fd)
{
  u64 v = 1;
  int rv;

 try:
  rv = write(fd, &v, sizeof(u64));
  if (rv < 0)
  {
    if (errno == EINTR)
      goto try;
    if (errno == EAGAIN)
      return;
    die("wakeup write: %m");
  }
}

static inline void
wakeup_init(struct birdloop *loop)
{
  pipe_new(loop->wakeup_fds);
}

static inline void
wakeup_drain(struct birdloop *loop)
{
  pipe_drain(loop->wakeup_fds[0]);
}

static inline void
wakeup_do_kick(struct birdloop *loop) 
{
  pipe_kick(loop->wakeup_fds[1]);
}

static inline void
wakeup_kick(struct birdloop *loop)
{
  if (!loop->wakeup_masked)
    wakeup_do_kick(loop);
  else
    loop->wakeup_masked = 2;
}


/*
 *	Events
 */

static inline uint
events_waiting(struct birdloop *loop)
{
  return !EMPTY_LIST(loop->event_list);
}

static inline void
events_init(struct birdloop *loop)
{
  init_list(&loop->event_list);
}

static void
events_fire(struct birdloop *loop)
{
  times_update(loop);
  ev_run_list(&loop->event_list);
}

void
ev2_schedule(event *e)
{
  struct birdloop *loop = birdloop_current();

  if (loop->poll_active && EMPTY_LIST(loop->event_list))
    wakeup_kick(loop);

  if (e->n.next)
    rem_node(&e->n);

  add_tail(&loop->event_list, &e->n);
}


/*
 *	Timers
 */

#define TIMER_LESS(a,b)		((a)->expires < (b)->expires)
#define TIMER_SWAP(heap,a,b,t)	(t = heap[a], heap[a] = heap[b], heap[b] = t, \
				   heap[a]->index = (a), heap[b]->index = (b))

static inline uint timers_count(struct birdloop *loop)
{ return loop->timers.used - 1; }

static inline timer2 *timers_first(struct birdloop *loop)
{ return (loop->timers.used > 1) ? loop->timers.data[1] : NULL; }


static void
tm2_free(resource *r)
{
  timer2 *t = (timer2 *) r;

  tm2_stop(t);
}

static void
tm2_dump(resource *r)
{
  timer2 *t = (timer2 *) r;

  debug("(code %p, data %p, ", t->hook, t->data);
  if (t->randomize)
    debug("rand %d, ", t->randomize);
  if (t->recurrent)
    debug("recur %d, ", t->recurrent);
  if (t->expires)
    debug("expires in %d ms)\n", (t->expires - current_time()) TO_MS);
  else
    debug("inactive)\n");
}


static struct resclass tm2_class = {
  "Timer",
  sizeof(timer2),
  tm2_free,
  tm2_dump,
  NULL,
  NULL
};

timer2 *
tm2_new(pool *p)
{
  timer2 *t = ralloc(p, &tm2_class);
  t->index = -1;
  return t;
}

void
tm2_set(timer2 *t, btime when)
{
  struct birdloop *loop = birdloop_current();
  uint tc = timers_count(loop);

  if (!t->expires)
  {
    t->index = ++tc;
    t->expires = when;
    BUFFER_PUSH(loop->timers) = t;
    HEAP_INSERT(loop->timers.data, tc, timer2 *, TIMER_LESS, TIMER_SWAP);
  }
  else if (t->expires < when)
  {
    t->expires = when;
    HEAP_INCREASE(loop->timers.data, tc, timer2 *, TIMER_LESS, TIMER_SWAP, t->index);
  }
  else if (t->expires > when)
  {
    t->expires = when;
    HEAP_DECREASE(loop->timers.data, tc, timer2 *, TIMER_LESS, TIMER_SWAP, t->index);
  }

  if (loop->poll_active && (t->index == 1))
    wakeup_kick(loop);
}

void
tm2_start(timer2 *t, btime after)
{
  tm2_set(t, current_time() + MAX(after, 0));
}

void
tm2_stop(timer2 *t)
{
  if (!t->expires)
    return;

  struct birdloop *loop = birdloop_current();
  uint tc = timers_count(loop);

  HEAP_DELETE(loop->timers.data, tc, timer2 *, TIMER_LESS, TIMER_SWAP, t->index);
  BUFFER_POP(loop->timers);

  t->index = -1;
  t->expires = 0;
}

static void
timers_init(struct birdloop *loop)
{
  BUFFER_INIT(loop->timers, loop->pool, 4);
  BUFFER_PUSH(loop->timers) = NULL;
}

static void
timers_fire(struct birdloop *loop)
{
  btime base_time;
  timer2 *t;

  times_update(loop);
  base_time = loop->last_time;

  while (t = timers_first(loop))
  {
    if (t->expires > base_time)
      return;

    if (t->recurrent)
    {
      btime when = t->expires + t->recurrent;
      
      if (when <= loop->last_time)
	when = loop->last_time + t->recurrent;

      if (t->randomize)
	when += random() % (t->randomize + 1);

      tm2_set(t, when);
    }
    else
      tm2_stop(t);

    t->hook(t);
  }
}


/*
 *	Sockets
 */

static void
sockets_init(struct birdloop *loop)
{
  init_list(&loop->sock_list);
  loop->sock_num = 0;

  BUFFER_INIT(loop->poll_sk, loop->pool, 4);
  BUFFER_INIT(loop->poll_fd, loop->pool, 4);
  loop->poll_changed = 1;	/* add wakeup fd */
}

static void
sockets_add(struct birdloop *loop, sock *s)
{
  add_tail(&loop->sock_list, &s->n);
  loop->sock_num++;

  s->index = -1;
  loop->poll_changed = 1;

  if (loop->poll_active)
    wakeup_kick(loop);
}

void
sk_start(sock *s)
{
  struct birdloop *loop = birdloop_current();

  sockets_add(loop, s);
}

static void
sockets_remove(struct birdloop *loop, sock *s)
{
  rem_node(&s->n);
  loop->sock_num--;

  if (s->index >= 0)
    loop->poll_sk.data[s->index] = NULL;

  s->index = -1;
  loop->poll_changed = 1;

  /* Wakeup moved to sk_stop() */
}

void
sk_stop(sock *s)
{
  struct birdloop *loop = birdloop_current();

  sockets_remove(loop, s);

  if (loop->poll_active)
  {
    loop->close_scheduled = 1;
    wakeup_kick(loop);
  }
  else
    close(s->fd);

  s->fd = -1;
}

static inline uint sk_want_events(sock *s)
{ return (s->rx_hook ? POLLIN : 0) | ((s->ttx != s->tpos) ? POLLOUT : 0); }

/*
FIXME: this should be called from sock code

static void
sockets_update(struct birdloop *loop, sock *s)
{
  if (s->index >= 0)
    loop->poll_fd.data[s->index].events = sk_want_events(s);
}
*/

static void
sockets_prepare(struct birdloop *loop)
{
  BUFFER_SET(loop->poll_sk, loop->sock_num + 1);
  BUFFER_SET(loop->poll_fd, loop->sock_num + 1);

  struct pollfd *pfd = loop->poll_fd.data;
  sock **psk = loop->poll_sk.data;
  int i = 0;
  node *n;

  WALK_LIST(n, loop->sock_list)
  {
    sock *s = SKIP_BACK(sock, n, n);

    ASSERT(i < loop->sock_num);

    s->index = i;
    *psk = s;
    pfd->fd = s->fd;
    pfd->events = sk_want_events(s);
    pfd->revents = 0;

    pfd++;
    psk++;
    i++;
  }

  ASSERT(i == loop->sock_num);

  /* Add internal wakeup fd */
  *psk = NULL;
  pfd->fd = loop->wakeup_fds[0];
  pfd->events = POLLIN;
  pfd->revents = 0;

  loop->poll_changed = 0;
}

static void
sockets_close_fds(struct birdloop *loop)
{
  struct pollfd *pfd = loop->poll_fd.data;
  sock **psk = loop->poll_sk.data;
  int poll_num = loop->poll_fd.used - 1;

  int i;
  for (i = 0; i < poll_num; i++)
    if (psk[i] == NULL)
      close(pfd[i].fd);

  loop->close_scheduled = 0;
}

int sk_read(sock *s);
int sk_write(sock *s);

static void
sockets_fire(struct birdloop *loop)
{
  struct pollfd *pfd = loop->poll_fd.data;
  sock **psk = loop->poll_sk.data;
  int poll_num = loop->poll_fd.used - 1;

  times_update(loop);

  /* Last fd is internal wakeup fd */
  if (pfd[loop->sock_num].revents & POLLIN)
    wakeup_drain(loop);

  int i;
  for (i = 0; i < poll_num; pfd++, psk++, i++)
  {
    int e = 1;

    if (! pfd->revents)
      continue;

    if (pfd->revents & POLLNVAL)
      die("poll: invalid fd %d", pfd->fd);

    if (pfd->revents & POLLIN)
      while (e && *psk && (*psk)->rx_hook)
	e = sk_read(*psk);

    e = 1;
    if (pfd->revents & POLLOUT)
      while (e && *psk)
	e = sk_write(*psk);
  }
}


/*
 *	Birdloop
 */

static void * birdloop_main(void *arg);

struct birdloop *
birdloop_new(void)
{
  /* FIXME: this init should be elsewhere and thread-safe */
  static int init = 0;
  if (!init)
    { birdloop_init_current(); init = 1; }

  pool *p = rp_new(NULL, "Birdloop root");
  struct birdloop *loop = mb_allocz(p, sizeof(struct birdloop));
  loop->pool = p;
  pthread_mutex_init(&loop->mutex, NULL);

  times_init(loop);
  wakeup_init(loop);

  events_init(loop);
  timers_init(loop);
  sockets_init(loop);

  return loop;
}

void
birdloop_start(struct birdloop *loop)
{
  int rv = pthread_create(&loop->thread, NULL, birdloop_main, loop);
  if (rv)
    die("pthread_create(): %M", rv);
}

void
birdloop_stop(struct birdloop *loop)
{
  pthread_mutex_lock(&loop->mutex);
  loop->stop_called = 1;
  wakeup_do_kick(loop);
  pthread_mutex_unlock(&loop->mutex);

  int rv = pthread_join(loop->thread, NULL);
  if (rv)
    die("pthread_join(): %M", rv);
}

void
birdloop_free(struct birdloop *loop)
{
  rfree(loop->pool);
}


void
birdloop_enter(struct birdloop *loop)
{
  /* TODO: these functions could save and restore old context */
  pthread_mutex_lock(&loop->mutex);
  birdloop_set_current(loop);
}

void
birdloop_leave(struct birdloop *loop)
{
  /* TODO: these functions could save and restore old context */
  birdloop_set_current(NULL);
  pthread_mutex_unlock(&loop->mutex);
}

void
birdloop_mask_wakeups(struct birdloop *loop)
{
  pthread_mutex_lock(&loop->mutex);
  loop->wakeup_masked = 1;
  pthread_mutex_unlock(&loop->mutex);
}

void
birdloop_unmask_wakeups(struct birdloop *loop)
{
  pthread_mutex_lock(&loop->mutex);
  if (loop->wakeup_masked == 2)
    wakeup_do_kick(loop);
  loop->wakeup_masked = 0;
  pthread_mutex_unlock(&loop->mutex);
}

static void *
birdloop_main(void *arg)
{
  struct birdloop *loop = arg;
  timer2 *t;
  int rv, timeout;

  birdloop_set_current(loop);

  pthread_mutex_lock(&loop->mutex);
  while (1)
  {
    events_fire(loop);
    timers_fire(loop);

    times_update(loop);
    if (events_waiting(loop))
      timeout = 0;
    else if (t = timers_first(loop))
      timeout = (tm2_remains(t) TO_MS) + 1;
    else
      timeout = -1;

    if (loop->poll_changed)
      sockets_prepare(loop);

    loop->poll_active = 1;
    pthread_mutex_unlock(&loop->mutex);

  try:
    rv = poll(loop->poll_fd.data, loop->poll_fd.used, timeout);
    if (rv < 0)
    {
      if (errno == EINTR || errno == EAGAIN)
	goto try;
      die("poll: %m");
    }

    pthread_mutex_lock(&loop->mutex);
    loop->poll_active = 0;

    if (loop->close_scheduled)
      sockets_close_fds(loop);

    if (loop->stop_called)
      break;

    if (rv)
      sockets_fire(loop);

    timers_fire(loop);
  }

  loop->stop_called = 0;
  pthread_mutex_unlock(&loop->mutex);

  return NULL;
}


