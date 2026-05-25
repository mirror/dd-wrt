/* ISC license. */

#include <errno.h>
#include <skalibs/error.h>
#include <skalibs/iopause.h>
#include <skalibs/unix-timed.h>

int timed_flush (void *b, init_func_ref getfd, init_func_ref isnonempty, init_func_ref flush, tain const *deadline, tain *stamp)
{
  iopause_fd x = { .fd = (*getfd)(b), .events = IOPAUSE_WRITE, .revents = 0 } ;
  while ((*isnonempty)(b))
  {
    int r = iopause_stamp(&x, 1, deadline, stamp) ;
    if (r < 0) return 0 ;
    else if (!r) return (errno = ETIMEDOUT, 0) ;
    else if (x.revents & IOPAUSE_WRITE)
    {
      if (!((*flush)(b)) && !error_isagain(errno)) return 0 ;
    }
    else if (x.revents & IOPAUSE_EXCEPT) return (*flush)(b) ;
  }
  return 1 ;
}
