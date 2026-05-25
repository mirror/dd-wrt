/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE
#endif

#ifndef _INCOMPLETE_XOPEN_C063
#define _INCOMPLETE_XOPEN_C063
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#include <sys/stat.h>
#include <fcntl.h>

int main (void)
{
  int fd = openat(0, "/", O_RDONLY) ;
  return (fd < 0) ;
}
