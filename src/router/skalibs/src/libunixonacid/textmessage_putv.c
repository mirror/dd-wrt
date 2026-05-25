/* ISC license. */

#include <sys/uio.h>
#include <errno.h>
#include <stdint.h>
#include <skalibs/uint32.h>
#include <skalibs/bufalloc.h>
#include <skalibs/siovec.h>
#include <skalibs/textmessage.h>

int textmessage_putv (textmessage_sender *ts, struct iovec const *v, unsigned int n)
{
  size_t len = siovec_len(v, n) ;
  char pack[4] ;
  struct iovec vv[n+1] ;
  if (len > TEXTMESSAGE_MAXLEN) return (errno = EINVAL, 0) ;
  vv[0].iov_base = pack ;
  vv[0].iov_len = 4 ;
  for (unsigned int i = 0 ; i < n ; i++) vv[i+1] = v[i] ;
  uint32_pack_big(pack, (uint32_t)len) ;
  return bufalloc_putv(&ts->out, vv, n+1) ;
}
