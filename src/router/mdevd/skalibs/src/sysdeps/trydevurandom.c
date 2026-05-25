/* ISC license. */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main (void)
{
  char a[64] ;
  char b[64] ;
  int fd ;
  fd = open("/dev/urandom", O_RDONLY) ;
  if ((fd == -1) || (read(fd, a, 64) < 64) ) return 111 ;
  close(fd) ;
  fd = open("/dev/urandom", O_RDONLY) ;
  if ((fd == -1) || (read(fd, b, 64) < 64) ) return 111 ;
  close(fd) ;
  return (!memcmp(a, b, 64)) ;  
}
