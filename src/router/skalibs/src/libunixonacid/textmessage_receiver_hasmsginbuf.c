/* ISC license. */

#include <stdint.h>
#include <sys/uio.h>
#include <skalibs/uint32.h>
#include <skalibs/siovec.h>
#include <skalibs/buffer.h>
#include <skalibs/textmessage.h>

int textmessage_receiver_hasmsginbuf (textmessage_receiver const *tr)
{
  size_t len = buffer_len(&tr->in) ;
  uint32_t n ;
  char pack[4] ;
  struct iovec v[2] ;
  if (len < 4) return 0 ;
  buffer_rpeek(&tr->in, v) ;
  siovec_gather(v, 2, pack, 4) ;
  uint32_unpack_big(pack, &n) ;
  return len - 4 >= n ;
}
