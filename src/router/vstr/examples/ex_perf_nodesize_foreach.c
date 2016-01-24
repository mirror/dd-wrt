/* This file tests how efficient differrent lengths of _BUF nodes are and also
 * compares them to one big pile
 * -- vstr is kind of built on the idea that this isn't a problem for
 *    reasonable sizes */

#define VSTR_COMPILE_INCLUDE 1

#include <vstr.h>

#if 0
# define TST_SZ_BEG (64 - (16 + 8))
# define TST_SZ_INC ((sz == (64 - (16 + 8)))   ? (16 + 8) : \
                     (sz == 64)                ? (64 - (16 + 8)) : \
                     (sz == (128 - (16 + 8)))  ? (16 + 8) : \
                     (sz == 128)               ? (128 - (16 + 8)) : \
                     (sz == (256 - (16 + 8)))  ? (16 + 8) : \
                     (sz == 256)               ? (256 - (16 + 8)) : \
                     (sz == (512 - (16 + 8)))  ? (16 + 8) : \
                     (sz == 512)               ? (512 - (16 + 8)) : \
                     (sz == (1024 - (16 + 8))) ? (16 + 8) : \
                     (sz == 1024)              ? (3072 - (16 + 8)) : \
                     (sz == (4096 - (16 + 8))) ? (16 + 8) : 1)
# define TST_SZ_END (4096)
#else
# define TST_SZ_BEG (512 - (16 + 8))
# define TST_SZ_INC ((sz == (512 - (16 + 8)))  ? (16 + 8) : \
                     (sz == 512)               ? (512 + 3072 - (16 + 8)) : \
                     (sz == (4096 - (16 + 8))) ? (16 + 8) : 1)
# define TST_SZ_END (4096)
#endif

/* #define TST_SZ_END (4096 - (sizeof(Vstr_node) + 16)) */

/*
  #define TST_NUM_BEG (64)
  #define TST_NUM_END (1024)
*/
#define TST_NUM_BEG (4096)
#define TST_NUM_INC (1)
#define TST_NUM_END (4096)

#define TST_LEN_BEG (sz >= 128 ? 128 : 1)
#define TST_LEN_INC (1)
#define TST_LEN_END (1024 * 100)

#define TST_MALLOC_EXTRA_BEG (0)
#define TST_MALLOC_EXTRA_INC (1024)
#define TST_MALLOC_EXTRA_END (1024 * 2)

#define TST_COUNT_BEG (1)
#define TST_COUNT_INC (1)
#define TST_COUNT_END (2)

#define TST_USE_MEMCPY 1

#include "ex_perf.h"

#include <assert.h>
#include <unistd.h>

#include <glib.h>


 /* this is M_TRIM_THRESH */
#define TST_ALLOC_TRIM() do { \
  struct mallinfo mal_info = mallinfo(); \
  trig_trim = malloc((128 * 1024) + mal_info.fordblks); \
  if (!trig_trim) goto failed; \
} while (0)

#define TST_CLEANUP_TRIM(x) do { \
  unsigned int clean_extra = num; \
  \
  if (extra) \
    while (clean_extra--) \
      free(mal_overhead[clean_extra]); \
  \
  if (x) free(trig_trim); \
} while (0)


int main(void)
{
  Vstr_conf *conf = NULL;
  Vstr_base *out = NULL;
  Vstr_base *s1 = NULL;
  char *s2 = NULL;
  GString *s3 = NULL;
  char *s4 = NULL;
  unsigned int ern = 0;
  char *mal_overhead[TST_NUM_END];
  char *s3_data = NULL;

  mallopt(M_MMAP_MAX, 0); /* mmap() can change the results */

  if (!vstr_init())
    exit(EXIT_FAILURE);

  conf = vstr_make_conf();
  if (!conf)
    exit(EXIT_FAILURE);

  out = vstr_make_base(NULL); /* used as stdio/stdout */
  if (!out)
    exit (EXIT_FAILURE);

  s3_data = malloc(TST_LEN_END);
  if (!s3_data)
    goto failed;
  memset(s3_data, 'x', TST_LEN_END);

  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF, 128, 128);

  if (1)
  {
    unsigned int sz = TST_SZ_BEG;
    while (sz <= TST_SZ_END)
    {
    unsigned int len = TST_LEN_BEG;

    vstr_free_base(s1);
    vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_NUM_BUF_SZ, sz);
    s1 = vstr_make_base(conf);
    if (!s1)
      goto failed;

    while (len <= TST_LEN_END)
    {
    unsigned int num = TST_NUM_BEG;
    while (num <= TST_NUM_END)
    {
    unsigned int extra = TST_MALLOC_EXTRA_BEG;
    while (extra <= TST_MALLOC_EXTRA_END)
    {
    unsigned int count = TST_COUNT_BEG;
    while (count <= TST_COUNT_END)
    {
      void *trig_trim = NULL;

      TST_ALLOC_TRIM();

      TST_BEG(1, num);
      if (extra)
        mal_overhead[tst_count] = malloc(extra);
      buf_out[0] = 'x';
      if (TST_USE_MEMCPY)
        vstr_add_buf(s1, s1->len, s3_data, len);
      else
        vstr_add_rep_chr(s1, s1->len, buf_out[0], len);
      TST_CALC_END("V");

      vstr_del(s1, 1, s1->len);
      TST_CLEANUP_TRIM(0);

      TST_BEG(1, num);
      if (extra)
        mal_overhead[tst_count] = malloc(extra);
      buf_out[0] = 'x';
      if (TST_USE_MEMCPY)
        vstr_add_buf(s1, s1->len, s3_data, len);
      else
        vstr_add_rep_chr(s1, s1->len, buf_out[0], len);
      TST_CALC_END("A");

      vstr_del(s1, 1, s1->len);
      vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF, 0, 0);
      TST_CLEANUP_TRIM(1);
      TST_ALLOC_TRIM();

      s2 = malloc(sz); /* single object */
      if (!s2)
        goto failed;

      TST_BEG(1, num);
      unsigned int off     = (len * tst_count);
      unsigned int new_len = off + len;
      if (extra)
        mal_overhead[tst_count] = malloc(extra);
      buf_out[0] = 'x';
      s2 = realloc(s2, new_len);
      if (!s2)
        goto failed;
      if (TST_USE_MEMCPY)
        memcpy(s2 + off, s3_data,    len);
      else
        memset(s2 + off, buf_out[0], len);
      TST_CALC_END("S");

      free(s2);
      s2 = NULL;
      TST_CLEANUP_TRIM(1);
      TST_ALLOC_TRIM();

      if (TST_USE_MEMCPY)
      {
        s3 = g_string_sized_new(sz);
        TST_BEG(1, num);
        if (extra)
          mal_overhead[tst_count] = malloc(extra);
        buf_out[0] = 'x';
        g_string_append_len(s3, s3_data, len);
        TST_CALC_END("G");

        g_string_free(s3, TRUE);
        s3 = NULL;
        TST_CLEANUP_TRIM(1);
        TST_ALLOC_TRIM();

        TST_BEG(1, num);
        if (extra)
          mal_overhead[tst_count] = malloc(extra);
        buf_out[0] = 'x';
        vstr_add_ptr(s1, s1->len, s3_data, len);
        TST_CALC_END("P");

        vstr_del(s1, 1, s1->len);
        vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR, 0, 0);
        TST_CLEANUP_TRIM(1);
        TST_ALLOC_TRIM();
      }

      s4 = malloc(len); /* single object */
      if (!s4)
        goto failed;

      TST_BEG(1, num);
      if (extra)
        mal_overhead[tst_count] = malloc(extra);
      buf_out[0] = 'x';
      if (TST_USE_MEMCPY)
        memcpy(s4, s3_data,    len);
      else
        memset(s4, buf_out[0], len);
      TST_CALC_END("B");

      free(s4);
      s4 = NULL;
      TST_CLEANUP_TRIM(1);

      if (out->len)
        vstr_sc_write_fd(out, 1, out->len, 1 /* stdout */, &ern);
      if (ern)
        goto failed;

      count += TST_COUNT_INC;
    }
      extra += TST_MALLOC_EXTRA_INC;
    }
      num   += TST_NUM_INC;
    }
      len   += TST_LEN_INC;
    }
      sz    += TST_SZ_INC;
    }
  }

  if (s1->conf->malloc_bad || out->conf->malloc_bad)
    goto failed;

  while (out->len && !ern)
    vstr_sc_write_fd(out, 1, out->len, STDOUT_FILENO, &ern);

  vstr_free_conf(conf);
  vstr_free_base(out);
  vstr_free_base(s1);
  free(s2);

  vstr_exit();

  exit (EXIT_SUCCESS);

 failed:
  ern = 0;
  while (out->len && !ern)
    vstr_sc_write_fd(out, 1, out->len, STDERR_FILENO, &ern);

  vstr_free_base(out);
  vstr_free_base(s1);
  free(s2);

  vstr_exit();

  exit (EXIT_FAILURE);
}

