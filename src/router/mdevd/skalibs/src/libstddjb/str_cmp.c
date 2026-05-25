/* ISC license. */

#include <string.h>

#include <skalibs/bytestr.h>

int str_cmp (void const *a, void const *b)
{
  return strcmp(*(char const *const *)a, *(char const *const *)b) ;
}
