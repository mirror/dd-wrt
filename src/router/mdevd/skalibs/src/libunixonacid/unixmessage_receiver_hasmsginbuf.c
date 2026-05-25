/* ISC license. */

#include <stdint.h>
#include <sys/uio.h>
#include <skalibs/uint32.h>
#include <skalibs/siovec.h>
#include <skalibs/cbuffer.h>
#include <skalibs/unixmessage.h>

int unixmessage_receiver_hasmsginbuf (unixmessage_receiver const *b)
{
  size_t len = cbuffer_len(&b->mainb) ;
  uint32_t mainlen ;
  char pack[4] ;
  struct iovec v[2] ;
  if (len < 6) return 0 ;
  cbuffer_rpeek(&b->mainb, v) ;
  siovec_gather(v, 2, pack, 4) ;
  uint32_unpack_big(pack, &mainlen) ;
  return len - 6 >= mainlen ;
}
