/* ISC license. */

#include <string.h>
#include <errno.h>

#include <skalibs/random.h>
#include <skalibs/posixplz.h>

int mkfiletemp (char *s, create_func_ref f, mode_t mode, void *data)
{
  size_t len = strlen(s) ;
  size_t xlen = 0 ;
  int r ;
  for (; xlen < len ; xlen++) if (s[len - 1 - xlen] != 'X') break ;
  if (xlen < 6) return (errno = EINVAL, -1) ;
  do
  {
    random_name_early(s + len - xlen, xlen) ;
    r = (*f)(s, mode, data) ;
  } while (r == -1 && errno == EEXIST) ;
  return r ;
}
