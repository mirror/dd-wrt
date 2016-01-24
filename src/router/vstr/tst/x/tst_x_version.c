#define VSTR_COMPILE_INCLUDE 1

#include <vstr.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern void vstr_version_func(void);

int main(void)
{
  int fd = -1;

#ifndef VSTR_AUTOCONF_HAVE_POSIX_HOST
  return (77); /* Failed .. ok */
#else
  if ((fd = open("/dev/null", O_WRONLY)) == -1)
    return (EXIT_FAILURE);

  if (dup2(fd, 1) == -1)
    return (EXIT_FAILURE);

  vstr_version_func();
#endif

  return (EXIT_FAILURE);
}
