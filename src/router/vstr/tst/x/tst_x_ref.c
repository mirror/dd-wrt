#include "tst-main.c"

static const char *rf = __FILE__;

static void tst_cb_free_ref_ref(Vstr_ref *ref)
{
  Vstr_ref *ptr = ref->ptr;
  
  vstr_ref_del(ptr);
  vstr_ref_cb_free_ref(ref);
}

int tst(void)
{
  Vstr_ref *ref = NULL;

  vstr_ref_del(NULL);

  ref = vstr_ref_make_malloc(4);
  vstr_ref_add(ref);
  vstr_ref_del(ref);
  vstr_ref_del(ref);

  ref = vstr_ref_make_ptr("", vstr_ref_cb_free_ref);
  vstr_ref_add(ref);
  vstr_ref_del(ref);
  vstr_ref_del(ref);

  ref = vstr_ref_make_ptr(malloc(1), vstr_ref_cb_free_ptr_ref);
  vstr_ref_add(ref);
  vstr_ref_del(ref);
  ref = vstr_ref_make_ptr(ref, tst_cb_free_ref_ref);
  vstr_ref_add(ref);
  vstr_ref_del(ref);
  vstr_ref_del(ref);

  ref = vstr_ref_make_ptr(malloc(1), vstr_ref_cb_free_ptr);
  vstr_ref_add(ref);
  vstr_ref_del(ref);
  vstr_ref_del(ref);
  free(ref); /* cb only free's the ref->ptr */

  ref = vstr_ref_make_strdup("abcd");
  vstr_ref_add(ref);
  if (strcmp("abcd", ref->ptr))
    ASSERT(FALSE);
  if ("abcd" == ref->ptr)
    ASSERT(FALSE);
  vstr_ref_del(ref);
  vstr_ref_del(ref);

  if (MFAIL_NUM_OK)
  {
    tst_mfail_num(1); /* test no check error */
    ref = vstr_ref_make_strdup("abcd");
    ASSERT(!ref);
    tst_mfail_num(0);
  }
  
  ref = VSTR_REF_MAKE_STRDUP("abcd");
  vstr_ref_add(ref);
  if (strcmp("abcd", ref->ptr))
    ASSERT(FALSE);
  if ("abcd" == ref->ptr)
    ASSERT(FALSE);
  vstr_ref_del(ref);
  vstr_ref_del(ref);

  {
    Vstr_base *b = vstr_make_base(NULL);
    Vstr_conf *c = vstr_make_conf();
    Vstr_sects *s = vstr_sects_make(4);

    ref = vstr_ref_make_vstr_base(b);
    vstr_ref_add(ref);
    vstr_ref_del(ref);
    vstr_ref_del(ref);
    ref = vstr_ref_make_vstr_conf(c);
    vstr_ref_add(ref);
    vstr_ref_del(ref);
    vstr_ref_del(ref);
    ref = vstr_ref_make_vstr_sects(s);
    vstr_ref_add(ref);
    vstr_ref_del(ref);
    vstr_ref_del(ref);
  }
  
  {
    Vstr_ref *refs[128];
    unsigned int scan = 0;
    size_t off;

    vstr_add_cstr_ptr(s1, 0, "abcd");

    scan = 0;
    while (scan < (sizeof(refs) / sizeof(refs[0])))
      refs[scan++] = vstr_export_ref(s1, 1, 1, &off);

    scan = 0;
    while (scan < (sizeof(refs) / sizeof(refs[0])))
      vstr_ref_del(refs[scan++]);
    
    scan = 0;
    while (scan < (sizeof(refs) / sizeof(refs[0])))
      refs[scan++] = vstr_export_ref(s1, 1, 1, &off);

    scan = (sizeof(refs) / sizeof(refs[0]));
    while (scan-- > 0)
      vstr_ref_del(refs[scan]);
  }
  
  return (0);
}
