/* ISC license. */

#include <sys/uio.h>
#include <stdint.h>
#include <errno.h>

#include <skalibs/uint32.h>
#include <skalibs/textclient.h>

#include "sassclient-internal.h"

int sassclient_cancel_internal (sassclient *a, uint32_t id, tain const *deadline, tain *stamp)
{
  struct iovec answer ;
  uint32_t ans ;
  char pack[5] = "-" ;

  uint32_pack_big(pack + 1, id) ;
  if (!textclient_exchange(&a->connection, pack, 5, &answer, deadline, stamp)) return errno ;
  if (answer.iov_len != 4) return EPROTO ;
  uint32_unpack_big((char *)answer.iov_base, &ans) ;
  return ans ;
}
