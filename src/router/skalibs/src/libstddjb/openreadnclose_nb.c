/* ISC license. */

#include <skalibs/djbunix.h>

ssize_t openreadnclose_nb (char const *file, char *s, size_t n)
{
  int fd = openc_read(file) ;
  return fd == -1 ? fd : readnclose(fd, s, n) ;
}
