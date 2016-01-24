#include "tst-main.c"

static const char *rf = __FILE__;

#define TST_FLAG(num, name, old, new) \
  tmp_f = 0xF0F0F0F0; \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_ ## name, &tmp_f))); \
  TST_B_TST(ret, (num), \
            !(tmp_f == (old))); \
  TST_B_TST(ret, (num), \
            !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_ ## name, (new))); \
  tmp_f = 0xF0F0F0F0; \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_ ## name, &tmp_f))); \
  TST_B_TST(ret, (num), \
            !(tmp_f == (new)))

#define TST_CHR(num, name, old, new) \
  tmp_c = 0xF; \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_ ## name, &tmp_c) && \
              tmp_c == (old))); \
  TST_B_TST(ret, (num), \
            !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_ ## name, (new))); \
  tmp_c = 0xF; \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_ ## name, &tmp_c) && \
              tmp_c == (new)))

#define TST_CSTR(num, name, old, new) \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, &tmp_pc) && \
              !strcmp(tmp_pc, (old)))); \
  TST_B_TST(ret, (num), \
            !vstr_cntl_conf(NULL, \
                            VSTR_CNTL_CONF_SET_ ## name, (new))); \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, &tmp_pc) && \
              !strcmp(tmp_pc, (new))))

#define TST_CSTR2(num, name, old, new, tst_new) \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, &tmp_pc) && \
              !strcmp(tmp_pc, (old)))); \
  TST_B_TST(ret, (num), \
            !vstr_cntl_conf(NULL, \
                            VSTR_CNTL_CONF_SET_ ## name, (new))); \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, &tmp_pc) && \
              !strcmp(tmp_pc, (tst_new))))

#define TST_REF(num, name, old, new) do { \
  Vstr_ref *tmp_ref = NULL; size_t len = 0; \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, \
                               &tmp_ref, &len) && \
              !strcmp(tmp_ref->ptr, (old)))); \
  vstr_ref_del(tmp_ref); \
  tmp_ref = vstr_ref_make_strdup(new); ASSERT(tmp_ref); \
  TST_B_TST(ret, (num), \
            !vstr_cntl_conf(NULL, \
                            VSTR_CNTL_CONF_SET_ ## name, \
                            tmp_ref, strlen(tmp_ref->ptr))); \
  vstr_ref_del(tmp_ref); \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, \
                               &tmp_ref, &len) && \
              !strcmp(tmp_ref->ptr, (new)))); \
  vstr_ref_del(tmp_ref); \
 } while (FALSE)

#define TST_NUM_REF(num, name, num_base, old, new) do { \
  Vstr_ref *tmp_ref = NULL; size_t len = 0; \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, (num_base), \
                               &tmp_ref, &len) && \
              !strcmp(tmp_ref->ptr, (old)))); \
  vstr_ref_del(tmp_ref); \
  tmp_ref = vstr_ref_make_strdup(new); ASSERT(tmp_ref); \
  TST_B_TST(ret, (num), \
            !vstr_cntl_conf(NULL, \
                            VSTR_CNTL_CONF_SET_ ## name, \
                            (num_base), tmp_ref, strlen(tmp_ref->ptr))); \
  vstr_ref_del(tmp_ref); \
  TST_B_TST(ret, (num), \
            !(!!vstr_cntl_conf(NULL, \
                               VSTR_CNTL_CONF_GET_ ## name, (num_base), \
                               &tmp_ref, &len) && \
              !strcmp(tmp_ref->ptr, (new)))); \
  vstr_ref_del(tmp_ref); \
 } while (FALSE)

int tst(void)
{
  int ret = 0;
  int tmp_f = FALSE;
  unsigned int tmp_t = 0;
  char tmp_c = 0;
  char *tmp_pc = NULL;
  Vstr_conf *tmp_cnf = NULL;
  int mfail_count = 0;
  
  TST_FLAG(  1, FLAG_ATOMIC_OPS, TRUE, FALSE);
  TST_FLAG(  2, FLAG_DEL_SPLIT, FALSE, TRUE);
  TST_FLAG(  3, FLAG_IOV_UPDATE, TRUE, FALSE);
  TST_CHR(   5, FMT_CHAR_ESC, 0, '$');

  TST_CSTR(  6, LOC_CSTR_DEC_POINT, ".", "->");
  TST_CSTR(  7, LOC_CSTR_NAME_NUMERIC, "C", "James");
  TST_CSTR2( 8, LOC_CSTR_THOU_GRP, "", "\1\2\3\255abcd", "\1\2\3\255");
  TST_CSTR(  9, LOC_CSTR_THOU_SEP, "", "<->");

  TST_B_TST(ret, 10, !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_REF, &tmp_f));
  TST_B_TST(ret, 11, (tmp_f != 2));
  TST_B_TST(ret, 12, !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_REF, &tmp_f));
  TST_B_TST(ret, 13, (tmp_f != 2));
  TST_B_TST(ret, 14, !vstr_cntl_base(s1, VSTR_CNTL_BASE_GET_CONF, &tmp_cnf));
  TST_B_TST(ret, 15, !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_REF, &tmp_f));
  TST_B_TST(ret, 16, (tmp_f != 3));
  TST_B_TST(ret, 17, !vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_REF, &tmp_f));
  TST_B_TST(ret, 18, (tmp_f != 3));
  vstr_free_conf(tmp_cnf);
  TST_B_TST(ret, 15, !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_REF, &tmp_f));
  TST_B_TST(ret, 16, (tmp_f != 2));
  TST_B_TST(ret, 20, !vstr_cntl_base(s1, VSTR_CNTL_BASE_SET_CONF, NULL));

  TST_NUM_REF(21, LOC_REF_DEC_POINT, 10, "->", "Foo1");
  TST_NUM_REF(21, LOC_REF_DEC_POINT, 11, "->", "Foo2");
  TST_NUM_REF(21, LOC_REF_DEC_POINT,  0, "->", "Foo3");
  TST_NUM_REF(21, LOC_REF_DEC_POINT,  8, "Foo3", "Foo4");
  TST_NUM_REF(21, LOC_REF_DEC_POINT, 11, "Foo2", "Foo8");
  TST_NUM_REF(21, LOC_REF_THOU_GRP, 11, "\1\2\3\255", "\2");
  TST_NUM_REF(21, LOC_REF_THOU_SEP, 0, "<->", "<-->");

  TST_REF(21, LOC_REF_NAME_NUMERIC, "James", "FooBar");
  TST_REF(21, LOC_REF_NULL_PTR, "(null)", "abcd");
  
#define MFAIL_TST_CSTR(name, val)  \
 vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_ ## name, val)
  
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!MFAIL_TST_CSTR(LOC_CSTR_DEC_POINT, "abcd") ||
           !MFAIL_TST_CSTR(LOC_CSTR_NAME_NUMERIC, "abcd") ||
           !MFAIL_TST_CSTR(LOC_CSTR_THOU_GRP, "abcd") ||
           !MFAIL_TST_CSTR(LOC_CSTR_THOU_SEP, "abcd"));

  tst_mfail_num(0);

  TST_CSTR(  26, LOC_CSTR_DEC_POINT, "abcd", "->");
  TST_CSTR(  27, LOC_CSTR_NAME_NUMERIC, "abcd", "James");
  TST_CSTR(  28, LOC_CSTR_THOU_GRP, "abcd", "\1\2\3\255");
  TST_CSTR(  29, LOC_CSTR_THOU_SEP, "abcd", "<->");

#ifndef USE_RESTRICTED_HEADERS
  mfail_count = 0;
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!MFAIL_TST_CSTR(LOC_CSTR_AUTO_NAME_NUMERIC, "en_US"));
#endif
  
  mfail_count = 0;
  do
  {
    vstr_free_spare_nodes(s1->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!MFAIL_TST_CSTR(NUM_SPARE_PTR, 4));

  tst_mfail_num(0);

  MFAIL_TST_CSTR(NUM_SPARE_PTR, 0);


  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FLAG_ALLOC_CACHE, FALSE);
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == FALSE);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE));
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FLAG_ALLOC_CACHE, FALSE);
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == FALSE);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE));
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FLAG_ALLOC_CACHE, TRUE);
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_POS));
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FLAG_ALLOC_CACHE, TRUE);
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_POS));
  
  /* none */
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == FALSE);
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == FALSE);
#define TST_RANGE() \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 0)); \
  vstr_free_base(s1); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 0)); \
  vstr_free_base(vstr_make_base(NULL)); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 1)); \
  vstr_free_base(vstr_make_base(NULL)); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 1)); \
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE, 0, 0); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 0)); \
  vstr_free_base(vstr_make_base(NULL)); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 1)); \
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE, 3, 6); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 3)); \
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE, 3, 6); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 3)); \
  vstr_free_base(vstr_make_base(NULL)); \
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_NUM_SPARE_BASE, &tmp_t) && \
         (tmp_t == 3)); \
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE, 1, 2); \
  vstr_free_base(vstr_make_base(NULL)); \
  s1 = vstr_dup_cstr_ptr(NULL, "XXXXXXXXXX"); \
  vstr_add_cstr_ptr(s1, s1->len, "XXXXXXXXXX"); \
  vstr_add_cstr_ptr(s1, s1->len, "XXXXXXXXXX"); \
  vstr_add_cstr_ptr(s1, s1->len, "XXXXXXXXXX"); \
  ASSERT(s1->len == 40); \
  if (tmp_f) \
  { \
    struct iovec *iov = NULL; \
    unsigned int  num = 0; \
    ASSERT(!strcmp(vstr_export_cstr_ptr(s1, 1, s1->len), \
                   "XXXXXXXXXX" "XXXXXXXXXX" "XXXXXXXXXX" "XXXXXXXXXX")); \
    vstr_export_iovec_ptr_all(s1, &iov, &num); \
    ASSERT(num == 4); \
    ASSERT(!strcmp(iov[0].iov_base, "XXXXXXXXXX")); \
    ASSERT(!strcmp(iov[1].iov_base, "XXXXXXXXXX")); \
    ASSERT(!strcmp(iov[2].iov_base, "XXXXXXXXXX")); \
    ASSERT(!strcmp(iov[3].iov_base, "XXXXXXXXXX")); \
  } \
  vstr_add_cstr_ptr(s1, s1->len, "a"); \
  ASSERT(s1->len == 41); \
  ASSERT(vstr_cmp_cstr_eq(s1, 21, 20, \
         "XXXXXXXXXX" "XXXXXXXXXX")); \
  ASSERT(vstr_cmp_cstr_eq(s1, 1, s1->len, \
         "XXXXXXXXXX" "XXXXXXXXXX" "XXXXXXXXXX" "XXXXXXXXXX" "a"))


  TST_RANGE();
  ASSERT(!!vstr_cntl_base(s1,
                          VSTR_CNTL_BASE_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE));
  
  /* pos */
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_POS);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_POS));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_POS);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_POS));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  TST_RANGE();
  ASSERT(!!vstr_cntl_base(s1,
                          VSTR_CNTL_BASE_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_POS));

  /* iovec */
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  TST_RANGE();
  ASSERT(!!vstr_cntl_base(s1,
                          VSTR_CNTL_BASE_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC));

  /* cstr */
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
  ASSERT(!!vstr_cntl_conf(NULL,
                          VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR));
  ASSERT(!!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE, &tmp_f) &&
         tmp_f == TRUE);
  TST_RANGE();
  ASSERT(!!vstr_cntl_base(s1,
                          VSTR_CNTL_BASE_GET_TYPE_GRPALLOC_CACHE, &tmp_t) &&
         (tmp_t == VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR));

  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE, 1, 1);
  if (MFAIL_NUM_OK)
  {
    tst_mfail_num(1);

    ASSERT(!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE, 2, 2));
  }
  
  return (TST_B_RET(ret));
}
/* Crap for tst_coverage constants....
 *
 * VSTR_CNTL_CONF_GET_FLAG_ATOMIC_OPS
 * VSTR_CNTL_CONF_SET_FLAG_ATOMIC_OPS
 *
 * VSTR_CNTL_CONF_GET_FLAG_DEL_SPLIT
 * VSTR_CNTL_CONF_SET_FLAG_DEL_SPLIT
 *
 * VSTR_CNTL_CONF_GET_FLAG_IOV_UPDATE
 * VSTR_CNTL_CONF_SET_FLAG_IOV_UPDATE
 *
 * VSTR_CNTL_CONF_GET_FMT_CHAR_ESC
 * VSTR_CNTL_CONF_SET_FMT_CHAR_ESC
 *
 * VSTR_CNTL_CONF_GET_LOC_CSTR_DEC_POINT
 * VSTR_CNTL_CONF_SET_LOC_CSTR_DEC_POINT
 *
 * VSTR_CNTL_CONF_GET_LOC_CSTR_NAME_NUMERIC
 * VSTR_CNTL_CONF_SET_LOC_CSTR_NAME_NUMERIC
 *
 * VSTR_CNTL_CONF_GET_LOC_CSTR_THOU_GRP
 * VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP
 *
 * VSTR_CNTL_CONF_GET_LOC_CSTR_THOU_SEP
 * VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP
 *
 * VSTR_CNTL_CONF_GET_LOC_REF_DEC_POINT
 * VSTR_CNTL_CONF_GET_LOC_REF_NAME_NUMERIC
 * VSTR_CNTL_CONF_GET_LOC_REF_NULL_PTR
 * VSTR_CNTL_CONF_GET_LOC_REF_THOU_GRP
 * VSTR_CNTL_CONF_GET_LOC_REF_THOU_SEP
 * VSTR_CNTL_CONF_SET_LOC_REF_DEC_POINT
 * VSTR_CNTL_CONF_SET_LOC_REF_NAME_NUMERIC
 * VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP
 *
 * VSTR_CNTL_CONF_GET_NUM_BUF_SZ
 *
 */
