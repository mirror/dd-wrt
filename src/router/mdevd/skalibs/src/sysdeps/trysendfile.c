/* ISC license. */

#include <sys/sendfile.h>

int main (void)
{
  sendfile(1, 0, 0, 4096) ;
  return 0 ;
}
