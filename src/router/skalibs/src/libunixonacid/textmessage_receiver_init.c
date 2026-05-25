/* ISC license. */

#include <skalibs/buffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/textmessage.h>

int textmessage_receiver_init (textmessage_receiver *tr, int fd, char *buf, size_t buflen, uint32_t max)
{
  if (!buffer_init(&tr->in, &buffer_read, fd, buf, buflen)) return 0 ;
  tr->indata = stralloc_zero ;
  tr->wanted = 0 ;
  tr->max = max ;
  return 1 ;
}
