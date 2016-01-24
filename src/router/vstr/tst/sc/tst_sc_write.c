#include "tst-main.c"

static const char *rf = __FILE__;

static const char *fn = "make_check_tst_sc_tst_write.tmp";

#ifndef HAVE_POSIX_HOST
extern        int unlink(const char *);

# define O_RDONLY (-1)
# define O_WRONLY (-1)
# define O_EXCL (-1)
# define O_CREAT (-1)
# define O_APPEND (-1)
#endif

static void read_file_s2(size_t tlen)
{
  vstr_del(s2, 1, s2->len);
  while (s2->len < tlen)
  {
    size_t left = tlen - s2->len;
    if (!vstr_sc_read_len_file(s2, s2->len, fn, s2->len, left, NULL))
      break;
  }
}

int tst(void)
{
  int ret = 0;
  unsigned int err = 0;

#ifndef HAVE_POSIX_HOST
  ASSERT(!vstr_sc_write_fd(s1, 1, 1, -1, &err));
  ASSERT((err == VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO) &&
         (errno == ENOSYS));
  
  ASSERT(!vstr_sc_write_file(s1, 1, 1, fn,
                             O_WRONLY | O_CREAT, 0600, 0, &err));
  ASSERT((err == VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO) &&
         (errno == ENOSYS));
  
  return (EXIT_FAILED_OK);
#endif

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  VSTR_ADD_CSTR_BUF(s2, 0, buf);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);
  VSTR_ADD_CSTR_BUF(s4, 0, buf);

  vstr_add_vstr(s3,       0, s3, 1, s1->len, 0);
  vstr_add_vstr(s3, s3->len, s3, 1, s1->len, 0);
  
  unlink(fn);
  TST_B_TST(ret, 1,
            !vstr_sc_write_file(s1, 1, s1->len, fn,
                                O_WRONLY | O_CREAT, 0600, 0, NULL));

  read_file_s2(s2->len);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  TST_B_TST(ret, 2, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  unlink(fn);
  if (MFAIL_NUM_OK)
  {
    tst_mfail_num(1);
    TST_B_TST(ret, 3,
              !!vstr_sc_write_file(s3, 1, s3->len,     fn, 0, 0600, 0, NULL));
    unlink(fn);
    tst_mfail_num(0);
  }
  TST_B_TST(ret, 4,
            !vstr_sc_write_file(s3, 2, s3->len - 1, fn, 0, 0600, 0, NULL));

  read_file_s2(s2->len);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);
  vstr_del(s3, 1, 1);
  TST_B_TST(ret, 5, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));
  
  unlink(fn);
  TST_B_TST(ret, 6,
            !vstr_sc_write_file(s4, 1, s4->len, fn,
                                O_WRONLY | O_CREAT, 0600, 0, NULL));

  read_file_s2(s2->len);
  VSTR_ADD_CSTR_BUF(s4, 0, buf);
  TST_B_TST(ret, 7, !VSTR_CMP_EQ(s4, 1, s4->len, s2, 1, s2->len));

  unlink(fn);
  TST_B_TST(ret, 8,
            !vstr_sc_write_file(s1, 1, s1->len, fn,
                                O_WRONLY | O_CREAT |O_APPEND, 0600, 0, NULL));

  read_file_s2(s2->len);
  VSTR_ADD_CSTR_BUF(s1, 0, buf);
  TST_B_TST(ret, 9, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  unlink(fn);
  TST_B_TST(ret, 10,
            !vstr_sc_write_file(s3, 1, s3->len, fn,
                                O_WRONLY | O_CREAT |O_APPEND, 0600, 0, NULL));

  read_file_s2(s2->len);
  VSTR_ADD_CSTR_BUF(s3, 0, buf);
  vstr_mov(s3, s3->len, s3, 1, 1);
  TST_B_TST(ret, 11, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  unlink(fn);
  TST_B_TST(ret, 12,
            !vstr_sc_write_file(s4, 1, s4->len, fn,
                                O_WRONLY | O_CREAT |O_APPEND, 0600, 0, NULL));
  TST_B_TST(ret, 13, (err != VSTR_TYPE_SC_WRITE_FILE_ERR_NONE));
  TST_B_TST(ret, 14, (err != VSTR_TYPE_SC_WRITE_FD_ERR_NONE));

  read_file_s2(s2->len);
  VSTR_ADD_CSTR_BUF(s4, 0, buf);
  TST_B_TST(ret, 15, !VSTR_CMP_EQ(s4, 1, s4->len, s2, 1, s2->len));

  /* end -- get rid of file */
  unlink(fn);

  VSTR_ADD_CSTR_BUF(s1, 0, buf);

  /* O_RD0NLY == 0, which means default flags ... as RDONLY isn't valid
   * (which is what we are testing for ... so hack it by including O_EXCL as
   * well, then it's non zero and hence won't use default */

  TST_B_TST(ret, 16, !!vstr_sc_write_file(s1, 1, s1->len, "/blah/.nothere",
                                         O_RDONLY | O_EXCL, 0666, 0, &err));

  TST_B_TST(ret, 17, (err != VSTR_TYPE_SC_WRITE_FILE_ERR_OPEN_ERRNO));

#ifdef __linux__
  /* FreeBSD stdin == stdout, so you can write to it */
  TST_B_TST(ret, 18, !!vstr_sc_write_file(s1, 1, s1->len, "/dev/stdin",
                                         O_RDONLY | O_EXCL, 0666, 1, &err));

  TST_B_TST(ret, 19, (err != VSTR_TYPE_SC_WRITE_FILE_ERR_SEEK_ERRNO));

  /* And god knows why these don't fail on FreeBSD.... */
  TST_B_TST(ret, 20, !!vstr_sc_write_file(s1, 1, s1->len, rf,
                                          O_RDONLY | O_EXCL, 0666, 1, &err));

  TST_B_TST(ret, 21, (err != VSTR_TYPE_SC_WRITE_FILE_ERR_WRITE_ERRNO));


  TST_B_TST(ret, 22, !!vstr_sc_write_fd(s1, 1, s1->len, 444, &err));

  TST_B_TST(ret, 23, (err != VSTR_TYPE_SC_WRITE_FD_ERR_WRITE_ERRNO));
#endif

  TST_B_TST(ret, 24, !vstr_sc_write_fd(s1, 1, 0, 444, NULL));
  TST_B_TST(ret, 25, !vstr_sc_write_file(s1, 1, 0, "/dev/null", 0, 0, 0, NULL));

  vstr_del(s1, 1, s1->len);

  if (SSIZE_MAX < (1000u * 1000u * 1000u * 3u))
  { /* don't do this for 64bit */
    Vstr_ref *ref = vstr_ref_make_malloc(VSTR_MAX_NODE_ALL);

    if (!ref)
      return (EXIT_FAILED_OK);

    while (s1->len <= SSIZE_MAX) /* assumes it will work */
      vstr_add_ref(s1, s1->len, ref, 0, VSTR_MAX_NODE_ALL);

    vstr_ref_del(ref);

    /* if this is passed all at once, we have EINVAL */
    ASSERT(vstr_sc_write_file(s1, 1, s1->len, "/dev/null",
                              O_WRONLY, 0, 0, &err));
    ASSERT(!s1->len);
  }
  
  return (TST_B_RET(ret));
}
/* Crap for tst_coverage constants... None trivial to test.
 *
 * VSTR_TYPE_SC_WRITE_FD_ERR_MEM
 * VSTR_TYPE_SC_WRITE_FILE_ERR_MEM
 *
 */
