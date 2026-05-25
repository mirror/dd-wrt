/* ISC license. */

#include <sys/mman.h>
#include <errno.h>

#include <skalibs/posixplz.h>

void munmap_void (void *addr, size_t len)
{
  int e = errno ;
  munmap(addr, len) ;
  errno = e ;
}
