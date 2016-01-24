/* TEST: abcd */
#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t tlen = strlen("/* TEST: abcd */\n");
  size_t fsize = 0;
  size_t count = 0;
  unsigned int ern = 0;
  
#if !defined(VSTR_AUTOCONF_HAVE_MMAP) /* || !defined(HAVE_POSIX_HOST) */
  return (EXIT_FAILED_OK);
#endif

  TST_B_TST(ret, 1,
            !vstr_sc_mmap_file(s1, s1->len, __FILE__, 0, tlen, NULL));
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "/* TEST: abcd */\n"));
  
  vstr_del(s1, 1, s1->len);
  TST_B_TST(ret, 3,
            !vstr_sc_mmap_file(s1, s1->len, __FILE__, 0,    0, NULL));
  fsize = s1->len;
  
  vstr_del(s1, 1, s1->len);

  vstr_add_non(s1, 0, SSIZE_MAX);

  ASSERT(vstr_cmp_buf_eq(s1, 1, s1->len, NULL, SSIZE_MAX));

  vstr_add_non(s1, 0, ((SIZE_MAX - SSIZE_MAX) - count));
  
  while (count < fsize)
  {
    ASSERT(vstr_cmp_buf_eq(s1, 1, s1->len, NULL, (SIZE_MAX - count)));
    
    ASSERT(!vstr_sc_mmap_file(s1,   s1->len, __FILE__, 0,    0, &ern));
    ASSERT(ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE);
    ASSERT(!vstr_sc_mmap_file(s1,         0, __FILE__, 0,    0, &ern));
    ASSERT(ern == VSTR_TYPE_SC_MMAP_FD_ERR_TOO_LARGE);
    ASSERT(!vstr_sc_mmap_file(s1, SSIZE_MAX, __FILE__, 0,    0, &ern));
    ASSERT(ern == VSTR_TYPE_SC_MMAP_FILE_ERR_TOO_LARGE);
    
    ASSERT(!vstr_sc_read_len_file(s1,   s1->len, __FILE__, 0,    0, &ern));
    ASSERT(ern == VSTR_TYPE_SC_READ_FILE_ERR_TOO_LARGE);
    ASSERT(!vstr_sc_read_len_file(s1,         0, __FILE__, 0,    0, &ern));
    ASSERT(ern == VSTR_TYPE_SC_READ_FD_ERR_TOO_LARGE);
    ASSERT(!vstr_sc_read_len_file(s1, SSIZE_MAX, __FILE__, 0,    0, &ern));
    ASSERT(ern == VSTR_TYPE_SC_READ_FILE_ERR_TOO_LARGE);
    
    vstr_sc_reduce(s1, 1, s1->len, 1);
    ++count;
  }

  ASSERT(vstr_sc_mmap_file(s1, s1->len, __FILE__, 0, 0, &ern));
  ASSERT(s1->len == SIZE_MAX);
  ASSERT(vstr_cmp_buf_eq(s1, 1, (SIZE_MAX - count), NULL, (SIZE_MAX - count)));
  ASSERT(vstr_cmp_cstr_eq(s1, (SIZE_MAX - count) + 1, tlen,
                          "/* TEST: abcd */\n"));

  /* What the while loop above does */
  vstr_sc_reduce(s1, 1, s1->len, fsize);
  
  ASSERT(!(fsize - (SIZE_MAX - s1->len)));
  while (s1->len != SIZE_MAX)
    ASSERT(vstr_sc_read_len_file(s1, s1->len, __FILE__,
                                 fsize - (SIZE_MAX - s1->len), 0, &ern));
  ASSERT(vstr_cmp_buf_eq(s1, 1, (SIZE_MAX - count), NULL, (SIZE_MAX - count)));
  ASSERT(vstr_cmp_cstr_eq(s1, (SIZE_MAX - count) + 1, tlen,
                          "/* TEST: abcd */\n"));
  
  return (TST_B_RET(ret));
}
