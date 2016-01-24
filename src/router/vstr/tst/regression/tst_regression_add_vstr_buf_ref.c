#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  vstr_add_rep_chr(s1, s1->len, 'a', s1->conf->buf_sz + 1);
  vstr_del(s1, 1, s1->conf->buf_sz - 1);

  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len, "aa"));

  vstr_add_vstr(s2, 0, s1, 2, 1, VSTR_TYPE_ADD_BUF_REF);

  ASSERT(vstr_cmp_cstr_eq(s2, 1, s2->len, "a"));
  
  return (EXIT_SUCCESS);
}
