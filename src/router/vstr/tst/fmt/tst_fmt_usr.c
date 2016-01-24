#include "tst-main.c"

static const char *rf = __FILE__;

#ifndef HAVE_POSIX_HOST
static int getpid(void) { return (0xdeadbeef); }
#endif

static int tst_errno = 0;

static char *tst_NULL = NULL;

static int tst_usr_vstr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  Vstr_base *sf          = VSTR_FMT_CB_ARG_PTR(spec, 0);
  unsigned int sf_flags  = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 1);

  assert(!strcmp(spec->name, "(VSTR:%p%u)"));

  if (!vstr_add_vstr(st, pos, sf, 1, sf->len, sf_flags))
    return (FALSE);

  return (TRUE);
}

static int tst_usr_pid_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  assert(!strcmp(spec->name, "PID"));

  if (!vstr_add_fmt(st, pos, "%lu", (unsigned long)getpid()))
    return (FALSE);

  return (TRUE);
}

static int tst_usr_intptr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  int *passed_num = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t len = 1;
  unsigned int num = 0;
  int flags = VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM; /* it's a number */
  int ret = FALSE;
  
  assert(!strcmp(spec->name, "[intptr]") ||
         !strcmp(spec->name, "[intptr:%n]") ||
         !strcmp(spec->name, "[intptr:%*n]"));

  ASSERT(passed_num);
  
  num = *passed_num; /* find out the "length" of the number */
  if (*passed_num < 0)
  {
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NEG;
    num = -num;
  }

  if ((num == 0xff) || (num == 0xeeb100b))
  {
    spec->fmt_plus = 0;
    len = vstr_sc_conv_num_uint(buf, sizeof(buf), num, "01yyyyyyyyybyyef", 16);
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_HEXNUM_L;
  }
  else if ((num == 0xfe) || (num == 0xeeb100c))
  {
    spec->fmt_plus = 0;
    len = vstr_sc_conv_num_uint(buf, sizeof(buf), num, "01yyyyyyyyyBCyEF", 16);
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_HEXNUM_H;
  }
  else if ((num == 0777) || (num == 01000))
  {
    spec->fmt_plus = 0;
    if (spec->fmt_hash && spec->fmt_precision)
      --spec->obj_precision;
    len = vstr_sc_conv_num_uint(buf, sizeof(buf), num, "01yyyyy7", 8);
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_OCTNUM;
  }
  else    
    len = vstr_sc_conv_num10_uint(buf, sizeof(buf), num);
  
  if (!vstr_sc_fmt_cb_beg(st, &pos, spec, &len, flags))
    return (FALSE);

  if (spec->fmt_quote)
    ret = vstr_sc_add_grpnum_buf(st, pos, buf, len);
  else
    ret = vstr_add_buf(st, pos, buf, len);

  if (!ret || !vstr_sc_fmt_cb_end(st, pos, spec, len))
    return (FALSE);
    
  return (TRUE);
}

static int tst_usr_sizptr_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  size_t *passed_num = VSTR_FMT_CB_ARG_PTR(spec, 0);
  unsigned int num = *passed_num;
  size_t len = vstr_sc_conv_num10_size(buf, sizeof(buf), num);
  int flags = VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM;
  int ret = FALSE;
 
  if (!vstr_sc_fmt_cb_beg(st, &pos, spec, &len, flags))
    return (FALSE);

  if (spec->fmt_quote)
    ret = vstr_sc_add_grpnum_buf(st, pos, buf, len);
  else
    ret = vstr_add_buf(st, pos, buf, len);

  if (!ret || !vstr_sc_fmt_cb_end(st, pos, spec, len))
    return (FALSE);
    
  return (TRUE);
}

static int tst_usr_blank_errno1_cb(Vstr_base *st, size_t pos,
                                   Vstr_fmt_spec *spec)
{
  size_t sf_len = 0;

  assert(st && !pos);
  assert(!strcmp(spec->name, "{BLANK_ERRNO1}"));

  assert(tst_errno == ERANGE);
  assert(errno == ERANGE);
  tst_errno = 0;
  errno = 0;

  if (!vstr_sc_fmt_cb_beg(st, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_DEF))
    return (FALSE);

  if (!vstr_sc_fmt_cb_end(st, pos, spec, sf_len))
    return (FALSE);


  return (TRUE);
}

static int tst_usr_blank_errno2_cb(Vstr_base *st, size_t pos,
                                   Vstr_fmt_spec *spec)
{
  assert(st && pos);
  assert(!strcmp(spec->name, "{BLANK_ERRNO2}"));

  assert(!tst_errno);
  assert(errno == ERANGE);
  tst_errno = 0;
  errno = 0;

  return (TRUE);
}

static int tst_usr_strerror_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  assert(!strcmp(spec->name, "m"));

  if (!VSTR_ADD_CSTR_PTR(st, pos, strerror(tst_errno)))
    return (FALSE);

  return (TRUE);
}

static int tst_usr_all_cb(Vstr_base *st, size_t pos, Vstr_fmt_spec *spec)
{
  double             arg1 = VSTR_FMT_CB_ARG_VAL(spec, double, 0);
  long double        arg2 = VSTR_FMT_CB_ARG_VAL(spec, long double, 1);
  int                arg3 = VSTR_FMT_CB_ARG_VAL(spec, int, 2);
  intmax_t           arg4 = VSTR_FMT_CB_ARG_VAL(spec, intmax_t, 3);
  long               arg5 = VSTR_FMT_CB_ARG_VAL(spec, long, 4);
  long long          arg6 = VSTR_FMT_CB_ARG_VAL(spec, long long, 5);
  ptrdiff_t          arg7 = VSTR_FMT_CB_ARG_VAL(spec, ptrdiff_t, 6);
  wchar_t           *arg8 = VSTR_FMT_CB_ARG_PTR(spec, 7);
  size_t             arg9 = VSTR_FMT_CB_ARG_VAL(spec, size_t, 8);
  ssize_t            arg10 = VSTR_FMT_CB_ARG_VAL(spec, ssize_t, 9);
  uintmax_t          arg11 = VSTR_FMT_CB_ARG_VAL(spec, uintmax_t, 10);
  unsigned long      arg12 = VSTR_FMT_CB_ARG_VAL(spec, unsigned long, 11);
  unsigned long long arg13 = VSTR_FMT_CB_ARG_VAL(spec, unsigned long long, 12);
  char              *arg14 = VSTR_FMT_CB_ARG_PTR(spec, 13);
  int                arg15 = VSTR_FMT_CB_ARG_VAL(spec, int, 14);
  signed char       *arg16 = VSTR_FMT_CB_ARG_PTR(spec, 15);
  short             *arg17 = VSTR_FMT_CB_ARG_PTR(spec, 16);
  int               *arg18 = VSTR_FMT_CB_ARG_PTR(spec, 17);
  long              *arg19 = VSTR_FMT_CB_ARG_PTR(spec, 18);
  long long         *arg20 = VSTR_FMT_CB_ARG_PTR(spec, 19);
  ssize_t           *arg21 = VSTR_FMT_CB_ARG_PTR(spec, 20);
  ptrdiff_t         *arg22 = VSTR_FMT_CB_ARG_PTR(spec, 21);
  intmax_t          *arg23 = VSTR_FMT_CB_ARG_PTR(spec, 22);

  ASSERT(spec->fmt_I);

  if (vstr_add_fmt(st, pos,
                   "%f|%Lf|%8d|%8jd|%8ld|%8lld|%8td|%ls|%8zu|%8zd|"
                   "%8ju|%8lu|%8llu|%s||%hhd|%hd|%d|%ld|%lld|%zd|%td|%jd|%c",
                   arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10,
                   arg11, arg12, arg13, arg14,
                   *arg16, *arg17, *arg18, *arg19,
                   *arg20, *arg21, *arg22, *arg23, 
                   arg15))
    return (TRUE);

  return (FALSE);
}

static void tst_fmt(int ret, Vstr_base *t1)
{
  int succeeded = FALSE;
  const char *t_fmt = "$I{TST_ALL}";
  signed char arg16 = 16;
  short arg17 = 17;
  int arg18 = 18;
  long arg19 = 19;
  long long arg20 = 20;
  ssize_t arg21 = 21;
  ptrdiff_t arg22 = 22;
  intmax_t arg23 = 23;
  const char *correct = ("1.000000|2.000000|"
                         "       3|       4|"
                         "       5|       6|"
                         "       7|"
                         "eight|"
                         "       9|      10|"
                         "      11|      12|"
                         "      13|"
                         "fourteen||"
                         "16|17|18|19|20|21|22|23|");

#ifdef FMT_DBL_none
  correct = ("0.000000|0.000000|"
             "       3|       4|"
             "       5|       6|"
             "       7|"
             "eight|"
             "       9|      10|"
             "      11|      12|"
             "      13|"
             "fourteen||"
             "16|17|18|19|20|21|22|23|");
#endif

  vstr_del(t1, 1, t1->len);

#if !USE_WIDE_CHAR_T
  return;
#endif  
  
  if (!MFAIL_NUM_OK)
  {
    succeeded = vstr_add_fmt(t1, t1->len, t_fmt,
                             1.0, (long double)2.0,
                             3, (intmax_t)4,
                             5L, 6LL,
                             (ptrdiff_t)7,
                             L"eight",
                             (size_t)9, (ssize_t)10, (uintmax_t)11,
                             12UL, 13ULL, "fourteen", 0,
                             &arg16, &arg17, &arg18, &arg19,
                             &arg20, &arg21, &arg22, &arg23);
    ASSERT(succeeded);
  }
  else
  {
    unsigned long mfail_count = 1;

    vstr_free_spare_nodes(t1->conf,     VSTR_TYPE_NODE_BUF, 1000);

    TST_B_TST(ret, 26, !tst_mfail_num(1));
    TST_B_TST(ret, 27,  vstr_add_fmt(t1, t1->len, t_fmt,
                                     1.0, (long double)2.0,
                                     3, (intmax_t)4,
                                     5L, 6LL,
                                     (ptrdiff_t)7,
                                     L"eight",
                                     (size_t)9, (ssize_t)10, (uintmax_t)11,
                                     12UL, 13ULL, "fourteen", 0,
                                     &arg16, &arg17, &arg18, &arg19,
                                     &arg20, &arg21, &arg22, &arg23));

    while (!succeeded) /* keep trying until we succeed now */
    {
      vstr_free_spare_nodes(t1->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
      succeeded = vstr_add_fmt(t1, t1->len, t_fmt,
                               1.0, (long double)2.0,
                               3, (intmax_t)4,
                               5L, 6LL,
                               (ptrdiff_t)7,
                               L"eight",
                               (size_t)9, (ssize_t)10, (uintmax_t)11,
                               12UL, 13ULL, "fourteen", 0,
                               &arg16, &arg17, &arg18, &arg19,
                               &arg20, &arg21, &arg22, &arg23);
    }
    tst_mfail_num(0);
  }

  TST_B_TST(ret, 28, !VSTR_CMP_BUF_EQ(t1, 1, t1->len,
                                      correct, strlen(correct) + 1));
}

#define IPV4(n1, n2) (((n1) << 8) | (n2))

int tst(void)
{
  int ret = 0;
  Vstr_ref *ref = NULL;
  int mfail_count = 0;
  int num = 0;
  unsigned int ipv4[4];
  unsigned int ipv6[8];

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  vstr_add_fmt(s2, s2->len, "$ %s -- %lu $", buf, (unsigned long)getpid());

  ASSERT(s1->conf->fmt_usr_curly_braces);
  ASSERT(s2->conf->fmt_usr_curly_braces);
  ASSERT(s3->conf->fmt_usr_curly_braces);

  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s2->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_cntl_conf(s4->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '%');

  /* setup custom formatters */
  /* test for memory failures... */
  do
  {
    tst_mfail_num(++mfail_count);
  } while (!vstr_fmt_add(s1->conf, "(VSTR:%p%u)", tst_usr_vstr_cb,
                         VSTR_TYPE_FMT_PTR_VOID,
                         VSTR_TYPE_FMT_UINT,
                         VSTR_TYPE_FMT_END));
  tst_mfail_num(0);

  ASSERT(!vstr_fmt_add(s1->conf, "(VSTR:%p%u)", tst_usr_vstr_cb,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));

  ASSERT(vstr_fmt_srch(s1->conf, "(VSTR:%p%u)"));
  ASSERT(s1->conf->fmt_usr_curly_braces);
  
  vstr_fmt_add(s1->conf, "(0)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(1)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(2)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(3)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(6)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(7)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(8)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(9)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(a)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(b)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(c)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(d)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(e)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(f)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(g)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(h)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(i)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(j)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(k)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(l)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(m)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(n)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(o)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(p)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(q)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(r)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(s)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(t)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(u)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(v)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(w)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(x)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(y)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(z)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(!)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(@)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(#)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "($)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(%)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(^)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "(*)", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "([]<>{})", tst_usr_pid_cb, VSTR_TYPE_FMT_END);

  ASSERT(s1->conf->fmt_usr_curly_braces);

  vstr_fmt_add(s1->conf, "{TST_ALL}", tst_usr_all_cb,
               VSTR_TYPE_FMT_DOUBLE,
               VSTR_TYPE_FMT_DOUBLE_LONG,
               VSTR_TYPE_FMT_INT,
               VSTR_TYPE_FMT_INTMAX_T,
               VSTR_TYPE_FMT_LONG,
               VSTR_TYPE_FMT_LONG_LONG,
               VSTR_TYPE_FMT_PTRDIFF_T,
               VSTR_TYPE_FMT_PTR_WCHAR_T,
               VSTR_TYPE_FMT_SIZE_T,
               VSTR_TYPE_FMT_SSIZE_T,
               VSTR_TYPE_FMT_UINTMAX_T,
               VSTR_TYPE_FMT_ULONG,
               VSTR_TYPE_FMT_ULONG_LONG,
               VSTR_TYPE_FMT_PTR_CHAR,
               VSTR_TYPE_FMT_INT,
               VSTR_TYPE_FMT_PTR_SIGNED_CHAR,
               VSTR_TYPE_FMT_PTR_SHORT,
               VSTR_TYPE_FMT_PTR_INT,
               VSTR_TYPE_FMT_PTR_LONG,
               VSTR_TYPE_FMT_PTR_LONG_LONG,
               VSTR_TYPE_FMT_PTR_SSIZE_T,
               VSTR_TYPE_FMT_PTR_PTRDIFF_T,
               VSTR_TYPE_FMT_PTR_INTMAX_T,
               VSTR_TYPE_FMT_END);
  vstr_fmt_add(s3->conf, "{TST_ALL}", tst_usr_all_cb,
               VSTR_TYPE_FMT_DOUBLE,
               VSTR_TYPE_FMT_DOUBLE_LONG,
               VSTR_TYPE_FMT_INT,
               VSTR_TYPE_FMT_INTMAX_T,
               VSTR_TYPE_FMT_LONG,
               VSTR_TYPE_FMT_LONG_LONG,
               VSTR_TYPE_FMT_PTRDIFF_T,
               VSTR_TYPE_FMT_PTR_WCHAR_T,
               VSTR_TYPE_FMT_SIZE_T,
               VSTR_TYPE_FMT_SSIZE_T,
               VSTR_TYPE_FMT_UINTMAX_T,
               VSTR_TYPE_FMT_ULONG,
               VSTR_TYPE_FMT_ULONG_LONG,
               VSTR_TYPE_FMT_PTR_CHAR,
               VSTR_TYPE_FMT_INT,
               VSTR_TYPE_FMT_PTR_SIGNED_CHAR,
               VSTR_TYPE_FMT_PTR_SHORT,
               VSTR_TYPE_FMT_PTR_INT,
               VSTR_TYPE_FMT_PTR_LONG,
               VSTR_TYPE_FMT_PTR_LONG_LONG,
               VSTR_TYPE_FMT_PTR_SSIZE_T,
               VSTR_TYPE_FMT_PTR_PTRDIFF_T,
               VSTR_TYPE_FMT_PTR_INTMAX_T,
               VSTR_TYPE_FMT_END);

  ref = vstr_ref_make_ptr("(BAD-BAD)", vstr_ref_cb_free_ref);
  ASSERT(vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_LOC_REF_NULL_PTR,
                        ref, strlen(ref->ptr)));
  vstr_ref_del(ref);
  
  vstr_sc_fmt_add_all(s3->conf);
  ASSERT(s3->conf->fmt_usr_curly_braces);
  vstr_fmt_add(s3->conf, "{BLANK_ERRNO1}", tst_usr_blank_errno1_cb,
               VSTR_TYPE_FMT_ERRNO, VSTR_TYPE_FMT_END);
  ASSERT(s3->conf->fmt_usr_curly_braces);
  vstr_fmt_add(s3->conf, "{BLANK_ERRNO2}", tst_usr_blank_errno2_cb,
               VSTR_TYPE_FMT_ERRNO, VSTR_TYPE_FMT_END);
  ASSERT(s3->conf->fmt_usr_curly_braces);
  vstr_fmt_add(s3->conf, "[intptr:%n]", tst_usr_intptr_cb,
               VSTR_TYPE_FMT_PTR_INT, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s3->conf, "[intptr:%*n]", tst_usr_intptr_cb,
               VSTR_TYPE_FMT_PTR_INT, VSTR_TYPE_FMT_END);
  ASSERT(vstr_fmt_srch(s3->conf, "[intptr:%n]"));
  ASSERT(s3->conf->fmt_usr_curly_braces);
  vstr_fmt_add(s3->conf, "PID", tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  ASSERT(vstr_fmt_srch(s3->conf, "PID"));
  ASSERT(!s3->conf->fmt_usr_curly_braces);

  vstr_fmt_add(s2->conf, "[intptr]", tst_usr_intptr_cb,
               VSTR_TYPE_FMT_PTR_INT, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s2->conf, "[sizptr]", tst_usr_sizptr_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s2->conf, "[sizptr:%p]", tst_usr_sizptr_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  
  vstr_fmt_add(s4->conf, "m", tst_usr_strerror_cb, VSTR_TYPE_FMT_END);

  /* output */

  vstr_add_fmt(s1, 0, "$(VSTR:%p%u)", (void *)s2, 0);

  TST_B_TST(ret, 1, !VSTR_CMP_EQ(s1, 1, s1->len, s2, 1, s2->len));

  vstr_add_fmt(s3, 0, "$$ ${vstr:%p%zu%zu%u} -- $PID $$",
               (void *)s2, (size_t)3, strlen(buf), 0);

  TST_B_TST(ret, 2, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, 0, "$$ ${buf:%s%zu} -- $PID $$",
               buf, strlen(buf));

  TST_B_TST(ret, 3, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, 0, "$$ ${ptr:%s%zu} -- $PID $$",
               buf, strlen(buf));

  TST_B_TST(ret, 4, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s3, 1, s3->len);
  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s3, 0, " ${non:%zu} ", strlen(buf));
  vstr_add_rep_chr(s1, s1->len, ' ', 1);
  vstr_add_non(s1, 1, strlen(buf));
  vstr_add_rep_chr(s1, s1->len, ' ', 1);

  TST_B_TST(ret, 5, !VSTR_CMP_EQ(s3, 1, s3->len, s1, 1, s1->len));

  vstr_del(s3, 1, s3->len);
  ref = vstr_ref_make_ptr(buf, vstr_ref_cb_free_ref);
  vstr_add_fmt(s3, 0, "$$ $*{ref:%*p%zu%zu} -- $PID $$",
               0, ref, (size_t)0, strlen(buf));
  vstr_ref_del(ref);

  TST_B_TST(ret, 6, !VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s2, 3, strlen(buf));
  vstr_del(s3, 1, s3->len);
  ref = vstr_ref_make_ptr("123", vstr_ref_cb_free_ref);
  vstr_add_fmt(s3, 0, "$$ $*.*{ref:%d%d%p%zu%zu} -- $PID $$",
               4, 0, ref, (size_t)0, strlen(buf));
  vstr_ref_del(ref);

  assert(VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s3, 1, s3->len);
  ref = vstr_ref_make_ptr("--", vstr_ref_cb_free_ref);
  vstr_add_fmt(s3, 0, "$$$*.*{ref:%d%d%p%zu%zu} $PID $$",
               4, 5, ref, (size_t)0, (size_t)2);
  vstr_ref_del(ref);

  assert(VSTR_CMP_EQ(s3, 1, s3->len, s2, 1, s2->len));

  vstr_del(s1, 1, s1->len);
  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, 0, "X${rep_chr:%c%zu}X", '-', (size_t)43);
  vstr_add_rep_chr(s1, s1->len, 'X', 1);
  vstr_add_rep_chr(s1, s1->len, '-', 43);
  vstr_add_rep_chr(s1, s1->len, 'X', 1);

  TST_B_TST(ret, 7, !VSTR_CMP_EQ(s3, 1, s3->len, s1, 1, s1->len));

  tst_errno = ERANGE;

  vstr_del(s4, 1, s4->len);
  vstr_add_fmt(s4, 0, "%d", 1);
  TST_B_TST(ret, 8, !VSTR_CMP_CSTR_EQ(s4, 1, s4->len, "1"));

  vstr_del(s4, 1, s4->len);
  errno = 0;
  vstr_add_fmt(s4, 0, "%m");

  assert(tst_errno == ERANGE);

  TST_B_TST(ret, 9, !VSTR_CMP_CSTR_EQ(s4, 1, s4->len, strerror(ERANGE)));

  vstr_del(s3, 1, s3->len);
  errno = tst_errno;
  vstr_add_fmt(s3, 0, "%m");

  TST_B_TST(ret, 10, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, strerror(ERANGE)));

  vstr_del(s3, 1, s3->len);
  errno = tst_errno;
  vstr_add_fmt(s3, 0,
               "${BLANK_ERRNO1}"
               "%m"
               "${BLANK_ERRNO2}");
  assert(!tst_errno);
  assert(!errno && !tst_errno);

  TST_B_TST(ret, 11, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, strerror(ERANGE)));

  vstr_del(s4, 1, s4->len);
  tst_errno = 0;
  errno = ERANGE;
  vstr_add_sysfmt(s4, 0, "%m");

  TST_B_TST(ret, 12, !VSTR_CMP_CSTR_EQ(s4, 1, s4->len, strerror(ERANGE)));

  /* en_US grouping */
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP, ",");
  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\3");

  num = 5;
  while (num--)
  {
    const char *fmt = NULL;
    int spaces = 2;
    
    #define FMT(x) (fmt = (((num == 0xff) || (num == 0xeeb100b)) ? "<%" x "x>" : ((num == 0xfe) || (num == 0xeeb100c)) ? "<%" x "X>" : ((num == 0777) || (num == 01000)) ? "<%" x "o>" : "<%" x "d>"))
    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    vstr_add_fmt(s3, s3->len, "<$[intptr:%n]>", &num);
    vstr_add_fmt(s4, s4->len, FMT(""), num);

    TST_B_TST(ret, 13, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));
    
    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    vstr_add_fmt(s3, s3->len, "<$7[intptr:%n]>", &num);
    vstr_add_fmt(s4, s4->len, FMT("7"), num);
    
    TST_B_TST(ret, 14, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));
    
    {
      int tmp = 0;

      vstr_del(s2, 1, s2->len);
      vstr_add_fmt(s2, 0, "$1$[intptr]%1$n$1$3[intptr] %1$n$1$[intptr]", &tmp);
      ASSERT(VSTR_CMP_CSTR_EQ(s2, 1, s2->len, "0  1 5"));
      vstr_del(s2, 1, s2->len);
      vstr_add_fmt(s2, 0, "$[sizptr:%p]$1$3[sizptr] $1$[sizptr]",
                   (void *)&s2->len);
      ASSERT(VSTR_CMP_CSTR_EQ(s2, 1, s2->len, "0  1 5"));
    }
    
    vstr_del(s2, 1, s2->len);
    spaces = 0;
    while (spaces <= 16)
    {
      vstr_del(s3, 1, s3->len);
      vstr_del(s4, 1, s4->len);

      mfail_count = 0;
      do
      {
        vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
        tst_mfail_num(++mfail_count);
      } while (!vstr_add_fmt(s3, s3->len, "%1$*2$s<$3$+0*2$[intptr:%*n]>",
                             "", spaces, &num));

      tst_mfail_num(0);
      FMT("+0*");
      vstr_add_rep_chr(s4, s4->len, ' ', spaces);
      vstr_add_fmt(s4, s4->len, fmt, spaces, num);
      
    TST_B_TST(ret, 15, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));

    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);

    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$.7[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT(".7");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s4, s4->len, fmt, num);
    
    TST_B_TST(ret, 16, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));
    
    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$#-7[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT("#-7");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s4, s4->len, fmt, num);
  
    TST_B_TST(ret, 17, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));

    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$-16.8[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT("-16.8");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s4, s4->len, fmt, num);
  
    TST_B_TST(ret, 18, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));

    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$16.8[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT("16.8");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s4, s4->len, fmt, num);
  
    TST_B_TST(ret, 19, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));

    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$#8.16[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT("#8.16");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s4, s4->len, fmt, num);
  
    TST_B_TST(ret, 20, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));

    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$'-8.16[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT("'-8.16");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s2, s2->len, fmt, num);
    vstr_mov(s4, s4->len, s2, 1, s2->len);
  
    TST_B_TST(ret, 21, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));

    vstr_del(s3, 1, s3->len);
    vstr_del(s4, 1, s4->len);
    mfail_count = 0;
    do
    {
      ASSERT(!s3->len);
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
    } while (!vstr_add_fmt(s3, s3->len, "%*s<$#'-16.8[intptr:%n]>", spaces, "",
                           &num));
    tst_mfail_num(0);
    FMT("#'-16.8");
    vstr_add_rep_chr(s4, s4->len, ' ', spaces);
    vstr_add_fmt(s2, s2->len, fmt, num);
    vstr_mov(s4, s4->len, s2, 1, s2->len);
  
    TST_B_TST(ret, 21, !VSTR_CMP_EQ(s3, 1, s3->len, s4, 1, s4->len));
    
    ++spaces;
    }

    switch (num)
    {
      case    4:      num =         -3; break;
      case   -4:      num =      0x100; break;
      case 0xfe:      num =  0xeeb100d; break;
      case 0xeeb100b: num =      01001; break;
      case 0777:      num =          1; break;
      case    0:                        break;
      default:                          break;
    }
  }
  
  
  vstr_fmt_del(s1->conf, "(VSTR:%p%u)");
  assert(!vstr_fmt_srch(s1->conf, "(VSTR:%p%u)"));

  tst_fmt(ret, s1);
  tst_fmt(ret, s3);

  TST_B_TST(ret, 27, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  ipv4[0] = ipv4[1] = ipv4[2] = ipv4[3] = 111;
  ipv6[0] = ipv6[1] = ipv6[2] = 0x22;
  ipv6[3] = ipv6[4] = 0;
  ipv6[5] = 0x222;
  ipv6[6] = ipv6[7] = IPV4(111U, 111);

  ref = vstr_ref_make_ptr((char *)"--ref--", vstr_ref_cb_free_ref);
  ASSERT(ref);
  
  vstr_del(s1, 1, s1->len);
  vstr_del(s2, 1, s2->len);
  vstr_del(s3, 1, s3->len);

  vstr_add_cstr_ref(s2, 0, ref, 0);
  
  mfail_count = 0;

  /* allocate fmt nodes... */
  vstr_add_fmt(s3, 0, "%s%s%s%s%.d%s%s%s%s%.d%s%s%s%s%.d%s%s%s%s%.d%s%s%s%s%.d",
               "", "", "", "", 0, "", "", "", "", 0, "", "", "", "", 0,
               "", "", "", "", 0, "", "", "", "", 0);
  do
  {
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(s3, 0,
                         "$16{vstr:%p%zu%zu%u}|"
                         "$16{buf:%s%zu}|"
                         "$16{ptr:%s%zu}|"
                         "$16{non:%zu}|"
                         "$16{ref:%p%zu%zu}|"
                         "$16{rep_chr:%c%zu}|"

                         "$76{ipv4.v:%p}|"
                         "$76{ipv6.v:%p%u}|"
                         
                         "$76{ipv4.v+C:%p%u}|"

                         "$76{ipv6.v+C:%p%u%u}|"
                         "$76{ipv6.v+C:%p%u%u}|"
                         "$76{ipv6.v+C:%p%u%u}|"

                         "$76{ipv6.v+C:%p%u%u}|"
                         "$76{ipv6.v+C:%p%u%u}|"
                         "$76{ipv6.v+C:%p%u%u}"
                         "",
                         s2, (size_t)1, s2->len, 0,
                         "--buf--", (size_t)7,
                         "--ptr--", (size_t)7,
                         (size_t)7,
                         ref, (size_t)0, (size_t)7,
                         'x', (size_t)8,
                         ipv4,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED,
                         ipv4, 24,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_STD, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD, 96));
  tst_mfail_num(0);

  vstr_add_fmt(s1, s1->len, "%s|%s|%s|         ",
               "         --ref--", "         --buf--", "         --ptr--");
  vstr_add_non(s1, s1->len, 7);
  vstr_add_fmt(s1, s1->len, "|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s",
               "         --ref--", "        xxxxxxxx",
"                                                             111.111.111.111",
"                                     0022:0022:0022:0000:0000:0222:6F6F:6F6F",
"                                                          111.111.111.111/24",
"                                  0022:0022:0022:0000:0000:0222:6F6F:6F6F/96",
"                                                  22:22:22::222:6F6F:6F6F/96",
"                                               22:22:22:0:0:222:6F6F:6F6F/96",
"                            0022:0022:0022:0000:0000:0222:111.111.111.111/96",
"                                            22:22:22::222:111.111.111.111/96",
"                                         22:22:22:0:0:222:111.111.111.111/96");
  
  TST_B_TST(ret, 28, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));
  
  vstr_del(s1, 1, s1->len);
  vstr_del(s3, 1, s3->len);

  mfail_count = 0;
  
  do
  {
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_NON, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_REF, 1000);
    tst_mfail_num(++mfail_count);
  } while (!vstr_add_fmt(s3, 0,
                         "$-16{vstr:%p%zu%zu%u}|"
                         "$-16{buf:%s%zu}|"
                         "$-16{ptr:%s%zu}|"
                         "$-16{non:%zu}|"
                         "$-16{ref:%p%zu%zu}|"
                         "$-16{rep_chr:%c%zu}|"
                         "$-76{ipv4.v:%p}|"
                         "$-76{ipv6.v:%p%u}|"

                         "$-76{ipv4.v+C:%p%u}|"

                         "$-76{ipv6.v+C:%p%u%u}|"
                         "$-76{ipv6.v+C:%p%u%u}|"
                         "$-76{ipv6.v+C:%p%u%u}|"

                         "$-76{ipv6.v+C:%p%u%u}|"
                         "$-76{ipv6.v+C:%p%u%u}|"
                         "$-76{ipv6.v+C:%p%u%u}"
                         "",
                         s2, (size_t)1, s2->len, 0,
                         "--buf--", (size_t)7,
                         "--ptr--", (size_t)7,
                         (size_t)7,
                         ref, (size_t)0, (size_t)7,
                         'x', (size_t)8,
                         ipv4,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED,
                         ipv4, 24,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_STD, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 96,
                         ipv6, VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD, 96));
  tst_mfail_num(0);

  vstr_add_fmt(s1, s1->len, "%s|%s|%s|",
               "--ref--         ", "--buf--         ", "--ptr--         ");
  vstr_add_non(s1, s1->len, 7);
  vstr_add_fmt(s1, s1->len, "         |%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s",
               "--ref--         ", "xxxxxxxx        ",
"111.111.111.111                                                             ",
"0022:0022:0022:0000:0000:0222:6F6F:6F6F                                     ",
"111.111.111.111/24                                                          ",
"0022:0022:0022:0000:0000:0222:6F6F:6F6F/96                                  ",
"22:22:22::222:6F6F:6F6F/96                                                  ",
"22:22:22:0:0:222:6F6F:6F6F/96                                               ",
"0022:0022:0022:0000:0000:0222:111.111.111.111/96                            ",
"22:22:22::222:111.111.111.111/96                                            ",
"22:22:22:0:0:222:111.111.111.111/96                                         ");

  TST_B_TST(ret, 29, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));

  vstr_ref_del(ref);
  
  /* flush the max name length cache ... */
  vstr_fmt_add(s3->conf, "{ref-xxxxxxxxxxxxxxxxxxxxxxx}",
               tst_usr_pid_cb, VSTR_TYPE_FMT_END);
  vstr_fmt_del(s3->conf, "{ref-xxxxxxxxxxxxxxxxxxxxxxx}");

  /* this also tests look without max names... */

  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, 0, "<$.4{buf:%s%zu}> <${buf:%s%zu}> <${buf:%s%zu}>",
               tst_NULL, 20, tst_NULL, 4, tst_NULL, 20);

  assert(VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "<(BAD> <(BAD> <(BAD-BAD)>"));

  vstr_del(s3, 1, s3->len);
  vstr_add_fmt(s3, 0, "<$.4{ptr:%s%zu}> <${ptr:%s%zu}> <${ptr:%s%zu}>",
               tst_NULL, 20, tst_NULL, 4, tst_NULL, 20);

  assert(VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "<(BAD> <(BAD> <(BAD-BAD)>"));

  /* i18n on usr fmt */
  {
    const char *fmt_hide_gcc = ("<$3$.*7${buf:%s%zu}> "
                                "<$1$*8${buf:%s%zu}> "
                                "<$5${buf:%s%zu}>");
    
    vstr_del(s3, 1, s3->len);
    vstr_add_fmt(s3, 0, fmt_hide_gcc,
                 "1abcd", 5, "2abcd", 5, "3abcd", 5,
                 3 /* precision for first */,
                 8 /* field width for second */);

    assert(VSTR_CMP_CSTR_EQ(s3, 1, s3->len, "<2ab> <   1abcd> <3abcd>"));
  }
  


  return (TST_B_RET(ret));
}
/* Crap for tst_coverage constants....
 *
 * VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR
 * VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM
 *
 */
