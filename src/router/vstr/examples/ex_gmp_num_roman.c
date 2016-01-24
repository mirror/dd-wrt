/* This is prints roman numerals.
 * Inspired by http://www.differentpla.net/node/view/58
 * Requires GMP
 */

/* we only need output here, so turn off other IO functions */
#define EX_UTILS_NO_USE_INPUT 1
#define EX_UTILS_NO_USE_OPEN 1
#include "ex_utils.h"

#include <limits.h>
#include <gmp.h>

/* do we want to test that it works? */
#define EX_ROMAN_USE_TST 0

#define ROMAN_MAKE(num, out)                        \
    {num, out, sizeof(out) - 1}                     \
    

static size_t roman_conv(Vstr_base *s1, size_t pos, const mpz_t num)
{
  struct Roman_conv
  {
   const unsigned int num;
   const char *str;
   const size_t len;
  };
  
  static struct Roman_conv conv[] =
    {
     ROMAN_MAKE(1000, "M"),
     ROMAN_MAKE(900,  "CM"),
     ROMAN_MAKE(500,  "D"),
     ROMAN_MAKE(400,  "CD"),
     ROMAN_MAKE(100,  "C"),
     ROMAN_MAKE(90,   "XC"),
     ROMAN_MAKE(50,   "L"),
     ROMAN_MAKE(40,   "XL"),
     ROMAN_MAKE(10,   "X"),
     ROMAN_MAKE(9,    "IX"),
     ROMAN_MAKE(5,    "V"),
     ROMAN_MAKE(4,    "IV"),
     ROMAN_MAKE(1,    "I"),
    };
  const struct Roman_conv *scan = conv;
  unsigned int alen = sizeof(conv) / sizeof(conv[0]);
  size_t ret = 0;
  mpz_t tmp;
  
  mpz_init_set(tmp, num);
  mpz_abs(tmp, tmp);
  
  while (alen)
  {
    while (mpz_cmp_ui(tmp, scan->num) >= 0)
    {
      mpz_sub_ui(tmp, tmp, scan->num);

      if (s1 && !vstr_add_buf(s1, pos, scan->str, scan->len))
        return (0);
      
      pos += scan->len;
      ret += scan->len;
    }

    --alen;
    ++scan;
  }

  
  return (ret);
}

/* This is the custom formatter. */
static int ex__usr_roman_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec,
                            const mpz_t val)
{
  int flags = VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM; /* it's a number */
  size_t len = 0;

  if (mpz_sgn(val) == -1)
    flags |= VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NEG;
  
  len = roman_conv(NULL, 0, val);
  
  /* Not handled atm. */
  spec->fmt_quote = 0;
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len, flags))
    return (FALSE);
 
  if (!roman_conv(base, pos, val))
    return (FALSE);
    
  if (!vstr_sc_fmt_cb_end(base, pos, spec, len))
    return (FALSE);
  
  return (TRUE);
}

/* Extra, do binary output... */
static int ex_usr_roman_cb(Vstr_base *base, size_t pos, Vstr_fmt_spec *spec)
{
  void *mpz = VSTR_FMT_CB_ARG_PTR(spec, 0);

  return (ex__usr_roman_cb(base, pos, spec, mpz));
}

int main(int argc, char *argv[])
{
  Vstr_base *s1 = ex_init(NULL);
  mpz_t bignum;
  
  if (argc < 2)
    errx(EXIT_FAILURE, "No count specified");

  vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  if (!vstr_sc_fmt_add_vstr(s1->conf, "{vstr:%p%zu%zu%u}") ||
      !vstr_fmt_add(s1->conf, "<roman:%p>", ex_usr_roman_cb,
                    VSTR_TYPE_FMT_PTR_VOID, VSTR_TYPE_FMT_END))
    errno = ENOMEM, err(EXIT_FAILURE, "Custom formatters");

#if EX_ROMAN_USE_TST
  {
    struct Roman_tst_conv
    {
     const char *i;
     const char *o;
    }
    tsts[] =
      {
       {"0",   ""},
       {"1",   "I"},
       {"2",   "II"},
       {"3",   "III"},
       {"4",   "IV"},
       {"5",   "V"},
       {"6",   "VI"},
       {"7",   "VII"},
       {"8",   "VIII"},
       {"9",   "IX"},
       {"10",  "X"},
       {"11",  "XI"},
       {"12",  "XII"},
       {"13",  "XIII"},
       {"14",  "XIV"},
       {"15",  "XV"},
       {"16",  "XVI"},
       {"17",  "XVII"},
       {"18",  "XVIII"},
       {"19",  "XIX"},
       {"20",  "XX"},
       {"21",  "XXI"},
       {"29",  "XXIX"},
       {"30",  "XXX"},
       {"34",  "XXXIV"},
       {"40",  "XL"},
       {"50",  "L"},
       {"60",  "LX"},
       {"70",  "LXX"},
       {"80",  "LXXX"},
       {"90",  "XC"},
       {"100", "C"},
       {"190", "CXC"},
       {"200", "CC"},
       {"300", "CCC"},
       {"400", "CD"},
       {"500", "D"},
       {"600", "DC"},
       {"700", "DCC"},
       {"800", "DCCC"},
       {"900", "CM"},
       {"1000", "M"},
                                                                                
       {"1900", "MCM"},
       {"1975", "MCMLXXV"},
       {"1989", "MCMLXXXIX"},
       {"1999", "MCMXCIX"},
       {"2000", "MM"},
       {"2001", "MMI"},
#define M10() "MMMMMMMMMM"
#define M100()    M10() M10() M10() M10() M10() M10() M10() M10() M10() M10()
#define M1_000()  M100()M100()M100()M100()M100()M100()M100()M100()M100()M100()
#define M10_000() M1_000()M1_000()M1_000()M1_000()M1_000() \
                  M1_000()M1_000()M1_000()M1_000()M1_000()
#define M100_000() M10_000()M10_000()M10_000()M10_000()M10_000() \
                   M10_000()M10_000()M10_000()M10_000()M10_000()
#define M1_000_000() M100_000()M100_000()M100_000()M100_000()M100_000() \
                     M100_000()M100_000()M100_000()M100_000()M100_000()
#define M10_000_000() M1_000_000()M1_000_000()M1_000_000()M1_000_000()M1_000_000() \
                      M1_000_000()M1_000_000()M1_000_000()M1_000_000()M1_000_000()
       {"10004", M10() "IV"},
       {"12345", M10() "MMCCCXLV"},
       {"10000000053", M10_000_000() "LIII"},
      };
    unsigned int alen = sizeof(tsts) / sizeof(tsts[0]);
    struct Roman_tst_conv *scan = tsts;
    

    while (alen)
    {
      mpz_init_set_str(bignum, scan->i, 0);

      vstr_del(s1, 1, s1->len);
      vstr_add_fmt(s1, s1->len, "$<roman:%p>", (void *)bignum);
      
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
  vstr_add_fmt(s1, s1->len, " Roman: $<roman:%p>\n", (void *)bignum);
  
  if (s1->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "Add string data");
  
  io_put_all(s1, STDOUT_FILENO);

  exit (ex_exit(s1, NULL));
}
