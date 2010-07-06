
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004-2009, the olsr.org team - see HISTORY file
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "scheduler.h"
#include "log.h"
#include "link_set.h"
#include "olsr.h"
#include "olsr_cookie.h"
#include "net_os.h"
#include "mpr_selector_set.h"

#include <sys/times.h>

#include <unistd.h>
#include <assert.h>

#ifdef WIN32
#define close(x) closesocket(x)
#endif

/* Timer data, global. Externed in scheduler.h */
uint32_t now_times;                    /* relative time compared to startup (in milliseconds */
struct timeval first_tv;               /* timevalue during startup */
struct timeval last_tv;                /* timevalue used for last olsr_times() calculation */

/* Hashed root of all timers */
static struct list_node timer_wheel[TIMER_WHEEL_SLOTS];
static uint32_t timer_last_run;        /* remember the last timeslot walk */

/* Memory cookie for the block based memory manager */
static struct olsr_cookie_info *timer_mem_cookie = NULL;

/* Head of all OLSR used sockets */
static struct list_node socket_head = { &socket_head, &socket_head };

/* Prototypes */
static void walk_timers(uint32_t *);
static void poll_sockets(void);
static uint32_t calc_jitter(unsigned int rel_time, uint8_t jitter_pct, unsigned int random_val);

/*
 * A wrapper around times(2). Note, that this function has some
 * portability problems, so do not rely on absolute values returned.
 * Under Linux, uclibc and libc directly call the sys_times() located
 * in kernel/sys.c and will only return an error if the tms_buf is
 * not writeable.
 */
static uint32_t
olsr_times(void)
{
  struct timeval tv;
  uint32_t t;

  if (gettimeofday(&tv, NULL) != 0) {
    olsr_exit("OS clock is not working, have to shut down OLSR", 1);
  }

  /* test if time jumped backward or more than 60 seconds forward */
  if (tv.tv_sec < last_tv.tv_sec || (tv.tv_sec == last_tv.tv_sec && tv.tv_usec < last_tv.tv_usec)
      || tv.tv_sec - last_tv.tv_sec > 60) {
    OLSR_PRINTF(1, "Time jump (%d.%06d to %d.%06d)\n",
              (int32_t) (last_tv.tv_sec), (int32_t) (last_tv.tv_usec), (int32_t) (tv.tv_sec), (int32_t) (tv.tv_usec));

    t = (last_tv.tv_sec - first_tv.tv_sec) * 1000 + (last_tv.tv_usec - first_tv.tv_usec) / 1000;
    t++;                        /* advance time by one millisecond */

    first_tv = tv;
    first_tv.tv_sec -= (t / 1000);
    first_tv.tv_usec -= ((t % 1000) * 1000);

    if (first_tv.tv_usec < 0) {
      first_tv.tv_sec--;
      first_tv.tv_usec += 1000000;
    }
    last_tv = tv;
    return t;
  }
  last_tv = tv;
  return (tv.tv_sec - first_tv.tv_sec) * 1000 + (tv.tv_usec - first_tv.tv_usec) / 1000;
}

/**
 * Returns a timestamp s seconds in the future
 */
uint32_t
olsr_getTimestamp(uint32_t s)
{
  return now_times + s;
}

/**
 * Returns the number of milliseconds until the timestamp will happen
 */

int32_t
olsr_getTimeDue(uint32_t s)
{
  uint32_t diff;
  if (s > now_times) {
    diff = s - now_times;

    /* overflow ? */
    if (diff > (1u << 31)) {
      return -(int32_t) (0xffffffff - diff);
    }
    return (int32_t) (diff);
  }

  diff = now_times - s;
  /* overflow ? */
  if (diff > (1u << 31)) {
    return (int32_t) (0xffffffff - diff);
  }
  return -(int32_t) (diff);
}

bool
olsr_isTimedOut(uint32_t s)
{
  if (s > now_times) {
    return s - now_times > (1u << 31);
  }

  return now_times - s <= (1u << 31);
}

/**
 * Add a socket and handler to the socketset
 * beeing used in the main select(2) loop
 * in listen_loop
 *
 *@param fd the socket
 *@param pf the processing function
 */
void
add_olsr_socket(int fd, socket_handler_func pf_pr, socket_handler_func pf_imm, void *data, unsigned int flags)
{
  struct olsr_socket_entry *new_entry;

  if (fd < 0 || (pf_pr == NULL && pf_imm == NULL)) {
    OLSR_PRINTF(1, "Bogus socket entry - not registering...");
    return;
  }
  OLSR_PRINTF(3, "Adding OLSR socket entry %d\n", fd);

  new_entry = olsr_malloc(sizeof(*new_entry), "Socket entry");

  new_entry->fd = fd;
  new_entry->process_immediate = pf_imm;
  new_entry->process_pollrate = pf_pr;
  new_entry->data = data;
  new_entry->flags = flags;

  /* Queue */
  list_node_init(&new_entry->socket_node);
  list_add_before(&socket_head, &new_entry->socket_node);
}

/**
 * Remove a socket and handler to the socketset
 * beeing used in the main select(2) loop
 * in listen_loop
 *
 *@param fd the socket
 *@param pf the processing function
 */
int
remove_olsr_socket(int fd, socket_handler_func pf_pr, socket_handler_func pf_imm)
{
  struct olsr_socket_entry *entry;

  if (fd < 0 || (pf_pr == NULL && pf_imm == NULL)) {
    OLSR_PRINTF(1, "Bogus socket entry - not processing...");
    return 0;
  }
  OLSR_PRINTF(3, "Removing OLSR socket entry %d\n", fd);

  OLSR_FOR_ALL_SOCKETS(entry) {
    if (entry->fd == fd && entry->process_immediate == pf_imm && entry->process_pollrate == pf_pr) {
      /* just mark this node as "deleted", it will be cleared later at the end of handle_fds() */
      entry->process_immediate = NULL;
      entry->process_pollrate = NULL;
      entry->flags = 0;
      return 1;
    }
  }
  OLSR_FOR_ALL_SOCKETS_END(entry);
  return 0;
}

void
enable_olsr_socket(int fd, socket_handler_func pf_pr, socket_handler_func pf_imm, unsigned int flags)
{
  struct olsr_socket_entry *entry;

  OLSR_FOR_ALL_SOCKETS(entry) {
    if (entry->fd == fd && entry->process_immediate == pf_imm && entry->process_pollrate == pf_pr) {
      entry->flags |= flags;
    }
  }
  OLSR_FOR_ALL_SOCKETS_END(entry);
}

void
disable_olsr_socket(int fd, socket_handler_func pf_pr, socket_handler_func pf_imm, unsigned int flags)
{
  struct olsr_socket_entry *entry;

  OLSR_FOR_ALL_SOCKETS(entry) {
    if (entry->fd == fd && entry->process_immediate == pf_imm && entry->process_pollrate == pf_pr) {
      entry->flags &= ~flags;
    }
  }
  OLSR_FOR_ALL_SOCKETS_END(entry);
}

/**
 * Close and free all sockets.
 */
void
olsr_flush_sockets(void)
{
  struct olsr_socket_entry *entry;

  OLSR_FOR_ALL_SOCKETS(entry) {
    close(entry->fd);
    list_remove(&entry->socket_node);
    free(entry);
  } OLSR_FOR_ALL_SOCKETS_END(entry);
}

static void
poll_sockets(void)
{
  int n;
  struct olsr_socket_entry *entry;
  fd_set ibits, obits;
  struct timeval tvp = { 0, 0 };
  int hfd = 0, fdsets = 0;

  /* If there are no registered sockets we
   * do not call select(2)
   */
  if (list_is_empty(&socket_head)) {
    return;
  }

  FD_ZERO(&ibits);
  FD_ZERO(&obits);

  /* Adding file-descriptors to FD set */
  OLSR_FOR_ALL_SOCKETS(entry) {
    if (entry->process_pollrate == NULL) {
      continue;
    }
    if ((entry->flags & SP_PR_READ) != 0) {
      fdsets |= SP_PR_READ;
      FD_SET((unsigned int)entry->fd, &ibits);  /* And we cast here since we get a warning on Win32 */
    }
    if ((entry->flags & SP_PR_WRITE) != 0) {
      fdsets |= SP_PR_WRITE;
      FD_SET((unsigned int)entry->fd, &obits);  /* And we cast here since we get a warning on Win32 */
    }
    if ((entry->flags & (SP_PR_READ | SP_PR_WRITE)) != 0 && entry->fd >= hfd) {
      hfd = entry->fd + 1;
    }
  }
  OLSR_FOR_ALL_SOCKETS_END(entry);

  /* Running select on the FD set */
  do {
    n = olsr_select(hfd, fdsets & SP_PR_READ ? &ibits : NULL, fdsets & SP_PR_WRITE ? &obits : NULL, NULL, &tvp);
  } while (n == -1 && errno == EINTR);

  if (n == 0) {
    return;
  }
  if (n == -1) {                /* Did something go wrong? */
    OLSR_PRINTF(1, "select error: %s", strerror(errno));
    return;
  }

  /* Update time since this is much used by the parsing functions */
  now_times = olsr_times();
  OLSR_FOR_ALL_SOCKETS(entry) {
    int flags;
    if (entry->process_pollrate == NULL) {
      continue;
    }
    flags = 0;
    if (FD_ISSET(entry->fd, &ibits)) {
      flags |= SP_PR_READ;
    }
    if (FD_ISSET(entry->fd, &obits)) {
      flags |= SP_PR_WRITE;
    }
    if (flags != 0) {
      entry->process_pollrate(entry->fd, entry->data, flags);
    }
  }
  OLSR_FOR_ALL_SOCKETS_END(entry);
}

static void
handle_fds(uint32_t next_interval)
{
  struct olsr_socket_entry *entry;
  struct timeval tvp;
  int32_t remaining;

  /* calculate the first timeout */
  now_times = olsr_times();

  remaining = TIME_DUE(next_interval);
  if (remaining <= 0) {
    /* we are already over the interval */
    if (list_is_empty(&socket_head)) {
      /* If there are no registered sockets we do not call select(2) */
      return;
    }
    tvp.tv_sec = 0;
    tvp.tv_usec = 0;
  } else {
    /* we need an absolute time - milliseconds */
    tvp.tv_sec = remaining / MSEC_PER_SEC;
    tvp.tv_usec = (remaining % MSEC_PER_SEC) * USEC_PER_MSEC;
  }

  /* do at least one select */
  for (;;) {
    fd_set ibits, obits;
    int n, hfd = 0, fdsets = 0;
    FD_ZERO(&ibits);
    FD_ZERO(&obits);

    /* Adding file-descriptors to FD set */
    OLSR_FOR_ALL_SOCKETS(entry) {
      if (entry->process_immediate == NULL) {
        continue;
      }
      if ((entry->flags & SP_IMM_READ) != 0) {
        fdsets |= SP_IMM_READ;
        FD_SET((unsigned int)entry->fd, &ibits);        /* And we cast here since we get a warning on Win32 */
      }
      if ((entry->flags & SP_IMM_WRITE) != 0) {
        fdsets |= SP_IMM_WRITE;
        FD_SET((unsigned int)entry->fd, &obits);        /* And we cast here since we get a warning on Win32 */
      }
      if ((entry->flags & (SP_IMM_READ | SP_IMM_WRITE)) != 0 && entry->fd >= hfd) {
        hfd = entry->fd + 1;
      }
    }
    OLSR_FOR_ALL_SOCKETS_END(entry);

    if (hfd == 0 && (long)remaining <= 0) {
      /* we are over the interval and we have no fd's. Skip the select() etc. */
      break;
    }

    do {
      n = olsr_select(hfd, fdsets & SP_IMM_READ ? &ibits : NULL, fdsets & SP_IMM_WRITE ? &obits : NULL, NULL, &tvp);
    } while (n == -1 && errno == EINTR);

    if (n == 0) {               /* timeout! */
      break;
    }
    if (n == -1) {              /* Did something go wrong? */
      OLSR_PRINTF(1, "select error: %s", strerror(errno));
      break;
    }

    /* Update time since this is much used by the parsing functions */
    now_times = olsr_times();
    OLSR_FOR_ALL_SOCKETS(entry) {
      int flags;
      if (entry->process_immediate == NULL) {
        continue;
      }
      flags = 0;
      if (FD_ISSET(entry->fd, &ibits)) {
        flags |= SP_IMM_READ;
      }
      if (FD_ISSET(entry->fd, &obits)) {
        flags |= SP_IMM_WRITE;
      }
      if (flags != 0) {
        entry->process_immediate(entry->fd, entry->data, flags);
      }
    }
    OLSR_FOR_ALL_SOCKETS_END(entry);

    /* calculate the next timeout */
    remaining = TIME_DUE(next_interval);
    if (remaining <= 0) {
      /* we are already over the interval */
      break;
    }
    /* we need an absolute time - milliseconds */
    tvp.tv_sec = remaining / MSEC_PER_SEC;
    tvp.tv_usec = (remaining % MSEC_PER_SEC) * USEC_PER_MSEC;
  }

  OLSR_FOR_ALL_SOCKETS(entry) {
    if (entry->process_immediate == NULL && entry->process_pollrate == NULL) {
      /* clean up socket handler */
      list_remove(&entry->socket_node);
      free(entry);
    }
  } OLSR_FOR_ALL_SOCKETS_END(entry);
}

/**
 * Main scheduler event loop. Polls at every
 * sched_poll_interval and calls all functions
 * that are timed out or that are triggered.
 * Also calls the olsr_process_changes()
 * function at every poll.
 *
 * @return nada
 */
void __attribute__ ((noreturn))
olsr_scheduler(void)
{
  OLSR_PRINTF(1, "Scheduler started - polling every %f ms\n", olsr_cnf->pollrate);

  /* Main scheduler loop */
  while (true) {
    uint32_t next_interval;

    /*
     * Update the global timestamp. We are using a non-wallclock timer here
     * to avoid any undesired side effects if the system clock changes.
     */
    now_times = olsr_times();
    next_interval = GET_TIMESTAMP(olsr_cnf->pollrate * 1000);

    /* Read incoming data */
    poll_sockets();

    /* Process timers */
    walk_timers(&timer_last_run);

    /* Update */
    olsr_process_changes();

    /* Check for changes in topology */
    if (link_changes) {
      increase_local_ansn();
      OLSR_PRINTF(3, "ANSN UPDATED %d\n\n", get_local_ansn());
      link_changes = false;
    }

    /* Read incoming data and handle it immediiately */
    handle_fds(next_interval);
  }
}

/**
 * Decrement a relative timer by a random number range.
 *
 * @param the relative timer expressed in units of milliseconds.
 * @param the jitter in percent
 * @param cached result of random() at system init.
 * @return the absolute timer in system clock tick units
 */
static uint32_t
calc_jitter(unsigned int rel_time, uint8_t jitter_pct, unsigned int random_val)
{
  unsigned int jitter_time;

  /*
   * No jitter or, jitter larger than 99% does not make sense.
   * Also protect against overflows resulting from > 25 bit timers.
   */
  if (jitter_pct == 0 || jitter_pct > 99 || rel_time > (1 << 24)) {
    return GET_TIMESTAMP(rel_time);
  }

  /*
   * Play some tricks to avoid overflows with integer arithmetic.
   */
  jitter_time = (jitter_pct * rel_time) / 100;
  jitter_time = random_val / (1 + RAND_MAX / (jitter_time + 1));

  OLSR_PRINTF(3, "TIMER: jitter %u%% rel_time %ums to %ums\n", jitter_pct, rel_time, rel_time - jitter_time);

  return GET_TIMESTAMP(rel_time - jitter_time);
}

/**
 * Init datastructures for maintaining timers.
 */
void
olsr_init_timers(void)
{
  int idx;

  OLSR_PRINTF(3, "Initializing scheduler.\n");

  /* Grab initial timestamp */
  if (gettimeofday(&first_tv, NULL)) {
    olsr_exit("OS clock is not working, have to shut down OLSR", 1);
  }
  last_tv = first_tv;
  now_times = olsr_times();

  for (idx = 0; idx < TIMER_WHEEL_SLOTS; idx++) {
    list_head_init(&timer_wheel[idx]);
  }

  /*
   * Reset the last timer run.
   */
  timer_last_run = now_times;

  /* Allocate a cookie for the block based memeory manager. */
  timer_mem_cookie = olsr_alloc_cookie("timer_entry", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(timer_mem_cookie, sizeof(struct timer_entry));
}

/**
 * Walk through the timer list and check if any timer is ready to fire.
 * Callback the provided function with the context pointer.
 */
static void
walk_timers(uint32_t * last_run)
{
  unsigned int total_timers_walked = 0, total_timers_fired = 0;
  unsigned int wheel_slot_walks = 0;

  /*
   * Check the required wheel slots since the last time a timer walk was invoked,
   * or check *all* the wheel slots, whatever is less work.
   * The latter is meant as a safety belt if the scheduler falls behind.
   */
  while ((*last_run <= now_times) && (wheel_slot_walks < TIMER_WHEEL_SLOTS)) {
    struct list_node tmp_head_node;
    /* keep some statistics */
    unsigned int timers_walked = 0, timers_fired = 0;

    /* Get the hash slot for this clocktick */
    struct list_node *const timer_head_node = &timer_wheel[*last_run & TIMER_WHEEL_MASK];

    /* Walk all entries hanging off this hash bucket. We treat this basically as a stack
     * so that we always know if and where the next element is.
     */
    list_head_init(&tmp_head_node);
    while (!list_is_empty(timer_head_node)) {
      /* the top element */
      struct list_node *const timer_node = timer_head_node->next;
      struct timer_entry *const timer = list2timer(timer_node);

      /*
       * Dequeue and insert to a temporary list.
       * We do this to avoid loosing our walking context when
       * multiple timers fire.
       */
      list_remove(timer_node);
      list_add_after(&tmp_head_node, timer_node);
      timers_walked++;

      /* Ready to fire ? */
      if (TIMED_OUT(timer->timer_clock)) {

        OLSR_PRINTF(7, "TIMER: fire %s timer %p, ctx %p, "
                   "at clocktick %u (%s)\n",
                   timer->timer_cookie->ci_name,
                   timer, timer->timer_cb_context, (unsigned int)*last_run, olsr_wallclock_string());

        /* This timer is expired, call into the provided callback function */
        timer->timer_cb(timer->timer_cb_context);

        /* Only act on actually running timers */
        if (timer->timer_flags & OLSR_TIMER_RUNNING) {
          /*
           * Don't restart the periodic timer if the callback function has
           * stopped the timer.
           */
          if (timer->timer_period) {
            /* For periodical timers, rehash the random number and restart */
            timer->timer_random = random();
            olsr_change_timer(timer, timer->timer_period, timer->timer_jitter_pct, OLSR_TIMER_PERIODIC);
          } else {
            /* Singleshot timers are stopped */
            olsr_stop_timer(timer);
          }
        }

        timers_fired++;
      }
    }

    /*
     * Now merge the temporary list back to the old bucket.
     */
    list_merge(timer_head_node, &tmp_head_node);

    /* keep some statistics */
    total_timers_walked += timers_walked;
    total_timers_fired += timers_fired;

    /* Increment the time slot and wheel slot walk iteration */
    (*last_run)++;
    wheel_slot_walks++;
  }

  OLSR_PRINTF(7, "TIMER: processed %4u/%d clockwheel slots, "
             "timers walked %4u/%u, timers fired %u\n",
             wheel_slot_walks, TIMER_WHEEL_SLOTS, total_timers_walked, timer_mem_cookie->ci_usage, total_timers_fired);

  /*
   * If the scheduler has slipped and we have walked all wheel slots,
   * reset the last timer run.
   */
  *last_run = now_times;
}

/**
 * Stop and delete all timers.
 */
void
olsr_flush_timers(void)
{
  struct list_node *timer_head_node;
  unsigned int wheel_slot = 0;

  for (wheel_slot = 0; wheel_slot < TIMER_WHEEL_SLOTS; wheel_slot++) {
    timer_head_node = &timer_wheel[wheel_slot & TIMER_WHEEL_MASK];

    /* Kill all entries hanging off this hash bucket. */
    while (!list_is_empty(timer_head_node)) {
      olsr_stop_timer(list2timer(timer_head_node->next));
    }
  }
}

/**
 * Returns the difference between gmt and local time in seconds.
 * Use gmtime() and localtime() to keep things simple.
 *
 * taken and slightly modified from www.tcpdump.org.
 */
static int
olsr_get_timezone(void)
{
#define OLSR_TIMEZONE_UNINITIALIZED -1
  static int time_diff = OLSR_TIMEZONE_UNINITIALIZED;
  if (time_diff == OLSR_TIMEZONE_UNINITIALIZED) {
    int dir;
    const time_t t = time(NULL);
    const struct tm gmt = *gmtime(&t);
    const struct tm *loc = localtime(&t);

    time_diff = (loc->tm_hour - gmt.tm_hour) * 60 * 60 + (loc->tm_min - gmt.tm_min) * 60;

    /*
     * If the year or julian day is different, we span 00:00 GMT
     * and must add or subtract a day. Check the year first to
     * avoid problems when the julian day wraps.
     */
    dir = loc->tm_year - gmt.tm_year;
    if (!dir) {
      dir = loc->tm_yday - gmt.tm_yday;
    }

    time_diff += dir * 24 * 60 * 60;
  }
  return time_diff;
}

/**
 * Format an absolute wallclock system time string.
 * May be called upto 4 times in a single printf() statement.
 * Displays microsecond resolution.
 *
 * @return buffer to a formatted system time string.
 */
const char *
olsr_wallclock_string(void)
{
  static char buf[sizeof("00:00:00.000000")];
  struct timeval now;
  int sec, usec;

  gettimeofday(&now, NULL);

  sec = (int)now.tv_sec + olsr_get_timezone();
  usec = (int)now.tv_usec;

  snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%06d", (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60, usec);

  return buf;
}

/**
 * Format an relative non-wallclock system time string.
 * May be called upto 4 times in a single printf() statement.
 * Displays millisecond resolution.
 *
 * @param absolute time expressed in clockticks
 * @return buffer to a formatted system time string.
 */
const char *
olsr_clock_string(uint32_t clk)
{
  static char buf[sizeof("00:00:00.000")];

  /* On most systems a clocktick is a 10ms quantity. */
  unsigned int msec = clk % 1000;
  unsigned int sec = clk / 1000;

  snprintf(buf, sizeof(buf), "%02u:%02u:%02u.%03u", sec / 3600, (sec % 3600) / 60, (sec % 60), (msec % MSEC_PER_SEC));

  return buf;
}

/**
 * Start a new timer.
 *
 * @param relative time expressed in milliseconds
 * @param jitter expressed in percent
 * @param timer callback function
 * @param context for the callback function
 * @return a pointer to the created entry
 */
struct timer_entry *
olsr_start_timer(unsigned int rel_time,
                 uint8_t jitter_pct, bool periodical, timer_cb_func cb_func, void *context, struct olsr_cookie_info *ci)
{
  struct timer_entry *timer;

  if (ci == NULL) {
    ci = def_timer_ci;
  }
  assert(cb_func);

  timer = olsr_cookie_malloc(timer_mem_cookie);

  /*
   * Compute random numbers only once.
   */
  if (!timer->timer_random) {
    timer->timer_random = random();
  }

  /* Fill entry */
  timer->timer_clock = calc_jitter(rel_time, jitter_pct, timer->timer_random);
  timer->timer_cb = cb_func;
  timer->timer_cb_context = context;
  timer->timer_jitter_pct = jitter_pct;
  timer->timer_flags = OLSR_TIMER_RUNNING;

  /* The cookie is used for debugging to traceback the originator */
  timer->timer_cookie = ci;
  olsr_cookie_usage_incr(ci->ci_id);

  /* Singleshot or periodical timer ? */
  timer->timer_period = periodical ? rel_time : 0;

  /*
   * Now insert in the respective timer_wheel slot.
   */
  list_add_before(&timer_wheel[timer->timer_clock & TIMER_WHEEL_MASK], &timer->timer_list);

  OLSR_PRINTF(7, "TIMER: start %s timer %p firing in %s, ctx %p\n",
             ci->ci_name, timer, olsr_clock_string(timer->timer_clock), context);

  return timer;
}

/**
 * Delete a timer.
 *
 * @param the timer_entry that shall be removed
 * @return nada
 */
void
olsr_stop_timer(struct timer_entry *timer)
{
  /* It's okay to get a NULL here */
  if (!timer) {
    return;
  }

  assert(timer->timer_cookie);     /* we want timer cookies everywhere */

  OLSR_PRINTF(7, "TIMER: stop %s timer %p, ctx %p\n",
             timer->timer_cookie->ci_name, timer, timer->timer_cb_context);


  /*
   * Carve out of the existing wheel_slot and free.
   */
  list_remove(&timer->timer_list);
  timer->timer_flags &= ~OLSR_TIMER_RUNNING;
  olsr_cookie_usage_decr(timer->timer_cookie->ci_id);

  olsr_cookie_free(timer_mem_cookie, timer);
}

/**
 * Change a timer_entry.
 *
 * @param timer_entry to be changed.
 * @param new relative time expressed in units of milliseconds.
 * @param new jitter expressed in percent.
 * @return nada
 */
void
olsr_change_timer(struct timer_entry *timer, unsigned int rel_time, uint8_t jitter_pct, bool periodical)
{
  /* Sanity check. */
  if (!timer) {
    return;
  }

  assert(timer->timer_cookie);     /* we want timer cookies everywhere */

  /* Singleshot or periodical timer ? */
  timer->timer_period = periodical ? rel_time : 0;

  timer->timer_clock = calc_jitter(rel_time, jitter_pct, timer->timer_random);
  timer->timer_jitter_pct = jitter_pct;

  /*
   * Changes are easy: Remove timer from the exisiting timer_wheel slot
   * and reinsert into the new slot.
   */
  list_remove(&timer->timer_list);
  list_add_before(&timer_wheel[timer->timer_clock & TIMER_WHEEL_MASK], &timer->timer_list);

  OLSR_PRINTF(7, "TIMER: change %s timer %p, firing to %s, ctx %p\n",
             timer->timer_cookie->ci_name, timer, olsr_clock_string(timer->timer_clock), timer->timer_cb_context);
}

/*
 * This is the one stop shop for all sort of timer manipulation.
 * Depending on the paseed in parameters a new timer is started,
 * or an existing timer is started or an existing timer is
 * terminated.
 */
void
olsr_set_timer(struct timer_entry **timer_ptr,
               unsigned int rel_time,
               uint8_t jitter_pct, bool periodical, timer_cb_func cb_func, void *context, struct olsr_cookie_info *cookie)
{
  if (cookie) {
    cookie = def_timer_ci;
  }

  if (rel_time == 0) {
    /* No good future time provided, kill it. */
    olsr_stop_timer(*timer_ptr);
    *timer_ptr = NULL;
  }
  else if ((*timer_ptr) == NULL) {
    /* No timer running, kick it. */
    *timer_ptr = olsr_start_timer(rel_time, jitter_pct, periodical, cb_func, context, cookie);
  }
  else {
    olsr_change_timer(*timer_ptr, rel_time, jitter_pct, periodical);
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
