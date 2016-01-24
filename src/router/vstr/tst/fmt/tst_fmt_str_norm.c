#include "tst-main.c"

static const char *rf = __FILE__;

#define CSTREQ(x, y) (strcmp(x, y) == 0)

int tst(void)
{
  int ret = 0;
  const char *t_fmt = "";
  const char *t_str = NULL;
  const wchar_t *t_wstr = NULL;
  Vstr_ref *ref = NULL;
  
  vstr_add_fmt(s1, 0, t_fmt);
  vstr_add_fmt(s1, 0, "%.*s %.*ls", -1, t_str, -3, t_wstr);
  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "(null) (null)"));
  vstr_del(s1, 1, s1->len);

  vstr_add_fmt(s1, 0, "%.*s %.*ls", 1, t_str, 2, t_wstr);
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "( (n"));
  
  ref = vstr_ref_make_strdup(".");
  ASSERT(ref);
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_LOC_REF_NULL_PTR,
                 ref, strlen(ref->ptr));
  vstr_ref_del(ref);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, t_fmt);
  vstr_add_fmt(s1, 0, "%.*s %.*ls", -1, t_str, -3, t_wstr);
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, ". ."));
  
  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%-3s%3s%3.1s%-3.1s%.s", "a", "b", "cdef", "def", "abcd");

  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "a    bcd"));

#if !USE_WIDE_CHAR_T
  return (TST_B_RET(ret));
#endif
  
  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%-3ls%3ls%3.1ls%-3.1ls%.ls",
               L"a", L"b", L"cdef", L"def", L"abcd");

  TST_B_TST(ret, 5, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "a    bcd"));

  {
    int spaces = 2;
    
    /* test for memory failures... */

    while (spaces < 10)
    {
      int mfail_count = 0;
      const char *fmt = "%s%*s%*s%s" "%ls%*ls%-*ls%S";
      
      vstr_del(s3, 1, s3->len);
      do
      {
        ASSERT(!s3->len);
        vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s3, 0, fmt,
                             "a", spaces, "b", -spaces, "c", "d",
                             L"a", spaces, L"b", -spaces, L"c", L"d"));
      sprintf(buf, fmt,
              "a", spaces, "b", -spaces, "c", "d",
              L"a", spaces, L"b", -spaces, L"c", L"d");
      
      TST_B_TST(ret, 6, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));
      tst_mfail_num(0);
      ++spaces;
    }
  }

  s1->conf->malloc_bad = FALSE;
  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%.*s %.*ls", 8, "abcd", 32, L"abcd");
  TST_B_TST(ret, 7, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "abcd abcd"));

  vstr_del(s1, 1, s1->len);
  if (!vstr_add_fmt(s1, 0, "%s%lc", "abcd", (wint_t)L'\xFEFE'))
  { /* glibc refuses badly encoded stuff */
    ASSERT(!s1->len && !s1->conf->malloc_bad);
    /* ASSERT(!vstr_add_fmt(s1, 0, "%s%ls", "abcd",
     *                      L"\x80\x80\x80\x80 123456789")); */
    ASSERT(!vstr_add_fmt(s1, 0, "%s%ls", "abcd", L"\xFEFE 123456789"));
    ASSERT(!s1->len && !s1->conf->malloc_bad);
  }
  else
    vstr_del(s1, 1, s1->len);

  vstr_add_fmt(s1, 0, "%.*ls", 1, L"abcd");
  TST_B_TST(ret, 6, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "a"));
  
  return (TST_B_RET(ret));
}
