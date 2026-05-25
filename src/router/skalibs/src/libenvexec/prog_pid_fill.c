/* ISC license. */

#include <string.h>
#include <unistd.h>

#include <skalibs/types.h>
#include <skalibs/prog.h>

void prog_pid_fill (char *s, char const *name, size_t len)
{
  memcpy(s, name, len) ; s += len ;
  memcpy(s, ": pid ", 6) ; s += 6 ;
  s += pid_fmt(s, getpid()) ;
  *s++ = 0 ;
}
