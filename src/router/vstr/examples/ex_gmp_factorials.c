/* Simple libgmp command, see...

http://www.gnu.org/manual/gmp/html_node/Converting-Integers.html
http://www.gnu.org/manual/gmp/html_node/Integer-Comparisons.html
http://www.gnu.org/manual/gmp/html_node/Miscellaneous-Integer-Functions.html
http://www.gnu.org/manual/gmp/html_node/Integer-Arithmetic.html

http://www.gnu.org/manual/gmp/html_node/Formatted-Output-Strings.html


...it prints the factorial of the first argument, you can check the output
against...

http://www.newdream.net/~sage/old/numbers/fact.htm

...or a POSIX "bc" via. ...

  define f (x) {
    if (x <= 1) return (1);
    return (f(x-1) * x);
  }

*/

/* we only need output here, so turn off other IO functions */
#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_OPEN 1
#include "ex_utils.h" /* helper functions */

#include <limits.h>
#include <gmp.h>
#include <locale.h>

/* if this is enabled we go through the factorials twice, which means we do
 * almost twice as much work ... the output is more readable for small values
 * though */
#define EX_GMP_FACT_USE_FIELDWIDTH 1

/* if this is enabled we add the malloc()d string as a reference,
 * which saves doing an extra copy. */
#define EX_GMP_FACT_USE_REFS 1

/* This is the custom formatter.
 * Note that this deals with grouping unlike the gmp_*printf() calls */
static int ex__usr_mpz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec,
                          /* gmp args, need to be in paramter list */
                          const mpz_t val)
{
  int flags = VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM; /* it's a number */
  size_t len = 0;
  int ret = FALSE;
  char ui_buf[sizeof(unsigned long) * CHAR_BIT];
  char *buf = NULL;
  char *out_buf = ui_buf;

  if (mpz_sgn(val) == -1) /* it's a negative number */
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NEG;
  
  if (mpz_fits_ulong_p(val)) /* it's a simple number */
    len = vstr_sc_conv_num10_ulong(ui_buf, sizeof(ui_buf), mpz_get_ui(val));
  else /* bignum, so get libgmp to export it as a string */
  {
    len = mpz_sizeinbase(val, 10); /* doesn't include minus sign */
    out_buf = buf = mpz_get_str(NULL, 10, val); /* dies on malloc error */

    if (mpz_sgn(val) == -1) ++out_buf; /* skip the minus sign */
    if (!out_buf[len - 1])  --len; /* see documentation for mpz_sizeinbase() */
  }

  ASSERT(strlen(out_buf) == len);
  
  /* this deals with things like having the the zero flag (Ie. %0d), or the
   * plus flag (Ie. %+d) or right shifted field widths */
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len, flags))
    goto mem_fail;
 
  if (spec->fmt_quote) /* add number including grouping */
    ret = vstr_sc_add_grpnum_buf(base, pos, out_buf, len);
  else if (!EX_GMP_FACT_USE_REFS || !buf) /* just add the number */
    ret = vstr_add_buf(base, pos, out_buf, len);
  else
  { /* create a reference to avoid copying data,
     * assumes mp_set_memory_functions() hasn't been called */
    Vstr_ref *ref = vstr_ref_make_ptr(buf, vstr_ref_cb_free_ptr_ref);

    if (!ref)
      goto mem_fail;

    ret = vstr_add_ref(base, pos, ref, out_buf - buf, len);
    
    buf = NULL; /* memory is free'd when the reference is used up */

    /* remove our reference, if !ret then this will free buf */
    vstr_ref_del(ref);
  }
  
  /* this deals with left shifted field widths */
  if (!ret || !vstr_sc_fmt_cb_end(base, pos, spec, len))
    goto mem_fail;

  free(buf);
  
  return (TRUE);

 mem_fail:
  free(buf);
  return (FALSE);
}

/* we need to jump though an extra function due to the way GMP defines the
 * mpz_t type */
static int ex_usr_mpz_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_mpz_cb(base, pos, spec, mpz));
}

/* The code to calculate the factorial... */
static void ex_gmp_fact(mpz_t bignum_ret, mpz_t bignum_cnt, mpz_t bignum_for,
                        int out, Vstr_base *s1, int ret_max_sz, int cnt_max_sz)
{
  while (mpz_cmp(bignum_cnt, bignum_for) <= 0)
  {
    int w_state = IO_OK;
    
    mpz_mul(bignum_ret, bignum_ret, bignum_cnt);

    if (out)
    { /* output the current values */
      vstr_add_fmt(s1, s1->len, "$'*<MPZ:%*p>%s %c $'*<MPZ:%*p>\n",
                   cnt_max_sz, (void *)bignum_cnt, "!", '=',
                   ret_max_sz, (void *)bignum_ret);
    
      if (s1->conf->malloc_bad)
        errno = ENOMEM, err(EXIT_FAILURE, "Add string data");

      w_state = io_put(s1, STDOUT_FILENO);
    
      if ((w_state == IO_BLOCK) && (s1->len > EX_MAX_W_DATA_INCORE))
        io_block(-1, STDOUT_FILENO);
    }
    
    mpz_add_ui(bignum_cnt, bignum_cnt, 1);
  }
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL);
  mpz_t bignum_ret;
  mpz_t bignum_for;
  mpz_t bignum_cnt;
  int cnt_max_sz = 1;
  int ret_max_sz = 1;
  const char *loc_num_name = NULL;
  
  if (argc < 2)
    errx(EXIT_FAILURE, "No count specified");

  /* setup the custom format specifier for GMP ... see above
   */
  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_fmt_add(s1->conf, "<MPZ:%p>", ex_usr_mpz_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);
  /* second version so we can give a field width */
  vstr_fmt_add(s1->conf, "<MPZ:%*p>", ex_usr_mpz_cb,
               VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END);

  /* get the numeric locale name... */
  setlocale(LC_ALL, "");
  loc_num_name = setlocale(LC_NUMERIC, NULL);
  
  /* change grouping, from locale, to make numbers more readable */
  if (!vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_AUTO_NAME_NUMERIC,
                      loc_num_name))
    errx(EXIT_FAILURE, "Couldn't change numeric locale info");
  
  mpz_init_set_str(bignum_for, argv[1], 0);
  mpz_init_set_str(bignum_ret,     "1", 0);
  mpz_init_set_str(bignum_cnt,     "1", 0);

  if (EX_GMP_FACT_USE_FIELDWIDTH)
  { /* find out the max length of the for values... */

    /* value of the count... */
    vstr_add_fmt(s1, s1->len, "$'<MPZ:%p>", (void *)bignum_for);
    if (s1->conf->malloc_bad) /* this checks a bunch of things above */
      errno = ENOMEM, err(EXIT_FAILURE, "Add string data");
  
    cnt_max_sz = s1->len; vstr_del(s1, 1, s1->len);

    /* work out the result */
    if (mpz_fits_ulong_p(bignum_for))
      mpz_fac_ui(bignum_ret, mpz_get_ui(bignum_for));
    else
      ex_gmp_fact(bignum_ret, bignum_cnt, bignum_for, FALSE, NULL, 0, 0);

    /* value of the result... */
    if (!vstr_add_fmt(s1, s1->len, "$'<MPZ:%p>", (void *)bignum_ret))
      errno = ENOMEM, err(EXIT_FAILURE, "Add string data");
  
    ret_max_sz = s1->len; vstr_del(s1, 1, s1->len);

    /* reinit, so we can print everything...  */
    mpz_init_set_str(bignum_ret,     "1", 0);
    mpz_init_set_str(bignum_cnt,     "1", 0);
  }
  
  /* do the output... */
  if (mpz_sgn(bignum_for) >= 0) /* special case 0! */
    if (!vstr_add_fmt(s1, s1->len, "%*u%s %c %*u\n\n",
                      cnt_max_sz, 0, "!", '=', ret_max_sz, 1))
      errno = ENOMEM, err(EXIT_FAILURE, "Add string data");

  ex_gmp_fact(bignum_ret, bignum_cnt, bignum_for,
              TRUE, s1, ret_max_sz, cnt_max_sz);
  
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
