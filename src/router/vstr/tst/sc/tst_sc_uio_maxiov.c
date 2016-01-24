#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int fd = -1;
  unsigned int scan = 0;
  
#ifndef HAVE_POSIX_HOST
  return (EXIT_FAILED_OK);
#else

  fd = open("/dev/null", O_WRONLY);
  if (fd == -1)
    return (EXIT_FAILED_OK);

  scan = 0;
  while (scan++ <= UIO_MAXIOV)
    vstr_add_cstr_ptr(s1, s1->len, "abcd");

  ASSERT(vstr_num(s1, 1, s1->len) > UIO_MAXIOV);
  
  TST_B_TST(ret, 1,
            !vstr_sc_write_fd(s1, 1, s1->len, fd, NULL));
  TST_B_TST(ret, 2, s1->len);

  scan = 0;
  while (scan++ <= UIO_MAXIOV)
    vstr_add_cstr_ptr(s1, s1->len, "abcd");
  TST_B_TST(ret, 3,
            !vstr_sc_write_file(s1, 1, s1->len, "/dev/null",
                                O_WRONLY, 0000, 0, NULL));
  TST_B_TST(ret, 4, s1->len);
#endif
  
  return (TST_B_RET(ret));
}
