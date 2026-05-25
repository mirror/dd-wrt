/* ISC license. */

#include <skalibs/djbunix.h>

ssize_t openreadnclose (char const *file, char *s, size_t n)
{
  int fd = openbc_read(file) ;
  return fd == -1 ? -1 : readnclose(fd, s, n) ;
}
