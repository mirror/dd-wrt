/*
 * Timers and signals wrapper
 *
 * $Id: timers.h,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#ifndef TIMERS_H
#define TIMERS_H

/* System includes needed for types */
#include <time.h>

/* Local includes needed for types */
#include "units.h"

/* Type definitions */
typedef struct {
  const Unit_t *unit;
  unsigned int alarm_time;
  void *data;
} Timer_t;

/* Global function prototypes */
Timer_t *timer_new(const Unit_t *unit);
void timer_free(const Unit_t *unit, Timer_t *timer);
void timer_alarm(const Unit_t *unit, Timer_t *timer, unsigned int nsecs, 
		 void *data);
void timer_ack(const Unit_t *unit, Timer_t *timer);
Timer_t *timer_find_soonest(const Unit_t *unit);

/* Global data */
extern const Unit_t timer_unit;
extern volatile int sig_hup, sig_usr1, sig_usr2, sig_alarm;

#endif

