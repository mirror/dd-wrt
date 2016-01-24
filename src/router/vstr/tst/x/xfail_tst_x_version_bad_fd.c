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
#if defined(VSTR_AUTOCONF_HAVE_POSIX_HOST) && !defined(VSTR_AUTOCONF_NDEBUG)
  close(1);
  
  vstr_version_func();
  
  return (EXIT_FAILURE);
#endif
  return (77); /* Failed .. ok */
}
