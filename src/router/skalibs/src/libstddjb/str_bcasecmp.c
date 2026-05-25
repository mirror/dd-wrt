/* ISC license. */

#include <strings.h>

#include <skalibs/bytestr.h>

int str_bcasecmp (void const *a, void const *b)
{
  return strcasecmp((char const *)a, *(char const *const *)b) ;
}
