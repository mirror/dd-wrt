/* ISC license. */

#ifndef SKALIBS_SEGFAULT_H
#define SKALIBS_SEGFAULT_H

extern int sigsegv (void) ;
extern int sigfpe (void) ;

#define segfault() sigsegv()

#endif
