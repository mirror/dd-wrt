/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/file.h>
#include <fcntl.h>

int main (void)
{
  return flock(0, LOCK_EX | LOCK_UN | LOCK_NB) ;
}
