/* ISC license */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main (int argc, char const *const *argv)
{
  char buf[8192] ;
  if (argc < 2) return 111 ;
  ssize_t r = readlink(argv[1], buf, 8192) ;
  return r == -1 ? errno == EIO || errno == ELOOP ? 111 : 1 : r <= 1 ;
}
