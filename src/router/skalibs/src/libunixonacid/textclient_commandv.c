/* ISC license. */

#include <sys/uio.h>
#include <errno.h>

#include <skalibs/textclient.h>
#include <skalibs/posixishard.h>

int textclient_commandv (textclient *a, struct iovec const *v, unsigned int n, tain const *deadline, tain *stamp)
{
  struct iovec ans ;
  if (!textclient_exchangev(a, v, n, &ans, deadline, stamp)) return 0 ;
  if (ans.iov_len != 1) return (errno = EPROTO, 0) ;
  if (*(unsigned char *)ans.iov_base) return (errno = *(unsigned char *)ans.iov_base, 0) ;
  return 1 ;
}
