/* vstr stuff and bug fixes by james antill ... LGPL and above MIT.
   timesstamp() function by Michael B Allen <mba2000@ioplex.com>
   gcc -g -Wall -W -O2 -o csv_vstr csv_vstr.c `pkg-config --cflags --libs vstr`
 */

/* configuration... */
#define VCSV_OPT_FLAGS VCSV_FLAG_LINE
#define USE_VDUMP 1
#define USE_DEBUG 0


#if !(USE_DEBUG)
# define NDEBUG 1 /* go backwards for traditional macro */
#endif

#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

/* option flags... */
#define VCSV_FLAG_NONE 0
#define VCSV_FLAG_LINE 1 /* only parse one line */


#define TRUE  1
#define FALSE 0


#define VCSV_ST_PRE           0
#define VCSV_ST_BEG           1
#define VCSV_ST_GET_BEG_DQUOT 2
#define VCSV_ST_GET_END_DQUOT 3
#define VCSV_ST_GET_NORM      4
#define VCSV_ST_SKIP_COMMA    5
#define VCSV_ST_SKIP_TRASH    6
#define VCSV_ST_SKIP_RET      7
#define VCSV_ST_INIT          8

#define VCSV__MEMCHR(x) memchr(iter->ptr, (x), iter->len)

#define VCSV__INC(x) do { size_t local_inc_tmp = (x); \
    assert(local_inc_tmp <= iter->len); \
    \
    iter->ptr += local_inc_tmp; \
    iter->len -= local_inc_tmp; \
    \
    len -= local_inc_tmp; \
 } while (FALSE)

static Vstr_base *out = NULL;
static Vstr_base *vcsv_data = NULL;

static inline void vcsv_end(size_t pos, size_t data_len,
                            size_t beg_len, size_t len,
                            Vstr_sects *rows)
{
  vstr_sects_add(rows, pos + (data_len - beg_len), beg_len - len);
}

static int
vcsv_row_parse(Vstr_base *s1, size_t pos, size_t len,
               Vstr_sects *rows, unsigned int flags)
{
  size_t data_len = len;
  size_t ret_len  = len;
  size_t beg_len  = 0;
  unsigned int state = VCSV_ST_INIT;
  const char *ptr = NULL;
  size_t tmp = 0;
  Vstr_iter iter[1];

  if (!len)
    return (0);

  if (!vstr_iter_fwd_beg(s1, pos, len, iter))
    abort();

  while (len)
  {
    if (!iter->len)
      if (!vstr_iter_fwd_nxt(iter)) abort();

    switch (state)
    {
      case VCSV_ST_SKIP_TRASH:
        if (!(ptr = VCSV__MEMCHR(',')))
        {
          VCSV__INC(iter->len);
          break;
        }
        tmp = ptr - iter->ptr;

        VCSV__INC(tmp);
        state = VCSV_ST_SKIP_COMMA;
        break;

      case VCSV_ST_SKIP_COMMA:
        assert(*iter->ptr == ',');

        VCSV__INC(1);
        state = VCSV_ST_PRE;
        beg_len = len;
        break;

      case VCSV_ST_PRE:
        if ((*iter->ptr == '\n') || (*iter->ptr == '\r'))
        {
          vcsv_end(pos, data_len, beg_len, len, rows);
          state = VCSV_ST_SKIP_RET;
        }
        else
          state = VCSV_ST_BEG;
        break;

      case VCSV_ST_SKIP_RET:
        if (flags & VCSV_FLAG_LINE)
          return (ret_len - len);
      case VCSV_ST_INIT:

        while ((*iter->ptr == '\n') || (*iter->ptr == '\r'))
        { /* skip blanks to start... */
          VCSV__INC(1);

          if (!iter->len && !vstr_iter_fwd_nxt(iter))
            return (ret_len);
        }
        beg_len = len;
        /* FALL THROUGH */

      case VCSV_ST_BEG:
        if (*iter->ptr == '"')
        {
          state = VCSV_ST_GET_BEG_DQUOT;
          --beg_len;
        }
        else if (*iter->ptr == ',')
        {
          vcsv_end(pos, data_len, beg_len, len, rows);
          state = VCSV_ST_SKIP_COMMA;
          continue;
        }
        else
        {
          beg_len = len; /* mark start */
          state = VCSV_ST_GET_NORM;
        }
        VCSV__INC(1);
        break;

      case VCSV_ST_GET_BEG_DQUOT:
        if (!(ptr = VCSV__MEMCHR('"')))
        {
          VCSV__INC(iter->len);
          break;
        }
        tmp = ptr - iter->ptr;
        VCSV__INC(tmp + 1);
        state = VCSV_ST_GET_END_DQUOT;
        if (!len)
        {
          vcsv_end(pos, data_len, beg_len, 1, rows);
          return (ret_len);
        }
        break;

      case VCSV_ST_GET_END_DQUOT:
      {
        unsigned int found_ret = FALSE;

        ++len; /* go back to the '"' */
        switch (*iter->ptr)
        {
          case '\r':
          case '\n':
            found_ret = TRUE;
          case ',':
            vcsv_end(pos, data_len, beg_len, len, rows);
            beg_len = 0;
            if (found_ret)
              state = VCSV_ST_SKIP_RET;
            else
              state = VCSV_ST_SKIP_COMMA;
            break;

          default:
            vcsv_end(pos, data_len, beg_len, len, rows);
            beg_len = 0;
            state = VCSV_ST_SKIP_TRASH;
            break;

          case '"':
          {
            size_t tpos = pos + (data_len - beg_len) + (beg_len - len);

            vstr_del(s1, tpos, 1);

            --data_len; /* update lengths and re-init iter */
            --beg_len;
            len -= 2;   /* for above */

            if (!vstr_iter_fwd_beg(s1, tpos + 1, len, iter)) abort();
            state = VCSV_ST_GET_BEG_DQUOT;
            continue;
          }
          break;
        }
        --len; /* reverse above */
      }
      break;

      case VCSV_ST_GET_NORM:
        tmp = 0;
        while (tmp < iter->len)
        {
          if ((iter->ptr[tmp] == ',') ||
              (iter->ptr[tmp] == '\r') ||
              (iter->ptr[tmp] == '\n'))
            break;

          ++tmp;
        }
        VCSV__INC(tmp);
        if (!iter->len)
          break;

        vcsv_end(pos, data_len, beg_len, len, rows);
        if (iter->ptr[0] == ',')
          state = VCSV_ST_SKIP_COMMA;
        else
          state = VCSV_ST_SKIP_RET;
        break;

      default:
        abort();
    }
  }

  if ((state != VCSV_ST_SKIP_RET) && (state != VCSV_ST_SKIP_TRASH))
    vcsv_end(pos, data_len, beg_len, len, rows);

  return (ret_len);
}

#if USE_VDUMP
# define VDUMP(ret, rows) vdump(ret, rows)
static void vdump(unsigned int ret, Vstr_sects *rows)
{
  unsigned int scan = 0;

  vstr_add_fmt(out, out->len,
               "${rep_chr:%c%zu}\n" "%d\n" "${rep_chr:%c%zu}\n",
               '=', 79, ret, '=', 79);

  while (scan < rows->num)
  {
    size_t pos = 1;
    size_t len = 0;

    ++scan;

    len = VSTR_SECTS_NUM(rows, scan)->len;
    if (len)
      pos = VSTR_SECTS_NUM(rows, scan)->pos;

    vstr_add_fmt(out, out->len, "|${vstr:%p%zu%zu%u}|\n",
                 vcsv_data, pos, len, 0);
  }

  vstr_add_fmt(out, out->len, "${rep_chr:%c%zu}\n", '-', 79);

  while (out->len)
    if (!vstr_sc_write_fd(out, 1, out->len, 1, NULL))
      abort();
}
#else
# define VDUMP(ret, rows) /* nothing */
#endif

#if defined(_WIN32)
#include <Windows.h>

#define MILLISECONDS_BETWEEN_1970_AND_1601 11644473600000Ui64
typedef unsigned __int64 uint64_t;

uint64_t
timestamp(void)
{
        FILETIME ftime;
        uint64_t ret;

        GetSystemTimeAsFileTime(&ftime);

        ret = ftime.dwHighDateTime;
        ret <<= 32Ui64;
        ret |= ftime.dwLowDateTime;
        ret = ret / 10000Ui64 - MILLISECONDS_BETWEEN_1970_AND_1601;

        return ret;
}

#else
#include <sys/time.h>
#include <inttypes.h>


static inline uint64_t
timestamp(void)
{
        struct timeval tval;

        gettimeofday(&tval, NULL);

        return tval.tv_sec * 1000LL + tval.tv_usec / 1000;
}

#endif

int main(int argc, char *argv[])
{
  VSTR_SECTS_DECL(vrows, 256);

    int ret = 0;

  if (argc != 2) abort();

  if (!vstr_init())
    abort();

  VSTR_SECTS_DECL_INIT(vrows);

  vcsv_data = vstr_make_base(NULL);

  out       = vstr_make_base(NULL);
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');
  vstr_sc_fmt_add_all(NULL);

  do
  {
    size_t pos = 1;
    size_t len = 0;
    unsigned int err = 0;
    uint64_t t0 = timestamp();

    if (!vstr_sc_mmap_file(vcsv_data, 0, argv[1], 0, 0, &err))
      break;

    len = vcsv_data->len;
    if (err)
      abort();

    while ((ret = vcsv_row_parse(vcsv_data, pos, len, vrows, VCSV_OPT_FLAGS)))
    {
      VDUMP(ret, vrows);

      pos += ret;
      len -= ret;
      vrows->num = 0;
    }

    vstr_add_fmt(out, out->len, "%'llu milliseconds\n", (timestamp() - t0));

    while (out->len)
      if (!vstr_sc_write_fd(out, 1, out->len, 2, NULL))
        abort();
  } while (FALSE);

  exit (EXIT_SUCCESS);
}
