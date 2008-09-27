
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * Timer rewrite (c) 2008, Hannes Gredler (hannes@gredler.at)
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

#include "defs.h"
#include "scheduler.h"
#include "log.h"
#include "tc_set.h"
#include "link_set.h"
#include "duplicate_set.h"
#include "mpr_selector_set.h"
#include "mid_set.h"
#include "mpr.h"
#include "olsr.h"
#include "build_msg.h"
#include "net_olsr.h"
#include "socket_parser.h"
#include "olsr_spf.h"
#include "link_set.h"
#include "olsr_cookie.h"

/* Timer data, global. Externed in defs.h */
clock_t now_times;		       /* current idea of times(2) reported uptime */

/* Hashed root of all timers */
struct list_node timer_wheel[TIMER_WHEEL_SLOTS];
clock_t timer_last_run;		       /* remember the last timeslot walk */

/* Pool of timers to avoid malloc() churn */
struct list_node free_timer_list;

/* Statistics */
unsigned int timers_running;


/**
 * Sleep until the next scheduling interval.
 *
 * @param scheduler loop runtime in clock ticks.
 * @return nada
 */
static void
olsr_scheduler_sleep(unsigned long scheduler_runtime)
{
  struct timespec remainder_spec, sleeptime_spec;
  struct timeval sleeptime_val, time_used, next_interval;
  olsr_u32_t next_interval_usec;
  unsigned long milliseconds_used;

  /* Calculate next planned scheduler invocation */
  next_interval_usec = olsr_cnf->pollrate * USEC_PER_SEC;
  next_interval.tv_sec = next_interval_usec / USEC_PER_SEC;
  next_interval.tv_usec = next_interval_usec % USEC_PER_SEC;

  /* Determine used runtime */
  milliseconds_used = scheduler_runtime * olsr_cnf->system_tick_divider;
  time_used.tv_sec = milliseconds_used / MSEC_PER_SEC;
  time_used.tv_usec = (milliseconds_used % MSEC_PER_SEC) * USEC_PER_MSEC;

  if (timercmp(&time_used, &next_interval, <)) {
    timersub(&next_interval, &time_used, &sleeptime_val);

    sleeptime_spec.tv_sec = sleeptime_val.tv_sec;
    sleeptime_spec.tv_nsec = sleeptime_val.tv_usec * NSEC_PER_USEC;

    while (nanosleep(&sleeptime_spec, &remainder_spec) < 0)
      sleeptime_spec = remainder_spec;
  }
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
void
olsr_scheduler(void)
{
  struct interface *ifn;

  OLSR_PRINTF(1, "Scheduler started - polling every %0.2f seconds\n",
	      olsr_cnf->pollrate);
  OLSR_PRINTF(3, "Max jitter is %f\n\n", olsr_cnf->max_jitter);

  /* Main scheduler loop */
  for (;;) {

    /*
     * Update the global timestamp. We are using a non-wallclock timer here
     * to avoid any undesired side effects if the system clock changes.
     */
    now_times = olsr_times();

    /* Read incoming data */
    olsr_poll_sockets();

    /* Process timers (before packet generation) */
    olsr_walk_timers(&timer_last_run);

    /* Update */
    olsr_process_changes();

    /* Check for changes in topology */
    if (link_changes) {
      OLSR_PRINTF(3, "ANSN UPDATED %d\n\n", get_local_ansn());
      increase_local_ansn();
      link_changes = OLSR_FALSE;
    }

    /* looping trough interfaces and emmitting pending data */
    for (ifn = ifnet; ifn; ifn = ifn->int_next) {
      if (net_output_pending(ifn) && TIMED_OUT(ifn->fwdtimer)) {
	net_output(ifn);
      }
    }

    /* We are done, sleep until the next scheduling interval. */
    olsr_scheduler_sleep(olsr_times() - now_times);

#if defined WIN32
    /* The Ctrl-C signal handler thread asks us to exit */
    if (olsr_win32_end_request) {
      break;
    }
#endif
  }

#if defined WIN32
  /* Tell the Ctrl-C signal handler thread that we have exited */
  olsr_win32_end_flag = TRUE;

  /*
   * The Ctrl-C signal handler thread will exit the process
   * and hence also kill us.
   */
  while (1) {
    Sleep(1000);		/* milliseconds */
  }
#endif
}


/**
 * Decrement a relative timer by a random number range.
 *
 * @param the relative timer expressed in units of milliseconds.
 * @param the jitter in percent
 * @param cached result of random() at system init.
 * @return the absolute timer in system clock tick units
 */
static clock_t
olsr_jitter(unsigned int rel_time, olsr_u8_t jitter_pct, unsigned int random)
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
  jitter_time = random / (1 + RAND_MAX / jitter_time);

#if 0
  OLSR_PRINTF(3, "TIMER: jitter %u%% rel_time %ums to %ums\n",
	      jitter_pct, rel_time, rel_time - jitter_time);
#endif

  return GET_TIMESTAMP(rel_time - jitter_time);
}


/**
 * Allocate a timer_entry.
 * Do this first by checking if something is available in the free_timer_pool
 * If not then allocate a big chunk of memory and thread its elements up
 * to the free_timer_list.
 */
static struct timer_entry *
olsr_get_timer(void)
{
  void *timer_block;
  struct timer_entry *timer;
  struct list_node *timer_list_node;
  unsigned int timer_index;

  /*
   * If there is at least one timer in the pool then remove the first
   * element from the pool and recycle it.
   */
  if (!list_is_empty(&free_timer_list)) {
    timer_list_node = free_timer_list.next;

    /* carve it out of the pool, do not memset overwrite timer->timer_random */
    list_remove(timer_list_node);
    timer = list2timer(timer_list_node);

    return timer;
  }

  /*
   * Nothing in the pool, allocate a new chunk.
   */
  timer_block =
    olsr_malloc(sizeof(struct timer_entry) * OLSR_TIMER_MEMORY_CHUNK,
		"timer chunk");

#if 0
  OLSR_PRINTF(3, "TIMER: alloc %u bytes chunk at %p\n",
	      sizeof(struct timer_entry) * OLSR_TIMER_MEMORY_CHUNK,
	      timer_block);
#endif

  /*
   * Slice the chunk up and put the future timer_entries in the free timer pool.
   */
  timer = timer_block;
  for (timer_index = 0; timer_index < OLSR_TIMER_MEMORY_CHUNK; timer_index++) {

    /* Insert new timers at the tail of the free_timer list */
    list_add_before(&free_timer_list, &timer->timer_list);

    /* 
     * For performance reasons (read: frequent timer changes),
     * precompute a random number once per timer and reuse later.
     * The random number only gets recomputed if a periodical timer fires,
     * such that a different jitter is applied for future firing.
     */
    timer->timer_random = random();

    timer++;
  }

  /*
   * There are now timers in the pool, recurse once.
   */
  return olsr_get_timer();
}


/**
 * Init datastructures for maintaining timers.
 */
void
olsr_init_timers(void)
{
  struct list_node *timer_head_node;
  int index;

  OLSR_PRINTF(5, "TIMER: init timers\n");

  memset(timer_wheel, 0, sizeof(timer_wheel));

  timer_head_node = timer_wheel;
  for (index = 0; index < TIMER_WHEEL_SLOTS; index++) {
    list_head_init(timer_head_node);
    timer_head_node++;
  }

  /*
   * Reset the last timer run.
   */
  timer_last_run = now_times;

  /* Timer memory pooling */
  list_head_init(&free_timer_list);
  timers_running = 0;
}

/*
 * olsr_get_next_list_entry
 *
 * Get the next list node in a hash bucket.
 * The listnode of the timer in may be subject to getting removed from
 * this timer bucket in olsr_change_timer() and olsr_stop_timer(), which
 * means that we can miss our walking context.
 * By caching the previous node we can figure out if the current node
 * has been removed from the hash bucket and compute the next node.
 */
static struct list_node *
olsr_get_next_list_entry (struct list_node **prev_node,
                          struct list_node *current_node)
{
  if ((*prev_node)->next == current_node) {

    /*
     * No change in the list, normal traversal, update the previous node.
     */
    *prev_node = current_node;
    return (current_node->next);
  } else {

    /*
     * List change. Recompute the walking context.
     */
    return ((*prev_node)->next);
  }
}

/**
 * Walk through the timer list and check if any timer is ready to fire.
 * Callback the provided function with the context pointer.
 */
void
olsr_walk_timers(clock_t * last_run)
{
  static struct timer_entry *timer;
  struct list_node *timer_head_node, *timer_walk_node, *timer_walk_prev_node;
  unsigned int timers_walked, timers_fired;
  unsigned int total_timers_walked, total_timers_fired;
  unsigned int wheel_slot_walks = 0;

  /*
   * Check the required wheel slots since the last time a timer walk was invoked,
   * or check *all* the wheel slots, whatever is less work.
   * The latter is meant as a safety belt if the scheduler falls behind.
   */
  total_timers_walked = total_timers_fired = timers_walked = timers_fired = 0;
  while ((*last_run <= now_times) && (wheel_slot_walks < TIMER_WHEEL_SLOTS)) {

    /* keep some statistics */
    total_timers_walked += timers_walked;
    total_timers_fired += timers_fired;
    timers_walked = 0;
    timers_fired = 0;

    /* Get the hash slot for this clocktick */
    timer_head_node = &timer_wheel[*last_run & TIMER_WHEEL_MASK];
    timer_walk_prev_node = timer_head_node;

    /* Walk all entries hanging off this hash bucket */
    for (timer_walk_node = timer_head_node->next;
         timer_walk_node != timer_head_node; /* circular list */
	 timer_walk_node = olsr_get_next_list_entry(&timer_walk_prev_node,
                                                    timer_walk_node)) {

      timer = list2timer(timer_walk_node);

      timers_walked++;

      /* Ready to fire ? */
      if (TIMED_OUT(timer->timer_clock)) {

	OLSR_PRINTF(3, "TIMER: fire %s timer %p, ctx %p, "
		    "at clocktick %u (%s)\n",
		    olsr_cookie_name(timer->timer_cookie),
		    timer, timer->timer_cb_context,
                    (unsigned int)*last_run,
                    olsr_wallclock_string());

	/* This timer is expired, call into the provided callback function */
	timer->timer_cb(timer->timer_cb_context);

	if (timer->timer_period) {

	  /*
	   * Don't restart the periodic timer if the callback function has
	   * stopped the timer.
	   */
	  if (timer->timer_flags & OLSR_TIMER_RUNNING) {

	    /* For periodical timers, rehash the random number and restart */
	    timer->timer_random = random();
	    olsr_change_timer(timer, timer->timer_period,
			      timer->timer_jitter_pct, OLSR_TIMER_PERIODIC);
	  }

	} else {

	  /*
	   * Don't stop the singleshot timer if the callback function has
	   * stopped the timer.
	   */
	  if (timer->timer_flags & OLSR_TIMER_RUNNING) {

	    /* Singleshot timers are stopped and returned to the pool */
	    olsr_stop_timer(timer);
	  }
	}

	timers_fired++;
      }
    }

    /* Increment the time slot and wheel slot walk iteration */
    (*last_run)++;
    wheel_slot_walks++;
  }

#ifdef DEBUG
  OLSR_PRINTF(3, "TIMER: processed %4u/%u clockwheel slots, "
	      "timers walked %4u/%u, timers fired %u\n",
	      wheel_slot_walks, TIMER_WHEEL_SLOTS,
	      total_timers_walked, timers_running, total_timers_fired);
#endif

  /*
   * If the scheduler has slipped and we have walked all wheel slots,
   * reset the last timer run.
   */
  *last_run = now_times;
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
  int dir;
  struct tm *gmt, *loc;
  struct tm sgmt;
  time_t t;

  if (time_diff != OLSR_TIMEZONE_UNINITIALIZED) {
    return time_diff;
  }

  t = time(NULL);
  gmt = &sgmt;
  *gmt = *gmtime(&t);
  loc = localtime(&t);

  time_diff = (loc->tm_hour - gmt->tm_hour) * 60 * 60
    + (loc->tm_min - gmt->tm_min) * 60;

  /*
   * If the year or julian day is different, we span 00:00 GMT
   * and must add or subtract a day. Check the year first to
   * avoid problems when the julian day wraps.
   */
  dir = loc->tm_year - gmt->tm_year;
  if (!dir) {
    dir = loc->tm_yday - gmt->tm_yday;
  }

  time_diff += dir * 24 * 60 * 60;

  return (time_diff);
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
  static char buf[4][sizeof("00:00:00.000000")];
  static int idx = 0;
  char *ret;
  struct timeval now;
  time_t sec, usec;

  ret = buf[idx];
  idx = (idx + 1) & 3;

  gettimeofday(&now, NULL);

  if (now.tv_sec>(60*60*24))
  sec = now.tv_sec + olsr_get_timezone();
  else
  sec = now.tv_sec;
  
  usec = now.tv_usec;
  
  if (sec<0)
    sec=0;
  if (usec<0)
    usec=0;

  snprintf(ret, sizeof(buf)/4, "%02lu:%02lu:%02lu.%06lu",	   
    (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60, usec);

  return ret;
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
olsr_clock_string(clock_t clock)
{
  static char buf[4][sizeof("00:00:00.000")];
  static int idx = 0;
  char *ret;
  clock_t sec, msec;

  ret = buf[idx];
  idx = (idx + 1) & 3;

  /* On most systems a clocktick is a 10ms quantity. */
  msec = olsr_cnf->system_tick_divider * (clock_t)(clock - now_times);
  sec = msec / MSEC_PER_SEC;

  if ((long)sec<0)
    sec = 0;

  snprintf(ret, sizeof(buf) / 4, "%02u:%02u:%02u.%03u",
	   sec / 3600, (sec % 3600) / 60, (sec % 60), (msec % MSEC_PER_SEC));

  return ret;
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
olsr_start_timer(unsigned int rel_time, olsr_u8_t jitter_pct,
		 olsr_bool periodical, void (*timer_cb_function) (void *),
		 void *context, olsr_cookie_t cookie)
{
  struct timer_entry *timer;

  timer = olsr_get_timer();

  /* Fill entry */
  timer->timer_clock = olsr_jitter(rel_time, jitter_pct, timer->timer_random);
  timer->timer_cb = timer_cb_function;
  timer->timer_cb_context = context;
  timer->timer_jitter_pct = jitter_pct;
  timer->timer_flags = OLSR_TIMER_RUNNING;

  /* The cookie is used for debugging to traceback the originator */
  timer->timer_cookie = cookie;
  olsr_cookie_usage_incr(cookie);

  /* Singleshot or periodical timer ? */
  if (periodical) {
    timer->timer_period = rel_time;
  } else {
    timer->timer_period = 0;
  }

  /*
   * Now insert in the respective timer_wheel slot.
   */
  list_add_before(&timer_wheel[timer->timer_clock & TIMER_WHEEL_MASK],
		  &timer->timer_list);
  timers_running++;

#ifdef DEBUG
  OLSR_PRINTF(3, "TIMER: start %s timer %p firing in %s, ctx %p\n",
	      olsr_cookie_name(timer->timer_cookie),
	      timer, olsr_clock_string(timer->timer_clock), context);
#endif

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

  /* sanity check */
  if (!timer) {
    return;
  }
#ifdef DEBUG
  OLSR_PRINTF(3, "TIMER: stop %s timer %p, ctx %p\n",
	      olsr_cookie_name(timer->timer_cookie),
	      timer, timer->timer_cb_context);
#endif

  /*
   * Carve out of the existing wheel_slot and return to the pool
   * rather than freeing for later reycling.
   */
  list_remove(&timer->timer_list);
  list_add_before(&free_timer_list, &timer->timer_list);
  timer->timer_flags &= ~OLSR_TIMER_RUNNING;
  olsr_cookie_usage_decr(timer->timer_cookie);
  timers_running--;
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
olsr_change_timer(struct timer_entry *timer, unsigned int rel_time,
		  olsr_u8_t jitter_pct, olsr_bool periodical)
{

  /* Sanity check. */
  if (!timer) {
    return;
  }

  /* Singleshot or periodical timer ? */
  if (periodical) {
    timer->timer_period = rel_time;
  } else {
    timer->timer_period = 0;
  }

  timer->timer_clock = olsr_jitter(rel_time, jitter_pct, timer->timer_random);
  timer->timer_jitter_pct = jitter_pct;

  /*
   * Changes are easy: Remove timer from the exisiting timer_wheel slot
   * and reinsert into the new slot.
   */
  list_remove(&timer->timer_list);
  list_add_before(&timer_wheel[timer->timer_clock & TIMER_WHEEL_MASK],
		  &timer->timer_list);

#ifdef DEBUG
  OLSR_PRINTF(3, "TIMER: change %s timer %p, firing to %s, ctx %p\n",
	      olsr_cookie_name(timer->timer_cookie), timer,
	      olsr_clock_string(timer->timer_clock), timer->timer_cb_context);
#endif
}


/*
 * This is the one stop shop for all sort of timer manipulation.
 * Depending on the paseed in parameters a new timer is started,
 * or an existing timer is started or an existing timer is
 * terminated.
 */
void
olsr_set_timer(struct timer_entry **timer_ptr, unsigned int rel_time,
	       olsr_u8_t jitter_pct, olsr_bool periodical,
	       void (*timer_cb_function) (void *), void *context,
	       olsr_cookie_t cookie)
{

  if (!*timer_ptr) {

    /* No timer running, kick it. */
    *timer_ptr = olsr_start_timer(rel_time, jitter_pct, periodical,
				  timer_cb_function, context, cookie);
  } else {

    if (!rel_time) {

      /* No good future time provided, kill it. */
      olsr_stop_timer(*timer_ptr);
      *timer_ptr = NULL;
    } else {

      /* Time is ok and timer is running, change it ! */
      olsr_change_timer(*timer_ptr, rel_time, jitter_pct, periodical);
    }
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
