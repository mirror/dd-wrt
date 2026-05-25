/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <sys/uio.h>

int main (void)
{
  char s[2][2] ;
  struct iovec v[2] = { { .iov_base = s[0], .iov_len = 2 }, { .iov_base = s[1], .iov_len = 2 } } ;
  loff_t in, out ;
  ssize_t r = splice(0, &in, 1, &out, 4096, SPLICE_F_MOVE) ;
  r = tee(0, 1, 4096, SPLICE_F_NONBLOCK) ;
  r = vmsplice(0, v, 2, 0) ;
  return 0 ;
}
