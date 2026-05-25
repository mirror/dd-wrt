/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/ioctl.h>
#include <linux/nsfs.h>

int main (void)
{
  return ioctl(0, NS_GET_PARENT) ;
}
