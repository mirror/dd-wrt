/* ISC license. */

#include <strings.h>

#include <skalibs/bytestr.h>

int str_casecmp (void const *a, void const *b)
{
  return strcasecmp(*(char const *const *)a, *(char const *const *)b) ;
}
