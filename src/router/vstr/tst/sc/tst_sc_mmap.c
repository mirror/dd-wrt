/* TEST: abcd */
#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t tlen = strlen("/* TEST: abcd */\n");
  unsigned int err = 0;
  int mfail_count = 0;

#if !defined(VSTR_AUTOCONF_HAVE_MMAP) /* || !defined(HAVE_POSIX_HOST) */
  ASSERT(!vstr_sc_mmap_fd(s1, 0, -1, 0, 0, &err));
  ASSERT((err == VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO) &&
         (errno == ENOSYS));
  
  ASSERT(!vstr_sc_mmap_file(s1, 0, __FILE__, 0, 0, &err));
  ASSERT((err == VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO) &&
         (errno == ENOSYS));
  
  return (EXIT_FAILED_OK);
#endif

  TST_B_TST(ret, 1,
            !vstr_sc_mmap_file(s1, s1->len, __FILE__, 0, tlen, NULL));

  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "/* TEST: abcd */\n"));

  TST_B_TST(ret, 3,
            !vstr_sc_mmap_file(s1, s1->len, __FILE__, 0, 0, &err));
  TST_B_TST(ret, 3, (err != VSTR_TYPE_SC_MMAP_FD_ERR_NONE));
  TST_B_TST(ret, 3, (err != VSTR_TYPE_SC_MMAP_FILE_ERR_NONE));
  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, strlen("/* TEST: abcd */\n"),
                                      "/* TEST: abcd */\n"));

  TST_B_TST(ret, 5, !!vstr_sc_mmap_fd(s1, s1->len, -1, 0, 0, &err));
  TST_B_TST(ret, 6, (err != VSTR_TYPE_SC_MMAP_FD_ERR_FSTAT_ERRNO));
  TST_B_TST(ret, 6, (err != VSTR_TYPE_SC_MMAP_FILE_ERR_FSTAT_ERRNO));

  TST_B_TST(ret, 5, !!vstr_sc_mmap_fd(s1, s1->len, -1, 0, 0, NULL));

  TST_B_TST(ret, 7, !!vstr_sc_mmap_fd(s1, s1->len, -1, 0, 1, &err));
  TST_B_TST(ret, 8, (err != VSTR_TYPE_SC_MMAP_FD_ERR_MMAP_ERRNO));
  TST_B_TST(ret, 8, (err != VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO));

  TST_B_TST(ret, 7, !!vstr_sc_mmap_fd(s1, s1->len, -1, 0, 1, NULL));

  TST_B_TST(ret, 9, !!vstr_sc_mmap_file(s1, s1->len, "/abcd_missing",
                                        0, 1, &err));
  TST_B_TST(ret, 10, (err != VSTR_TYPE_SC_MMAP_FILE_ERR_OPEN_ERRNO));

#ifdef __linux__
  TST_B_TST(ret, 11, !!vstr_sc_mmap_file(s1, s1->len, "/proc/self/maps",
                                        0, 1, &err));
  TST_B_TST(ret, 12, (err != VSTR_TYPE_SC_MMAP_FILE_ERR_MMAP_ERRNO));
#endif

  vstr_del(s1, 1, s1->len);

  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_sc_mmap_file(s1, s1->len, __FILE__, 0, tlen, &err) &&
           (err == VSTR_TYPE_SC_MMAP_FILE_ERR_MEM));

  assert(VSTR_TYPE_SC_MMAP_FILE_ERR_MEM == VSTR_TYPE_SC_MMAP_FD_ERR_MEM);

  return (TST_B_RET(ret));
}
