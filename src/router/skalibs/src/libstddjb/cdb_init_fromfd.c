/* ISC license. */

#include <skalibs/bsdsnowflake.h>

#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#include <skalibs/cdb.h>

int cdb_init_fromfd (cdb *c, int fd)
{
  char *map ;
  struct stat st ;
  if (fstat(fd, &st) < 0) return 0 ;
  if (st.st_size > UINT32_MAX) return (errno = EOVERFLOW, 0) ;
  map = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0) ;
  if (map == MAP_FAILED) return 0 ;
  c->map = map ;
  c->size = st.st_size ;
  return 1 ;
}
