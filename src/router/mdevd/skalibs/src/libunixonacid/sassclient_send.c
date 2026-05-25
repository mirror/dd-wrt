/* ISC license. */

#include <sys/uio.h>

#include <skalibs/sassclient.h>

int sassclient_send (sassclient *a, uint32_t *id, uint32_t flags, uint32_t timeout, uint32_t opcode, char const *s, size_t len, sassclient_cb_func_ref cb, void *data, tain const *deadline, tain *stamp)
{
  struct iovec v = { .iov_base = (char *)s, .iov_len = len } ;
  return sassclient_sendv(a, id, flags, timeout, opcode, &v, 1, cb, data, deadline, stamp) ;
}
