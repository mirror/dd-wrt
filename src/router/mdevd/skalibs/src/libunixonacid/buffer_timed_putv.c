/* ISC license. */

#include <sys/uio.h>

#include <skalibs/buffer.h>
#include <skalibs/siovec.h>
#include <skalibs/unix-timed.h>

size_t buffer_timed_putv (buffer *b, struct iovec const *v, unsigned int vlen, tain const *deadline, tain *stamp)
{
  if (!vlen) return 0 ;
  size_t tot = siovec_len(v, vlen) ;
  size_t w = 0 ;
  struct iovec vv[vlen] ;
  for (unsigned int i = 0 ; i < vlen ; i++) vv[i] = v[i] ;
  for (;;)
  {
    size_t r = buffer_putvnoflush(b, vv, vlen) ;
    w += r ; siovec_seek(vv, vlen, r) ;
    if (w >= tot || !buffer_timed_flush(b, deadline, stamp)) break ;
  }
  return w ;
}
