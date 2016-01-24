/* libgmp formatters */

/* we only need output here, so turn off other IO functions */
#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_OPEN 1
#include "ex_utils.h" /* helper functions */

#include <limits.h>
#include <gmp.h>
#include <locale.h>

/* if this is enabled we add the malloc()d string as a reference,
 * which saves doing an extra copy. */
#define EX_GMP_NUMS_USE_REFS 0

/* do we want to test that it works? */
#define EX_GMP_NUMS_USE_TST 0

/* This is the custom formatter.
 * Note that this deals with grouping unlike the gmp_*printf() calls */
static int ex__usr_mpz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec,
                          char fmt, const mpz_t val)
{
  unsigned int nb = 10;
  int flags = VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM; /* it's a number */
  size_t len = 0;
  int ret = FALSE;
  char ui_buf[(sizeof(unsigned long) * CHAR_BIT) + 1];
  char *buf = NULL;
  char *out_buf = ui_buf;
  
  switch (fmt)
  {
    case 'd':                                                         break;
    case 'x': nb = 16; flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_HEXNUM_L; break;
    case 'o': nb =  8; flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_OCTNUM;   break;
    case 'b': nb =  2; flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_BINNUM_L; break;
    default: ASSERT(FALSE);                                           break;
  }
  
  if (mpz_sgn(val) == -1)
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NEG;
  
  if (mpz_fits_ulong_p(val))
    len = vstr_sc_conv_num_ulong(ui_buf, sizeof(ui_buf), mpz_get_ui(val),
                                 "0123456789abcdef", nb);
  else
  {
    len = mpz_sizeinbase(val, nb);
    out_buf = buf = mpz_get_str(NULL, nb, val); /* dies on malloc error */

    if (mpz_sgn(val) == -1) ++out_buf;
    if (!out_buf[len - 1])  --len; /* see documentation for mpz_sizeinbase() */
  }

  ASSERT(strlen(out_buf) == len);
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len, flags))
    goto mem_fail;
 
  if (spec->fmt_quote) /* add number including grouping */
    ret = vstr_sc_add_grpbasenum_buf(base, pos, nb, out_buf, len);
  else if (!EX_GMP_NUMS_USE_REFS || !buf) /* just add the number */
    ret = vstr_add_buf(base, pos, out_buf, len);
  else
  { /* assumes mp_set_memory_functions() hasn't been called */
    Vstr_ref *ref = vstr_ref_make_ptr(buf, vstr_ref_cb_free_ptr_ref);

    if (!ref)
      goto mem_fail;

    ret = vstr_add_ref(base, pos, ref, out_buf - buf, len);
    
    buf = NULL; /* memory is free'd when the reference is used up */

    /* if !ret then this will free buf */
    vstr_ref_del(ref);
  }
  
  if (!ret || !vstr_sc_fmt_cb_end(base, pos, spec, len))
    goto mem_fail;

  free(buf);
  
  return (TRUE);

 mem_fail:
  free(buf);
  return (FALSE);
}

static int ex_usr_dmpz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_mpz_cb(base, pos, spec, 'd', mpz));
}

static int ex_usr_ompz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_mpz_cb(base, pos, spec, 'o', mpz));
}

static int ex_usr_xmpz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_mpz_cb(base, pos, spec, 'x', mpz));
}

/* Extra, do binary output... */
static int ex_usr_bmpz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_mpz_cb(base, pos, spec, 'b', mpz));
}

static void *vmpz_num(unsigned int base, int val,
                      unsigned int *ern, void *bignum)
{
  *ern = 0;

  mpz_mul_ui(bignum, bignum, base);

  if (val > 0)
    mpz_add_ui(bignum, bignum, val);
  else
  {
    unsigned int uval = ++val;
    uval = -uval;
    ++uval;
    mpz_sub_ui(bignum, bignum, uval);
  }

  return (bignum);
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL);
  mpz_t bignum;
  const char *loc_num_name = NULL;
  Vstr_ref *ref = NULL;
  const unsigned int num_flags = (VSTR_FLAG_PARSE_NUM_SEP |
                                  VSTR_FLAG_PARSE_NUM_SPACE);
  
  if (argc < 2)
    errx(EXIT_FAILURE, "No count specified");

  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_fmt_add(s1->conf, "<dMPZ:%p>", ex_usr_dmpz_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "<oMPZ:%p>", ex_usr_ompz_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "<xMPZ:%p>", ex_usr_xmpz_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  vstr_fmt_add(s1->conf, "<bMPZ:%p>", ex_usr_bmpz_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);

  setlocale(LC_ALL, "");
  loc_num_name = setlocale(LC_NUMERIC, NULL);
  
  if (!vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_AUTO_NAME_NUMERIC,
                      loc_num_name))
    errx(EXIT_FAILURE, "Couldn't change numeric locale info");

  mpz_init(bignum);
  
  if (!vstr_add_cstr_ptr(s1, 0, argv[1]) ||
      !vstr_parse_num(s1, 1, s1->len, num_flags, NULL, NULL, vmpz_num, bignum))
    errno = ENOMEM, err(EXIT_FAILURE, "Couldn't init number");

  vstr_del(s1, 1, s1->len);
    
#if (EX_GMP_NUMS_USE_TST)
  {
    mpz_abs(bignum, bignum);
    mpz_neg(bignum, bignum);
  do
  {
    mpz_neg(bignum, bignum);

    vstr_add_fmt(s1, s1->len, "%%'20.40d = <$'20.40<dMPZ:%p>>\n",
                 (void *)bignum);
    if (mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%'20.40d = <%'20.40ld>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%'40.20d = <$'40.20<dMPZ:%p>>\n",
                 (void *)bignum);
    if (mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%'40.20d = <%'40.20ld>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%'20.40o = <$'20.40<oMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%'20.40o = <%'20.40lo>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%'40.20o = <$'40.20<oMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%'40.20o = <%'40.20lo>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%'20.40x = <$'20.40<xMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%'20.40x = <%'20.40lx>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%'40.20x = <$'40.20<xMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%'40.20x = <%'40.20lx>\n",
                 mpz_get_si(bignum));

    vstr_add_fmt(s1, s1->len, "%%#'40d   = <$#'40<dMPZ:%p>>\n",
                 (void *)bignum);
    if (mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%#'40d   = <%#'40ld>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%#'-40d  = <$#'-40<dMPZ:%p>>\n",
                 (void *)bignum);
    if (mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%#'-40d  = <%#'-40ld>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%#'40o   = <$#'40<oMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%#'40o   = <%#'40lo>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%#'-40o  = <$#'-40<oMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%#'-40o  = <%#'-40lo>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%#'40x   = <$#'40<xMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%#'40x   = <%#'40lx>\n",
                 mpz_get_si(bignum));
    vstr_add_fmt(s1, s1->len, "%%#'-40x  = <$#'-40<xMPZ:%p>>\n",
                 (void *)bignum);
    if ((mpz_sgn(bignum) != -1) && mpz_fits_slong_p(bignum))
    vstr_add_fmt(s1, s1->len, "%%#'-40x  = <%#'-40lx>\n",
                 mpz_get_si(bignum));

    vstr_add_fmt(s1, s1->len, "%%#'b  = <$#'<bMPZ:%p>>\n",
                 (void *)bignum);
    
    vstr_add_rep_chr(s1, s1->len, '-', 79);
    vstr_add_rep_chr(s1, s1->len, '\n', 1);
    
  } while (mpz_sgn(bignum) > 0);
    mpz_abs(bignum, bignum);
  }
#endif
  
  if (!(ref = vstr_ref_make_strdup("_")))
    errno = ENOMEM, err(EXIT_FAILURE, "Ref seperator");

  if (!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP,  2, ref, 1) ||
      !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP,  8, ref, 1) ||
      !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP, 16, ref, 1) ||
      FALSE)
    errno = ENOMEM, err(EXIT_FAILURE, "Add seperator");
  vstr_ref_del(ref);
  
  if (!(ref = vstr_ref_make_strdup("\4")))
    errno = ENOMEM, err(EXIT_FAILURE, "Ref grouping");

  if (!vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP,  2, ref) ||
      !vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP, 16, ref) ||
      FALSE)
    errno = ENOMEM, err(EXIT_FAILURE, "Add grouping");
  vstr_ref_del(ref);
  
  vstr_add_fmt(s1, s1->len, " Input: %s\n", argv[1]);
  vstr_add_fmt(s1, s1->len, "    %%#'x = $#'<xMPZ:%p>\n",
               (void *)bignum);
  vstr_add_fmt(s1, s1->len, "    %%#'d = $#'<dMPZ:%p>\n",
               (void *)bignum);
  vstr_add_fmt(s1, s1->len, "    %%#'o = $#'<oMPZ:%p>\n",
               (void *)bignum);
  vstr_add_fmt(s1, s1->len, "    %%#'b = $#'<bMPZ:%p>\n",
               (void *)bignum);
  
  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");
  
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
