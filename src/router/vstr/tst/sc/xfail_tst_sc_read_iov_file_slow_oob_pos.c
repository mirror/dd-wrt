#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
  unsigned int ern = 0;

#ifndef HAVE_POSIX_HOST
  exit (EXIT_FAILED_OK);
#endif

  ASSERT(!vstr_sc_read_iov_file(s4, 1, __FILE__, 0, 1, 1, &ern));
  ASSERT((ern == VSTR_TYPE_SC_READ_FD_ERR_READ_ERRNO) &&
         (errno == EINVAL));
  ASSERT(!vstr_sc_read_iov_file(s4, 1, __FILE__, 0, 1, 1, NULL));
}
