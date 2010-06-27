/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 2001-2010 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * BUT witHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/*
 * Timer system, based on alarm() and SIGALRM
 * $Id: timers.c,v 1.32 2010/02/09 20:08:09 castaglia Exp $
 */

#include "conf.h"

#include <signal.h>

/* From src/main.c */
volatile extern unsigned int recvd_signal_flags;

struct timer {
  struct timer *next, *prev;

  long count;                   /* Amount of time remaining */
  long interval;                /* Original length of timer */

  int timerno;                  /* Caller dependent timer number */
  module *mod;                  /* Module owning this timer */
  callback_t callback;          /* Function to callback */
  char remove;                  /* Internal use */

  const char *desc;		/* Description of timer, provided by caller */
};

static int _current_timeout = 0;
static int _total_time = 0;
static int _sleep_sem = 0;
static int alarms_blocked = 0, alarm_pending = 0;
static xaset_t *timers = NULL;
static xaset_t *recycled = NULL;
static xaset_t *free_timers = NULL;
static int _indispatch = 0;
static int dynamic_timerno = 1024;
static unsigned int nalarms = 0;
static time_t _alarmed_time = 0;

static pool *timer_pool = NULL;

static int timer_cmp(struct timer *t1, struct timer *t2) {
  if (t1->count < t2->count)
    return -1;

  if (t1->count > t2->count)
    return 1;

  return 0;
}

/* This function does the work of iterating through the list of registered
 * timers, checking to see if their callbacks should be invoked and whether
 * they should be removed from the registration list. Its return value is
 * the amount of time remaining on the first timer in the list.
 */
static int process_timers(int elapsed) {
  struct timer *t = NULL, *next = NULL;

  if (!recycled)
    recycled = xaset_create(timer_pool, NULL);

  if (!elapsed &&
      !recycled->xas_list) {

    if (!timers)
      return 0;

    return (timers->xas_list ? ((struct timer *) timers->xas_list)->count : 0);
  }

  /* Critical code, no interruptions please */
  if (_indispatch)
    return 0;

  pr_alarms_block();
  _indispatch++;

  if (elapsed) {
    for (t = (struct timer *) timers->xas_list; t; t = next) {
      /* If this timer has already been handled, skip */
      next = t->next;

      if (t->remove) {
        /* Move the timer onto the free_timers chain, for later reuse. */
        xaset_remove(timers, (xasetmember_t *) t);
        xaset_insert(free_timers, (xasetmember_t *) t);

      } else if ((t->count -= elapsed) <= 0) {
        /* This timer's interval has elapsed, so trigger its callback. */

        pr_trace_msg("timer", 4,
          "%ld %s for timer ID %d ('%s', for module '%s') elapsed, invoking "
          "callback (%p)", t->interval,
          t->interval != 1 ? "seconds" : "second", t->timerno,
          t->desc ? t->desc : "<unknown>",
          t->mod ? t->mod->name : "<none>", t->callback);

        if (t->callback(t->interval, t->timerno, t->interval - t->count,
            t->mod) == 0) {

          /* A return value of zero means this timer is done, and can be
           * removed.
           */
          xaset_remove(timers, (xasetmember_t *) t);
          xaset_insert(free_timers, (xasetmember_t *) t);

        } else {
          /* A non-zero return value from a timer callback signals that
           * the timer should be reused/restarted.
           */
          pr_trace_msg("timer", 6, "restarting timer ID %d ('%s'), as per "
            "callback", t->timerno, t->desc ? t->desc : "<unknown>");

          xaset_remove(timers, (xasetmember_t *) t);
          t->count = t->interval;
          xaset_insert(recycled, (xasetmember_t *) t);
        }
      }
    }
  }

  /* Put the recycled timers back into the main timer list. */
  while ((t = (struct timer *) recycled->xas_list) != NULL) {
    xaset_remove(recycled, (xasetmember_t *) t);
    xaset_insert_sort(timers, (xasetmember_t *) t, TRUE);
  }

  _indispatch--;
  pr_alarms_unblock();

  /* If no active timers remain in the list, there is no reason to set the
   * SIGALRM handle.
   */
  return (timers->xas_list ? ((struct timer *) timers->xas_list)->count : 0);
}

static RETSIGTYPE sig_alarm(int signo) {
  struct sigaction act;

  act.sa_handler = sig_alarm;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT;
#endif

  /* Install this handler for SIGALRM. */
  sigaction(SIGALRM, &act, NULL);

#ifdef HAVE_SIGINTERRUPT
  siginterrupt(SIGALRM, 1);
#endif

  recvd_signal_flags |= RECEIVED_SIG_ALRM;
  nalarms++;

  /* Reset the alarm */
  _total_time += _current_timeout;
  if (_current_timeout) {
    _alarmed_time = time(NULL);
    alarm(_current_timeout);
  }
}

static void set_sig_alarm(void) {
  struct sigaction act;

  act.sa_handler = sig_alarm;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT;
#endif

  /* Install this handler for SIGALRM. */
  sigaction(SIGALRM, &act, NULL);

#ifdef HAVE_SIGINTERRUPT
  siginterrupt(SIGALRM, 1);
#endif
}

void handle_alarm(void) {
  int new_timeout = 0;

  /* We need to adjust for any time that might be remaining on the alarm,
   * in case we were called in order to change alarm durations.  Note
   * that rapid-fire calling of this function will probably screw
   * up the already poor resolution of alarm() _horribly_.  Oh well,
   * this shouldn't be used for any precise work anyway, it's only
   * for modules to perform approximate timing.
   */

  /* It's possible that alarms are blocked when this function is
   * called, if so, increment alarm_pending and exit swiftly
   */
  while (nalarms) {
    nalarms = 0;

    if (!alarms_blocked) {
      int alarm_elapsed;

      alarm(0);
      alarm_elapsed = _alarmed_time ? (int) time(NULL) - _alarmed_time : 0;
      new_timeout = _total_time + alarm_elapsed;
      _total_time = 0;
      new_timeout = process_timers(new_timeout);

      _alarmed_time = time(NULL);
      alarm(_current_timeout = new_timeout);

    } else
      alarm_pending++;
  }
}

int pr_timer_reset(int timerno, module *mod) {
  struct timer *t = NULL;

  if (!timers) {
    errno = EPERM;
    return -1;
  }

  if (_indispatch) {
    errno = EINTR;
    return -1;
  }

  pr_alarms_block();

  if (!recycled)
    recycled = xaset_create(timer_pool, NULL);

  for (t = (struct timer *) timers->xas_list; t; t = t->next) {
    if (t->timerno == timerno &&
        (t->mod == mod || mod == ANY_MODULE)) {
      t->count = t->interval;
      xaset_remove(timers, (xasetmember_t *) t);
      xaset_insert(recycled, (xasetmember_t *) t);
      nalarms++;

      /* The handle_alarm() function also readjusts the timers lists
       * as part of its processing, so it needs to be called when a timer
       * is reset.
       */
      handle_alarm();
      break;
    }
  }

  pr_alarms_unblock();

  if (t) {
    pr_trace_msg("timer", 7, "reset timer ID %d ('%s', for module '%s')",
      t->timerno, t->desc, t->mod ? t->mod->name : "[none]");
    return t->timerno;
  }

  return (t ? t->timerno : 0);
}

int pr_timer_remove(int timerno, module *mod) {
  struct timer *t = NULL;

  /* If there are no timers currently registered, do nothing. */
  if (!timers)
    return 0;

  pr_alarms_block();

  for (t = (struct timer *) timers->xas_list; t; t = t->next) {
    if (t->timerno == timerno &&
        (t->mod == mod || mod == ANY_MODULE)) {
      if (_indispatch) {
        t->remove++;

      } else {
        xaset_remove(timers, (xasetmember_t *) t);
        xaset_insert(free_timers, (xasetmember_t *) t);
	nalarms++;

        /* The handle_alarm() function also readjusts the timers lists
         * as part of its processing, so it needs to be called when a timer
         * is removed.
         */
        handle_alarm();
      }
      break;
    }
  }

  pr_alarms_unblock();

  if (t) {
    pr_trace_msg("timer", 7, "removed timer ID %d ('%s', for module '%s')",
      t->timerno, t->desc, t->mod ? t->mod->name : "[none]");
    return t->timerno;
  }

  errno = ENOENT;
  return -1;
}

int pr_timer_add(int seconds, int timerno, module *mod, callback_t cb,
    const char *desc) {
  struct timer *t = NULL;

  if (seconds <= 0 ||
      cb == NULL ||
      desc == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (!timers)
    timers = xaset_create(timer_pool, (XASET_COMPARE) timer_cmp);

  /* Check to see that, if specified, the timerno is not already in use. */
  if (timerno != -1) {
    for (t = (struct timer *) timers->xas_list; t; t = t->next) {
      if (t->timerno == timerno) {
        errno = EPERM;
        return -1;
      }
    }
  }

  if (!free_timers)
    free_timers = xaset_create(timer_pool, NULL);

  /* Try to use an old timer first */
  pr_alarms_block();
  t = (struct timer *) free_timers->xas_list;
  if (t != NULL) {
    xaset_remove(free_timers, (xasetmember_t *) t);

  } else {

    if (timer_pool == NULL) {
      timer_pool = make_sub_pool(permanent_pool);
      pr_pool_tag(timer_pool, "Timer Pool");
    }

    /* Must allocate a new one */
    t = palloc(timer_pool, sizeof(struct timer));
  }

  if (timerno == -1) {
    /* Dynamic timer */
    if (dynamic_timerno < 1024)
      dynamic_timerno = 1024;
    timerno = dynamic_timerno++;
  }

  t->timerno = timerno;
  t->count = t->interval = seconds;
  t->callback = cb;
  t->mod = mod;
  t->remove = 0;
  t->desc = desc;

  /* If called while _indispatch, add to the recycled list to prevent
   * list corruption
   */

  if (_indispatch) {
    if (!recycled)
      recycled = xaset_create(timer_pool, NULL);
    xaset_insert(recycled, (xasetmember_t *) t);

  } else {
    xaset_insert_sort(timers, (xasetmember_t *) t, TRUE);
    nalarms++;
    set_sig_alarm();

    /* The handle_alarm() function also readjusts the timers lists
     * as part of its processing, so it needs to be called when a timer
     * is added.
     */
    handle_alarm();
  }

  pr_alarms_unblock();

  pr_trace_msg("timer", 7, "added timer ID %d ('%s', for module '%s'), "
    "triggering in %ld %s", t->timerno, t->desc,
    t->mod ? t->mod->name : "[none]", t->interval,
    t->interval != 1 ? "seconds" : "second");
  return timerno;
}

/* Alarm blocking.  This is done manually rather than with syscalls,
 * so as to allow for easier signal handling, portability and
 * detecting the number of blocked alarms, as well as nesting the
 * block/unblock functions.
 */

void pr_alarms_block(void) {
  ++alarms_blocked;
}

void pr_alarms_unblock(void) {
  --alarms_blocked;
  if (alarms_blocked == 0 && alarm_pending) {
    alarm_pending = 0;
    nalarms++;
    handle_alarm();
  }
}

static int sleep_cb(CALLBACK_FRAME) {
  _sleep_sem++;
  return 0;
}

int pr_timer_sleep(int seconds) {
  int timerno = 0;
  sigset_t oset;

  _sleep_sem = 0;

  if (alarms_blocked || _indispatch) {
    errno = EPERM;
    return -1;
  }

  timerno = pr_timer_add(seconds, -1, NULL, sleep_cb, "sleep");
  if (timerno == -1)
    return -1;

  sigemptyset(&oset);
  while (!_sleep_sem) {
    sigsuspend(&oset);
    handle_alarm();
  }

  return 0;
}

void timers_init(void) {

  /* Reset some of the key static variables. */
  _current_timeout = 0;
  _total_time = 0;
  nalarms = 0;
  _alarmed_time = 0;

  /* Don't inherit the parent's timer lists. */
  timers = NULL;
  recycled = NULL;
  free_timers = NULL;

  /* Reset the timer pool. */
  if (timer_pool)
    destroy_pool(timer_pool);

  timer_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(timer_pool, "Timer Pool");

  return;
}
