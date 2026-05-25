/* ISC license. */

#ifndef SKALIBS_KEVENTBRIDGE_H
#define SKALIBS_KEVENTBRIDGE_H

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASKEVENT

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct keventbridge_s keventbridge, *keventbridge_ref ;
struct keventbridge_s
{
  pthread_t th ;
  int kq ;
  int p[2] ;
} ;
#define KEVENTBRIDGE_ZERO { .kq = -1, .p = { -1, -1 } }

extern int keventbridge_start (keventbridge *) ;
#define keventbridge_write(kb, ke, n) kevent((kb)->kq, ke, (n), 0, 0, 0)
extern int keventbridge_read (keventbridge const *, struct kevent *) ;
extern void keventbridge_end (keventbridge *) ;

#endif
#endif
