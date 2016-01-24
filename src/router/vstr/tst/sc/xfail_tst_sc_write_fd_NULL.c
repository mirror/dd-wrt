#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
#ifndef HAVE_POSIX_HOST
  exit (EXIT_FAILED_OK);
#else
  unsigned int ern = 0;
  
  ASSERT(!vstr_sc_write_fd(TST__NULL_ptr, 1, 1, -1, &ern));
  ASSERT((ern == VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO) &&
         (errno == EINVAL));
#endif
}
