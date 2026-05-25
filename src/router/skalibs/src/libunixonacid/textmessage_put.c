/* ISC license. */

#include <sys/uio.h>
#include <errno.h>
#include <stdint.h>
#include <skalibs/uint32.h>
#include <skalibs/bufalloc.h>
#include <skalibs/textmessage.h>

int textmessage_put (textmessage_sender *ts, char const *s, size_t len)
{
  char pack[4] ;
  struct iovec v[2] =
  {
    { .iov_base = pack, .iov_len = 4 },
    { .iov_base = (char *)s, .iov_len = len }
  } ;
  if (len > TEXTMESSAGE_MAXLEN) return (errno = EINVAL, 0) ;
  uint32_pack_big(pack, (uint32_t)len) ;
  return bufalloc_putv(&ts->out, v, 2) ;
}
