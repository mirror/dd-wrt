/* This is prints US words for a number.
   http://www.jimloy.com/math/billion.htm
 */

/* we only need output here, so turn off other IO functions */
#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_OPEN 1
#include "ex_utils.h"

#include <limits.h>
#include <gmp.h>

/* do we want to test that it works? */
#define EX_WORDS_USE_TST 1

#define VAL0_3()    "000"
#define VAL0_6()    VAL0_3()VAL0_3()
#define VAL0_9()    VAL0_3()VAL0_3()VAL0_3()
#define VAL0_10()   "0" VAL0_9()
#define VAL0_12()   VAL0_3()VAL0_3()VAL0_3()VAL0_3()
#define VAL0_15()   VAL0_3()VAL0_3()VAL0_3()VAL0_3()VAL0_3()
#define VAL0_30()   VAL0_15()VAL0_15()
#define VAL0_50()   VAL0_10()VAL0_10()VAL0_10()VAL0_10()VAL0_10()
#define VAL0_60()   VAL0_30()VAL0_30()
#define VAL0_100()  VAL0_50()VAL0_50()
#define VAL0_300()  VAL0_100()VAL0_100()VAL0_100()

#define WORDS_MAKE(num, out)                        \
    {num, " " out, sizeof(out), {{0, 0, NULL}}}     \

#define WORDS_ADD_WORD()                        \
    if (s1 && !vstr_add_ptr(s1, pos, scan->str, scan->len))     \
      return (0);                                               \
                                                                \
    pos += scan->len;                                           \
    ret += scan->len

/* can't be bothered doing the english version ...
 * we often just use the american version anyway. */
static size_t words_conv(Vstr_base *s1, size_t pos, const mpz_t num,
                         int cap, int del)
{
  size_t orig_pos = pos;
  struct Words_conv
  {
   const char *num;
   const char *str;
   const size_t len;
   mpz_t bignum;
  };
  static struct Words_conv conv_m[] =
    {
#if 0
     WORDS_MAKE("1" VAL0_300() VAL0_3(),           "centillion"),
     WORDS_MAKE("1" VAL0_60() VAL0_3(),            "vigintillion"),
     WORDS_MAKE("1" VAL0_60(),                     "novemdecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_15() VAL0_12(), "octodecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_15() VAL0_9(),  "septendecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_15() VAL0_6(),  "sexdecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_15() VAL0_3(),  "quindecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_15(),           "quattuordecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_12(),           "tredecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_9(),            "duodecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_6(),            "undecillion"),
     WORDS_MAKE("1" VAL0_30() VAL0_3(),            "decillion"),
     WORDS_MAKE("1" VAL0_30(),                     "nonillion"),
     WORDS_MAKE("1" VAL0_15() VAL0_12(),           "octillion"),
     WORDS_MAKE("1" VAL0_15() VAL0_9(),            "septillion"),
     WORDS_MAKE("1" VAL0_15() VAL0_6(),            "sextillion"),
#endif
     WORDS_MAKE("1" VAL0_15() VAL0_3(),            "quintillion"),
     WORDS_MAKE("1" VAL0_15(),                     "quadrillion"),
     WORDS_MAKE("1" VAL0_12(),                     "trillion"),
     WORDS_MAKE("1" VAL0_9(),                      "billion"),
     WORDS_MAKE("1" VAL0_6(),                      "million"),
     WORDS_MAKE("1" VAL0_3(),                      "thousand"),
     WORDS_MAKE("100",                             "hundred"),
    };
  static struct Words_conv conv_and =
     WORDS_MAKE("&",                               "and");
  static struct Words_conv conv_a[] =
    {
     WORDS_MAKE("90",                              "ninety"),
     WORDS_MAKE("80",                              "eighty"),
     WORDS_MAKE("70",                              "seventy"),
     WORDS_MAKE("60",                              "sixty"),
     WORDS_MAKE("50",                              "fifty"),
     WORDS_MAKE("40",                              "forty"),
     WORDS_MAKE("30",                              "thirty"),
     WORDS_MAKE("20",                              "twenty"),
     WORDS_MAKE("19",                              "nineteen"),
     WORDS_MAKE("18",                              "eighteen"),
     WORDS_MAKE("17",                              "seventeen"),
     WORDS_MAKE("16",                              "sixteen"),
     WORDS_MAKE("15",                              "fifteen"),
     WORDS_MAKE("14",                              "fourteen"),
     WORDS_MAKE("13",                              "thirteen"),
     WORDS_MAKE("12",                              "twelve"),
     WORDS_MAKE("11",                              "eleven"),
     WORDS_MAKE("10",                              "ten"),
     WORDS_MAKE("9",                               "nine"),
     WORDS_MAKE("8",                               "eight"),
     WORDS_MAKE("7",                               "seven"),
     WORDS_MAKE("6",                               "six"),
     WORDS_MAKE("5",                               "five"),
     WORDS_MAKE("4",                               "four"),
     WORDS_MAKE("3",                               "three"),
     WORDS_MAKE("2",                               "two"),
     WORDS_MAKE("1",                               "one"),
    };
  static struct Words_conv conv_zero =
     WORDS_MAKE("0",                               "zero");
  static struct Words_conv conv_minus =
     WORDS_MAKE("-",                               "minus");  
  static int done = FALSE;
  struct Words_conv *scan = conv_m;
  unsigned int alen = sizeof(conv_m) / sizeof(conv_m[0]);
  size_t ret = 0;
  mpz_t tmp;

  if (!done)
  {
    while (alen)
    {
      mpz_init_set_str(scan->bignum, scan->num, 10);
      
      --alen;
      ++scan;
    }
    scan = conv_a;
    alen = sizeof(conv_a) / sizeof(conv_a[0]);
    while (alen)
    {
      mpz_init_set_str(scan->bignum, scan->num, 10);
      
      --alen;
      ++scan;
    }
    
    scan = conv_m;
    alen = sizeof(conv_m) / sizeof(conv_m[0]);
    
    done = TRUE;
  }
  
  mpz_init_set(tmp, num);
  mpz_abs(tmp, tmp);

  if (mpz_cmp_ui(num, 0) == 0)
  {
    scan = &conv_zero;
    WORDS_ADD_WORD();
  }
  else
  {
    while (alen)
    {
      mpz_t quo;

      mpz_init(quo);
      
      while (mpz_cmp(tmp, scan->bignum) >= 0)
      {
        size_t front = 0;
        
        mpz_tdiv_qr(quo, tmp, tmp, scan->bignum);

        if (!(front = words_conv(s1, pos, quo, FALSE, FALSE)))
          return (0);
        ret += front;
        pos += front;

        WORDS_ADD_WORD();
      }
      
      --alen;
      ++scan;
    }

    if (ret && (mpz_cmp_ui(tmp, 0) != 0))
    {
      scan = &conv_and;
      WORDS_ADD_WORD();
    }

    scan = conv_a;
    alen = sizeof(conv_a) / sizeof(conv_a[0]);
    while (alen)
    {
      while (mpz_cmp(tmp, scan->bignum) >= 0)
      {
        mpz_mod(tmp, tmp, scan->bignum);
        WORDS_ADD_WORD();
      }
      
      --alen;
      ++scan;
    }
  }
  
  ASSERT(ret >= 2);
  
  if (mpz_sgn(num) == -1)
  {
    scan = &conv_minus;
    pos  = orig_pos;
    WORDS_ADD_WORD();
  }
  
  ASSERT(ret >= 2);

  ++orig_pos;
  if (del)
  { /* get rid of space */
    if (s1) vstr_del(s1, orig_pos, 1);
    --ret;
  }
  
  if (cap && s1 && !vstr_conv_uppercase(s1, orig_pos, 1)) /* capitalize */
    return (0);
  
  return (ret);
}

/* This is the custom formatter. */
static int ex__usr_words_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec,
                            const mpz_t val)
{
  int flags = VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM;
  size_t len = 0;
  
  len = words_conv(NULL, 0, val, TRUE, TRUE);
  
  /* Not handled atm. */
  spec->fmt_quote = 0;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len, flags))
    return (FALSE);
 
  if (!words_conv(base, pos, val, TRUE, TRUE))
    return (FALSE);
    
  if (!vstr_sc_fmt_cb_end(base, pos, spec, len))
    return (FALSE);
  
  return (TRUE);
}

/* Extra, do binary output... */
static int ex_usr_words_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_words_cb(base, pos, spec, mpz));
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL);
  mpz_t bignum;
  
  if (argc < 2)
    errx(EXIT_FAILURE, "No count specified");

  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  if (!vstr_sc_fmt_add_vstr(s1->conf, "{vstr:%p%zu%zu%u}") ||
      !vstr_fmt_add(s1->conf, "<words:%p>", ex_usr_words_cb,
                    VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END))
    errno = ENOMEM, err(EXIT_FAILURE, "Custom formatters");

#if EX_WORDS_USE_TST
  {
    struct Words_tst_conv
    {
     const char *i;
     const char *o;
    }
    tsts[] =
      {
       {"0",   "Zero"},
       {"1",   "One"},
       {"2",   "Two"},
       {"3",   "Three"},
       {"4",   "Four"},
       {"5",   "Five"},
       {"6",   "Six"},
       {"7",   "Seven"},
       {"8",   "Eight"},
       {"9",   "Nine"},
       {"10",  "Ten"},
       {"11",  "Eleven"},
       {"12",  "Twelve"},
       {"13",  "Thirteen"},
       {"14",  "Fourteen"},
       {"15",  "Fifteen"},
       {"16",  "Sixteen"},
       {"17",  "Seventeen"},
       {"18",  "Eighteen"},
       {"19",  "Nineteen"},
       {"20",  "Twenty"},
       {"21",  "Twenty one"},
       {"29",  "Twenty nine"},
       {"30",  "Thirty"},
       {"34",  "Thirty four"},
       {"40",  "Forty"},
       {"50",  "Fifty"},
       {"60",  "Sixty"},
       {"70",  "Seventy"},
       {"80",  "Eighty"},
       {"90",  "Ninety"},
       {"100", "One hundred"},
       {"190", "One hundred and ninety"},
       {"200", "Two hundred"},
       {"1000", "One thousand"},
       {"2000", "Two thousand"},
       {"3210", "Three thousand two hundred and ten"},
                                                                                
       {"9876543210", "Nine billion eight hundred and seventy six million five hundred and forty three thousand two hundred and ten"},
       {"9876543210" VAL0_9(), "Nine quintillion eight hundred and seventy six quadrillion five hundred and forty three trillion two hundred and ten billion"},
       {"-3210", "Minus three thousand two hundred and ten"},
      };
    unsigned int alen = sizeof(tsts) / sizeof(tsts[0]);
    struct Words_tst_conv *scan = tsts;
    

    while (alen)
    {
      mpz_init_set_str(bignum, scan->i, 0);

      vstr_del(s1, 1, s1->len);
      vstr_add_fmt(s1, s1->len, "$<words:%p>", (void *)bignum);
      
      if (!vstr_cmp_cstr_eq(s1, 1, s1->len, scan->o))
        errx(EXIT_FAILURE, "Tst failed(%s): %s",
             scan->o, vstr_export_cstr_ptr(s1, 1, s1->len));

      --alen;
      ++scan;
    }
  }
#endif
  
  mpz_init_set_str(bignum, argv[1], 0);

  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, s1->len, " Input: %s\n", argv[1]);
  vstr_add_fmt(s1, s1->len, " Words: $<words:%p>\n", (void *)bignum);
  
  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");
  
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
