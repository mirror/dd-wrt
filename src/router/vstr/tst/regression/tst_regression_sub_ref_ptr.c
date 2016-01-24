#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_ref *ref = vstr_ref_make_malloc(1);

  ASSERT(ref);
  ASSERT(ref->ref == 1);
  ASSERT(!s1->len);
  
  vstr_add_ref(s1, 0, ref, 0, 1);
  ASSERT(ref->ref == 2);
  ASSERT(s1->len == 1);
  
  vstr_sub_ptr(s1, 1, s1->len, "", 1);
  ASSERT(ref->ref == 1);
  ASSERT(s1->len == 1);
  
  vstr_ref_del(ref);

  return (EXIT_SUCCESS);
}


