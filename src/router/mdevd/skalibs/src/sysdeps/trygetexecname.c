/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#include <stdlib.h>

int main (void)
{
  return !!getexecname() ;
}
