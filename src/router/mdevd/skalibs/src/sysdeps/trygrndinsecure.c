/* ISC license. */

#include <features.h>

#ifdef __stub_getrandom
#error getrandom() appears to be a stub function, we won't use it.
#endif

#include <sys/random.h>

int main (void)
{
  char buf[4] ;
  if (getrandom(buf, 4, GRND_INSECURE) < 0) return 1 ;
  return 0 ;
}
