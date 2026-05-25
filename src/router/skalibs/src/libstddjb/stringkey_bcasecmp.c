/* ISC license. */

#include <strings.h>

#include <skalibs/bytestr.h>
#include "bytestr-internal.h"

int stringkey_bcasecmp (void const *a, void const *b)
{
  struct stringkey_s const *bb = b ;
  return strcasecmp((char const *)a, bb->s) ;
}
