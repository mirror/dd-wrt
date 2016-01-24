#define VSTR_COMPILE_INLINE 0

#include "tst-main.c"

char *xfail_dummy_ptr = NULL;
void *xfail_NULL_ptr = NULL;

#define TST__NULL_ptr xfail_NULL_ptr

static void xfail_tst(void); /* fwd */

int tst(void)
{
#ifdef HAVE_POSIX_HOST
  int fd = -1;

  xfail_NULL_ptr = xfail_dummy_ptr;
  
  if ((fd = open("/dev/null", O_WRONLY)) == -1) return (EXIT_SUCCESS);
  if (dup2(fd, 2) == -1)                        return (EXIT_SUCCESS);
#else
  xfail_NULL_ptr = xfail_dummy_ptr;
  free(malloc(1));
#endif

  xfail_NULL_ptr = NULL;
  xfail_tst();

  xfail_NULL_ptr = xfail_dummy_ptr;
  
  return (EXIT_SUCCESS);
}
