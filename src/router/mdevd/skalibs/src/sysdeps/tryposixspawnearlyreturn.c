/* ISC license. */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <spawn.h>

int main (void)
{
  posix_spawnattr_t attr ;
  int e ;
  char tmp[40] = "./tryposixspawnearlyreturn child:XXXXXX" ;
  char *argv[2] = { tmp, 0 } ;
  char *env = 0 ;
  int fd = mkstemp(tmp) ;
  if (fd == -1) return 111 ;
  e = posix_spawnattr_init(&attr) ;
  if (e) return 111 ;
  e = posix_spawnattr_setflags(&attr, 0) ;
  if (e) return 111 ;
  if (close(fd) == -1 || unlink(tmp) == -1) return 111 ;
  e = posix_spawn(0, argv[0], 0, &attr, argv, &env) ;
  return e ? e == ENOENT ? 1 : 111 : 0 ;
}
