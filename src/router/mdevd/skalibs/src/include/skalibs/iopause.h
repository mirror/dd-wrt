/* ISC license. */

#ifndef SKALIBS_IOPAUSE_H
#define SKALIBS_IOPAUSE_H

#include <poll.h>

#include <skalibs/tai.h>

typedef struct pollfd iopause_fd, *iopause_fd_ref ;

#define IOPAUSE_READ (POLLIN|POLLHUP)
#define IOPAUSE_WRITE POLLOUT
#define IOPAUSE_EXCEPT (POLLERR|POLLHUP|POLLNVAL)

typedef int iopause_func (iopause_fd *, unsigned int, tain const *, tain const *) ;
typedef iopause_func *iopause_func_ref ;

extern iopause_func iopause_select ;
extern iopause_func iopause_poll ;
extern iopause_func iopause_ppoll ;

extern iopause_func_ref const iopause_ ;
#define iopause (*iopause_)

extern int iopause_stamp (iopause_fd *, unsigned int, tain const *, tain *) ;
#define iopause_g(x, n, deadline) iopause_stamp(x, n, (deadline), &STAMP)

extern void deepsleepuntil (tain const *, tain *) ;
#define deepsleepuntil_g(deadline) deepsleepuntil((deadline), &STAMP)

#endif
