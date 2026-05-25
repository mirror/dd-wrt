/* ISC license. */

#include <sys/uio.h>
#include <errno.h>

#include <skalibs/textclient.h>
#include <skalibs/posixishard.h>

int textclient_command (textclient *a, char const *s, size_t len, tain const *deadline, tain *stamp)
{
  struct iovec ans ;
  if (!textclient_exchange(a, s, len, &ans, deadline, stamp)) return 0 ;
  if (ans.iov_len != 1) return (errno = EPROTO, 0) ;
  if (*(unsigned char *)ans.iov_base) return (errno = *(unsigned char *)ans.iov_base, 0) ;
  return 1 ;
}
