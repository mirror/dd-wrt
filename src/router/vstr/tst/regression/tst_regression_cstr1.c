#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  vstr_add_cstr_buf(s1, s1->len, "12345678");
  
  ASSERT(vstr_export_cstr_ptr(s1, 1, 4));
         
  vstr_del(s1, 1, 4);

  /* calls vstr_export_ref() -- which uses cstr cache */
  vstr_add_vstr(s2, 0, s1, 1, s1->len, VSTR_TYPE_ADD_ALL_REF);
  
  return (!vstr_cmp_eq(s1, 1, s1->len, s2, 1, s2->len));
}
