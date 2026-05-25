/* ISC license. */

#include <skalibs/tai.h>

int timestamp (char *s)
{
  tain now ;
  return timestamp_r(s, &now) ;
}
