#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  unsigned int ern = 0;

#ifndef HAVE_POSIX_HOST
  return (EXIT_FAILED_OK);
#else
  if (!FD_CLOSEFAIL_NUM_OK)
    return (EXIT_FAILED_OK);
  
  tst_fd_closefail_num(1);
  ASSERT(!vstr_sc_mmap_file(s1, 0, __FILE__, 0, 1, &ern));
  ASSERT(ern == VSTR_TYPE_SC_MMAP_FILE_ERR_CLOSE_ERRNO);
  ASSERT(errno == EIO);
  tst_fd_closefail_num(0);
  
  tst_fd_closefail_num(1);
  ASSERT(!vstr_sc_read_iov_file(s1, 0, __FILE__, 0, 1, 1, &ern));
  ASSERT(ern == VSTR_TYPE_SC_READ_FILE_ERR_CLOSE_ERRNO);
  ASSERT(errno == EIO);
  tst_fd_closefail_num(0);
  
  tst_fd_closefail_num(1);
  ASSERT(!vstr_sc_read_len_file(s1, 0, __FILE__, 0, 1, &ern));
  ASSERT(ern == VSTR_TYPE_SC_READ_FILE_ERR_CLOSE_ERRNO);
  ASSERT(errno == EIO);
  tst_fd_closefail_num(0);
  
  tst_fd_closefail_num(1);
  ASSERT(!vstr_sc_write_file(s1, 1, s1->len, "/dev/null",
                             O_WRONLY, 0666, 0, &ern));
  ASSERT(ern == VSTR_TYPE_SC_WRITE_FILE_ERR_CLOSE_ERRNO);
  ASSERT(errno == EIO);
  tst_fd_closefail_num(0);
  
  return (EXIT_SUCCESS);
#endif
}
