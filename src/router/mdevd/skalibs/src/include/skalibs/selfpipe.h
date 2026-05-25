/* ISC license. */

#ifndef SKALIBS_SELFPIPE_H
#define SKALIBS_SELFPIPE_H

#include <signal.h>

extern int selfpipe_init (void) ;
extern int selfpipe_trap (int) ;
extern int selfpipe_trapset (sigset_t const *) ;
extern int selfpipe_fd (void) ;
extern int selfpipe_read (void) ;
extern void selfpipe_finish (void) ;

#endif
