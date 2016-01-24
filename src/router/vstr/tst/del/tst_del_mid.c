#include "tst-main.c"

static const char *rf = __FILE__;

#define ADD(x) do { Vstr_ref *ref = NULL; \
  VSTR_ADD_CSTR_BUF(x, 0, buf); \
  VSTR_ADD_CSTR_PTR(x, 0, buf); \
  VSTR_ADD_CSTR_BUF(x, 0, buf); \
  VSTR_ADD_CSTR_PTR(x, 0, buf); \
  vstr_add_non(x, 0, 4); \
  ref = vstr_ref_make_malloc(4); \
  vstr_add_ref(x, 0, ref, 0, 4); \
  vstr_ref_del(ref); \
 } while (FALSE)

static void tst_del(Vstr_base *t1, size_t len)
{
  assert(len);

  while (--len)
    vstr_del(t1, 2, 1);
  vstr_del(t1, 1, 1);
}

int tst(void)
{
  size_t len = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  len = strlen(buf);

  ADD(s1);
  ADD(s2);
  ADD(s3);
  ADD(s4);

  tst_del(s1, (len * 4) + 8);
  tst_del(s2, (len * 4) + 8);
  tst_del(s3, (len * 4) + 8);
  tst_del(s4, (len * 4) + 8);

  return (s1->len || s2->len || s3->len);
}
