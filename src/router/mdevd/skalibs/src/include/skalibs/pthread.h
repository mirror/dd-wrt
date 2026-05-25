/* ISC license. */

#ifndef SKALIBS_PTHREAD_H
#define SKALIBS_PTHREAD_H

#include <pthread.h>

#include <skalibs/tai.h>

extern int pthread_mutex_tailock (pthread_mutex_t *, tain const *, tain *) ;
#define pthread_mutex_tailock_g(mtx, deadline) pthread_mutex_tailock(mtx, (deadline), &STAMP)

#endif
