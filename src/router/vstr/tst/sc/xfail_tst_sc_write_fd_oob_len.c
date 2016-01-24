#include "xfail-tst-main.c"

static const char *rf = __FILE__;

void xfail_tst(void)
{
#ifndef HAVE_POSIX_HOST
  exit (EXIT_FAILED_OK);
#else
  unsigned int ern = 0;

  vstr_add_cstr_buf(s1, 0, "a");
  ASSERT(!vstr_sc_write_fd(s1, 1, 2, -1, &ern));
  ASSERT((ern == VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO) &&
         (errno == EINVAL));
#endif
}
