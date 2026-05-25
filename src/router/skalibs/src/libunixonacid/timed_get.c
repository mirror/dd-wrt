/* ISC license. */

#include <errno.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/iopause.h>
#include <skalibs/unix-timed.h>

ssize_t timed_get (void *b, init_func_ref getfd, get_func_ref get, tain const *deadline, tain *stamp)
{
  iopause_fd x = { .fd = (*getfd)(b), .events = IOPAUSE_READ, .revents = 0 } ;
  ssize_t r = (*get)(b) ;
  while (!r)
  {
    r = iopause_stamp(&x, 1, deadline, stamp) ;
    if (!r) return (errno = ETIMEDOUT, -1) ;
    else if (r > 0 && x.revents & (IOPAUSE_READ | IOPAUSE_EXCEPT)) r = (*get)(b) ;
  }
  return unsanitize_read(r) ;
}
