/*
 * Timers and signals wrapper
 *
 * $Id: timers.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#define _POSIX_SOURCE 1
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>

/* Local includes */
#include "timers.h"
#include "load.h"
#include "dump.h"
#include "mem.h"

/* Type definitions */
typedef struct _TimerList_t {
  Timer_t *timer;
  struct _TimerList_t *next;
} TimerList_t;

/* Local function prototypes */
static void timer_init0(void);
static void timer_init1(void);
static void timer_dump(void);
static void timer_release(void);
static void hup_handler(int nsig);
static void usr1_handler(int nsig);
static void usr2_handler(int nsig);
static void alarm_handler(int nsig);

/* Data */
#define TIMER_MAX 65535

static const char *rcsid="$Id: timers.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $";

const Unit_t timer_unit = {
  "timer",
  &timer_init0,
  &timer_init1,
  &timer_dump,
  &timer_release
};

static TimerList_t *timerlist;
volatile int sig_hup, sig_usr1, sig_usr2, sig_alarm;

/* Functions */

/* Initialize local data */
static void
timer_init0(void)
{
  struct sigaction sig;

  sig_hup = sig_usr1 = sig_usr2 = sig_alarm = 0;

  sig.sa_handler = hup_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGHUP, &sig, NULL);

  sig.sa_handler = usr1_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGUSR1, &sig, NULL);

  sig.sa_handler = usr2_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGUSR2, &sig, NULL);

  sig.sa_handler = alarm_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGALRM, &sig, NULL);

  timerlist = NULL;
}

/* Initialization for data that needs other units */
static void
timer_init1(void)
{
  set_var_str(&timer_unit, "version", rcsid);
  Debug_unit(&timer_unit, "Initialized.");
}

/* Dump status, local data etc. */
static void
timer_dump(void)
{
  TimerList_t *tmp;

  for (tmp = timerlist; tmp != NULL; tmp = tmp->next) {
    assert(tmp->timer != NULL);
    assert(tmp->timer->unit != NULL);
    assert(tmp->timer->unit->name != NULL);
    Debug_unit(&timer_unit, "unit %s alarm %d data 0x%x", tmp->timer->unit->name, tmp->timer->alarm_time, tmp->timer->data);
  }
}

/* Release allocated memory, close files etc. */
static void
timer_release(void)
{
  TimerList_t *tmp;

  alarm(0);
  for (tmp = timerlist; tmp != NULL; ) {
    assert(tmp->timer != NULL);
    assert(tmp->timer->unit != NULL);
    assert(tmp->timer->unit->name != NULL);
    Debug_unit(&timer_unit, "discarding timer, unit %s alarm %d data 0x%x", tmp->timer->unit->name, tmp->timer->alarm_time, tmp->timer->data);
    tmp = timerlist->next;
    mem_free(&timer_unit, timerlist->timer);
    mem_free(&timer_unit, timerlist);
    timerlist = tmp;
  }
}

Timer_t *
timer_new(const Unit_t *unit)
{
  Timer_t *timer;
  TimerList_t *tmp;

  timer = (Timer_t *)mem_alloc(&timer_unit, sizeof(Timer_t));
  timer->alarm_time = INT_MAX;
  timer->unit = unit;
  tmp = (TimerList_t *)mem_alloc(&timer_unit, sizeof(TimerList_t *));
  tmp->timer = timer;
  tmp->next = timerlist;
  timerlist = tmp;
  timer_dump();
  return (Timer_t *)timer;
}

void
timer_free(const Unit_t *unit, Timer_t *timer)
{
  TimerList_t *tmp, *prev = NULL;

  for (tmp = timerlist; tmp != NULL; tmp = tmp->next) {
    assert(tmp->timer != NULL);
    if (tmp->timer == timer) {
      mem_free(&timer_unit, tmp->timer);
      if (prev != NULL) {
	prev->next = tmp->next;
      }
      else {
	timerlist = tmp->next;
      }
      mem_free(&timer_unit, tmp);
      return;
    }
    prev = tmp;
  }
}

/* Delay for the specified period */
void
timer_alarm(const Unit_t *unit, Timer_t *timer, unsigned int nsecs, void *data)
{
  assert (timer != NULL);
  timer->data = data;
  timer->alarm_time = time(NULL) + nsecs;
}

/* Acknowledge alarm */
void
timer_ack(const Unit_t *unit, Timer_t *timer)
{
  assert (timer != NULL);
  timer->alarm_time = INT_MAX;
}

/* Find soonest timer expiration */
Timer_t *
timer_find_soonest(const Unit_t *unit)
{
  TimerList_t *tmp;
  Timer_t *lowest = NULL;

  for (tmp = timerlist; tmp != NULL; tmp = tmp->next ) {
    assert(tmp->timer != NULL);
    if (lowest == NULL || tmp->timer->alarm_time < lowest->alarm_time) {
      lowest = tmp->timer;
    }
  }
  if (lowest == NULL) {
    return NULL;
  }
  Debug_unit(&timer_unit, "lowest: %d", lowest->alarm_time);
  return lowest;
}

/* Handler for signal HUP, restart */
static void
hup_handler(int nsig)
{
  Debug_unit(&timer_unit, "Hup");
  sig_hup = 1;
}

/* Handler for signal USR1, status dump */
static void
usr1_handler(int nsig)
{
  Debug_unit(&timer_unit, "Usr1");
  sig_usr1 = 1;
}

/* Handler for signal INT, exit */
static void
usr2_handler(int nsig)
{
  Debug_unit(&timer_unit, "Int");
  sig_usr2 = 1;
}

/* Handler for signal ALARM, timer expiration */
static void
alarm_handler(int nsig)
{
  Debug_unit(&timer_unit, "Alarm");
  sig_alarm = 1;
}

