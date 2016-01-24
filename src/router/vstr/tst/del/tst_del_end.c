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

  assert(vstr_sc_posdiff(1, t1->len) == len);
  assert(vstr_sc_posdiff(1, t1->len) == VSTR_SC_POSDIFF(1, t1->len));

  assert(vstr_sc_posdiff(2, t1->len) == (len - 1));
  assert(vstr_sc_posdiff(2, t1->len) == VSTR_SC_POSDIFF(2, t1->len));

  ASSERT( vstr_del(t1, t1->len + 1, 0));

  while (--len)
    vstr_sc_reduce(t1, 1, t1->len, 1);

  assert(vstr_sc_posdiff(1, t1->len) == 1);
  assert(vstr_sc_posdiff(1, t1->len) == VSTR_SC_POSDIFF(1, t1->len));

  assert(vstr_sc_reduce(t1, 1, 1, 0) == 1);
  assert(vstr_sc_reduce(t1, 1, 0, 0) == 1);
  assert(vstr_sc_posdiff(1, t1->len) == 1);
  vstr_sc_reduce(t1, 1, 1, 1);
}

int tst(void)
{
  size_t len = 0;
  Vstr_ref *adder_ref = NULL;
  size_t off = 0;
  const char *tmp_s = NULL;
  int tmp_i = 0;
  
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

  if (s1->len || s2->len || s3->len || s4->len)
    return (s1->len + s2->len + s3->len + s4->len);

  ADD(s1);
  ADD(s2);
  ADD(s3);
  ADD(s4);


  tmp_s = vstr_export_cstr_ptr(s1, 4, s1->len - 3);
  tmp_i = vstr_export_iovec_ptr_all(s1, NULL, NULL);
  ASSERT(tmp_s && tmp_i);
  
  tmp_s = vstr_export_cstr_ptr(s2, 4, s2->len - 3);
  tmp_i = vstr_export_iovec_ptr_all(s2, NULL, NULL);
  ASSERT(tmp_s && tmp_i);

  tmp_s = vstr_export_cstr_ptr(s3, 4, s3->len - 3);
  tmp_i = vstr_export_iovec_ptr_all(s3, NULL, NULL);
  ASSERT(tmp_s && tmp_i);

  tmp_s = vstr_export_cstr_ptr(s4, 4, s4->len - 3);
  tmp_i = vstr_export_iovec_ptr_all(s4, NULL, NULL);
  ASSERT(!tmp_s && !tmp_i);


  tst_del(s1, (len * 4) + 8);
  tst_del(s2, (len * 4) + 8);
  tst_del(s3, (len * 4) + 8);
  tst_del(s4, (len * 4) + 8);

  if (s1->len || s2->len || s3->len || s4->len)
    return (s1->len + s2->len + s3->len + s4->len);

  ADD(s1);
  adder_ref = vstr_export_cstr_ref(s1, 4, s1->len - 3, &off);
  vstr_export_iovec_ptr_all(s1, NULL, NULL);
	 
  tst_del(s1, (len * 4) + 8);
	     
  if (s1->len || s2->len || s3->len || s4->len)
    return (s1->len + s2->len + s3->len + s4->len);
	     
  vstr_ref_del(adder_ref);

  return (EXIT_SUCCESS);
}
