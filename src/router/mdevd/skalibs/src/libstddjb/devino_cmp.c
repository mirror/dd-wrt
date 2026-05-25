/* ISC license. */

#include <skalibs/devino.h>

int devino_cmp (void const *a, void const *b)
{
  devino const *aa = a ;
  devino const *bb = b ;
  return aa->dev < bb->dev ? -1 :
    aa->dev > bb->dev ? 1 :
    aa->ino < bb->ino ? -1 :
    aa->ino > bb->ino ;
}
