/* This file tests how efficient differrent lengths of _BUF nodes are and also
 * compares them to one big pile
 * -- vstr is kind of built on the idea that this isn't a problem for
 *    reasonable sizes */

#define VSTR_COMPILE_INCLUDE 1

#include <vstr.h>

#ifndef TST_SZ /* chunk size for custom Vstr */
# define TST_SZ (4096 - (sizeof(Vstr_node) + 8)) /* one page each */
#endif

#ifndef TST_NUM /* number of times to perform each test */
# define TST_NUM (1024 * 128)
#endif

#ifndef TST_LEN /* length of data for each test */
# define TST_LEN (512)
#endif

#ifndef TST_MALLOC_EXTRA /* malloc extra bytes before each loop */
# define TST_MALLOC_EXTRA (0)
#endif

#ifndef TST_FREE /* free the data from previous runs between each test */
# define TST_FREE (1)
#endif

#ifndef TST_REUSE
# define TST_REUSE (0)
#endif

#include "ex_perf.h"

#include <assert.h>

int main(void)
{
  Vstr_conf *conf = NULL;
  Vstr_base *out = NULL;
  Vstr_base *s1 = NULL;
  char *s2 = NULL;
  Vstr_base *s3 = NULL;
  unsigned int err = 0;
  char *mal_overhead[TST_NUM];

  if (!vstr_init())
    exit(EXIT_FAILURE);

  conf = vstr_make_conf();
  if (!conf)
    exit(EXIT_FAILURE);

  /* have a custom config. for output and s1 ... */
  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_NUM_BUF_SZ, TST_SZ);

  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP, "_");
  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\3");

  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_all(conf);

  out = vstr_make_base(conf); /* used as stdio/stdout */

  s1 = vstr_make_base(conf); /* configured size */
  s2 = malloc(TST_LEN); /* single object */
  s3 = vstr_make_base(NULL); /* default size */
  vstr_free_conf(conf);
  if (!out || !s1 || !s3)
    exit (EXIT_FAILURE);

  if (!s2)
    exit (EXIT_FAILURE);

  /* print table to say what config we are using ... */
  vstr_add_fmt(out, out->len,
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}+\n",
               '=', 17, '=', 18, '=', 18, '=', 18);
  vstr_add_fmt(out, out->len, "|%16s | %16s | %16s | %16s |\n",
               "name", "len", "chunks", "chunk size");

  vstr_add_fmt(out, out->len,
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}+\n",
               '-', 17, '-', 18, '-', 18, '-', 18);
  {
    unsigned int sz = 0;

    vstr_cntl_conf(s1->conf, VSTR_CNTL_CONF_GET_NUM_BUF_SZ, &sz);
    vstr_add_fmt(out, out->len,
                 "|%16s | $16{BKMG.u:%u} | %'16zu | $16{BKMG.u:%u} |\n",
                 "s1:  conf iovs", (unsigned int) (TST_LEN * TST_NUM),
                 ((TST_LEN * TST_NUM) / sz) + 1, sz);

    vstr_add_fmt(out, out->len,
                 "|%16s | $16{BKMG.u:%u} | %'16zu | $16{BKMG.u:%u} |\n",
                 "s2:       mmap", (unsigned int) (TST_LEN * TST_NUM),
                 TST_NUM, TST_LEN);

    vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_GET_NUM_BUF_SZ, &sz);
    vstr_add_fmt(out, out->len,
                 "|%16s | $16{BKMG.u:%u} | %'16zu | $16{BKMG.u:%u} |\n",
                 "s3:   def iovs", (unsigned int) (TST_LEN * TST_NUM),
                 ((TST_LEN * TST_NUM) / sz) + 1, sz);
  }
  vstr_add_fmt(out, out->len,
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}"
               "+${rep_chr:%c%zu}+\n",
               '-', 17, '-', 18, '-', 18, '-', 18);

  vstr_add_fmt(out, out->len,
               " Doing tests %'u times with extra malloc of %u:\n",
               (unsigned int)TST_NUM, TST_MALLOC_EXTRA);

  err = 0;
  while (out->len && !err)
    vstr_sc_write_fd(out, 1, out->len, 1 /* stdout */, &err);

  TST_HDR_BEG();

  TST_BEG(1, 1);
  mal_overhead[tst_count] = malloc(64 - 8);
  buf_out[0] = 'x';
  TST_END("single malloc ");

  for (err = 0; err < 1; ++err)
    free(mal_overhead[err]);

  TST_BEG(1, TST_NUM);
  if (TST_MALLOC_EXTRA)
    mal_overhead[tst_count] = malloc(TST_MALLOC_EXTRA);
  buf_out[0] = 'x';
  if (TST_REUSE)
    vstr_del(s1, 1, s1->len);
  vstr_add_rep_chr(s1, s1->len, buf_out[0], TST_LEN);
  TST_END("s1:  conf iovs");

  if (TST_FREE)
  {
    vstr_del(s1, 1, s1->len);
    if (TST_MALLOC_EXTRA)
      for (err = 0; err < TST_NUM; ++err)
        free(mal_overhead[err]);
  }

  TST_BEG(1, TST_NUM);
  unsigned int off     = (TST_LEN * tst_count);
  unsigned int new_len = off + TST_LEN;

  if (TST_MALLOC_EXTRA)
    mal_overhead[tst_count] = malloc(TST_MALLOC_EXTRA);
  buf_out[0] = 'x';
  if (TST_REUSE)
    off = 0;
  else
    s2 = realloc(s2, new_len);
  if (!s2)
    goto failed;
  memset(s2 + off, buf_out[0], TST_LEN);
  TST_END("s2:     malloc");

  if (TST_FREE)
  {
    free(s2); s2 = NULL;
    if (TST_MALLOC_EXTRA)
      for (err = 0; err < TST_NUM; ++err)
        free(mal_overhead[err]);
  }

  TST_BEG(1, TST_NUM);
  if (TST_MALLOC_EXTRA)
    mal_overhead[tst_count] = malloc(TST_MALLOC_EXTRA);
  buf_out[0] = 'x';
  if (TST_REUSE)
    vstr_del(s3, 1, s3->len);
  vstr_add_rep_chr(s3, s3->len, buf_out[0], TST_LEN);
  TST_END("s3:   def iovs");

  if (TST_FREE)
  {
    vstr_del(s3, 1, s3->len);
    if (TST_MALLOC_EXTRA)
      for (err = 0; err < TST_NUM; ++err)
        free(mal_overhead[err]);
  }

  TST_HDR_END();

  if (s1->conf->malloc_bad || s3->conf->malloc_bad)
    goto failed;

  err = 0;
  while (out->len && !err)
    vstr_sc_write_fd(out, 1, out->len, 1 /* stdout */, &err);

  vstr_free_base(out);
  vstr_free_base(s1);
  free(s2);
  vstr_free_base(s3);

  vstr_exit();

  exit (EXIT_SUCCESS);

 failed:
  err = 0;
  while (out->len && !err)
    vstr_sc_write_fd(out, 1, out->len, 2 /* stderr */, &err);

  vstr_free_base(out);
  vstr_free_base(s1);
  free(s2);
  vstr_free_base(s3);

  vstr_exit();

  exit (EXIT_FAILURE);
}

