#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_ref *ref = vstr_ref_make_malloc(1);

  
  
  ASSERT(ref);
  ASSERT(ref->ref == 1);
  ASSERT(!s1->len);

  vstr_add_ptr(s1, 0, "", 1);
  ASSERT(ref->ref == 1);
  ASSERT(s1->len == 1);  
  
  vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_REF, 1000);
  
  if (tst_mfail_num(1))
  {
    ASSERT(!vstr_sub_ref(s1, 1, s1->len, ref, 0, 1));
    ASSERT(ref->ref == 1);
    ASSERT(s1->len == 1);
  }
  
  ASSERT(vstr_sub_ref(s1, 1, s1->len, ref, 0, 1));
  ASSERT(ref->ref == 2);
  ASSERT(s1->len == 1);
  
  vstr_ref_del(ref);

  return (EXIT_SUCCESS);
}


