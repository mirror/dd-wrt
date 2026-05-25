/* ISC license. */

#ifndef SKALIBS_ALARM_H
#define SKALIBS_ALARM_H

#include <skalibs/tai.h>

extern int alarm_milliseconds (unsigned int) ;
extern int alarm_timeout (tain const *) ;
extern int alarm_deadline (tain const *) ;
extern void alarm_disable (void) ;

#endif
