/* -*- mode: c -*-
 *
 * Alternatve version of the strcat test using the vstr string library ...
 *    http://www.and.org/vstr/
 * gcc -Wall -W -O2 -o tst tst.c `pkg-config --cflags --libs vstr`
 *
 * http://www.bagley.org/~doug/shootout/
 */
/* #define VSTR_COMPILE_INLINE 0 */
#include <vstr.h>

#include <errno.h>
#include <err.h>
#include <assert.h>

#include "ex_perf.h"

#define HAVE_IOV 0
#define FMT_ALL 0

#define MCPY_TYPE MCPY_GCC

static const char *mcpy_type_map[3] = {
#define MCPY_LIBC 0
 "MCPY_LIBC",
#define MCPY_GCC  1
 "MCPY_GCC",
#define MCPY_VSTR 2
 "MCPY_VSTR",
};

#if   MCPY_TYPE == MCPY_LIBC
# define MCPY(x, y, z) memcpy(x, y, z)
#elif MCPY_TYPE == MCPY_GCC
# define MCPY(x, y, z) __builtin_memcpy(x, y, z)
#elif MCPY_TYPE == MCPY_VSTR
# define MCPY(x, y, z) vstr_wrap_memcpy(x, y, z)
#else
# error "Not a valid MCPY_TYPE"
#endif

#define BUF_SZ (4 * 1024) /* size of node */

#define STUFF "hello\n" /* size of data to append each time -- from shootout */

#define YES_NO(x) ((x) ? "yes" : "no")

static void hand_inline(Vstr_base *tst, unsigned int num)
{
  unsigned int scan = 0;
  
  while (scan < num)
  {
    size_t len = tst->end ? (BUF_SZ - tst->end->len) : 0;
    
    if (len >= strlen(STUFF))
    { /* hand inline of vstr_add_cstr_buf()...
       *   massive hack -- DO NOT rely on this working this is just so we can
       *   mesure the best possible inline perf. */
      unsigned int orig = scan;
      char *buf = ((Vstr_node_buf *)tst->end)->buf;
      
      buf += tst->end->len;
      while (len >= strlen(STUFF))
      {
        MCPY(buf, STUFF, strlen(STUFF));
        buf += strlen(STUFF);
        len -= strlen(STUFF);

        if (++scan == num)
          break;
      }

      len = (scan - orig) * strlen(STUFF);
      tst->len      += len;
      tst->end->len += len;
      if (tst->iovec_upto_date)
      {
        unsigned int num_off = tst->num + VSTR__CACHE(tst)->vec->off - 1;
        VSTR__CACHE(tst)->vec->v[num_off].iov_len += len;
      }
    }
    
    vstr_add_cstr_buf(tst, tst->len, STUFF);
    ++scan;
  }
}

static void hand_iov(Vstr_base *tst, unsigned int num)
{
  unsigned int scan = 0;
  
  while (scan < num)
  {
    struct iovec *iov = NULL;
    unsigned int iov_num = 0;
    size_t len = 0;

    if (!vstr_add_iovec_buf_beg(tst, tst->len, 1, 2, &iov, &iov_num))
      errno = ENOMEM, err(EXIT_FAILURE, "iovec_buf_beg");

    assert(iov_num);
    
    len = iov[0].iov_len;
    
    if (len < strlen(STUFF))
      vstr_add_cstr_buf(tst, tst->len, STUFF);
    else
    { /* hand inline using iovectors */
      unsigned int orig = scan;
      char *buf = iov[0].iov_base;
      
      while (len >= strlen(STUFF))
      {
        MCPY(buf, STUFF, strlen(STUFF));
        buf += strlen(STUFF);
        len -= strlen(STUFF);

        if (++scan >= num)
          break;
      }
      assert(scan <= num);
      
      len = (scan - orig) * strlen(STUFF);
      vstr_add_iovec_buf_end(tst, tst->len, len);
    }

    ++scan;
  }
}

static void del(Vstr_base *tst)
{
  vstr_del(tst, 1, tst->len);
}
static void del_alloc(Vstr_base *tst)
{
  unsigned int num = tst->num;
  vstr_del(tst, 1, tst->len);
  vstr_free_spare_nodes(tst->conf, VSTR_TYPE_NODE_BUF, num);
}

int main(int argc, char *argv[])
{
  unsigned int num = ((argc == 2) ? atoi(argv[1]) : 1);
  Vstr_base *tst = NULL;
  Vstr_base *out = NULL;
                                                                                
  if (!vstr_init())
    errno = ENOMEM, err(EXIT_FAILURE, "vstr_init");
                                                                                
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_NUM_BUF_SZ, BUF_SZ);
  
  if (!(tst = vstr_make_base(NULL)))
    errno = ENOMEM, err(EXIT_FAILURE, "vstr_make_base");
  if (!(out = vstr_make_base(NULL)))
    errno = ENOMEM, err(EXIT_FAILURE, "vstr_make_base");

  vstr_cntl_conf(out->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');

  if (FMT_ALL)
    vstr_sc_fmt_add_all(out->conf);
  else
  {
    vstr_sc_fmt_add_buf(out->conf,     "{buf:%s%zu}");
    vstr_sc_fmt_add_rep_chr(out->conf, "{rep_chr:%c%zu}"); /* for TST_HDR */
  }
  
  vstr_cntl_conf(out->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP, "_");
  vstr_cntl_conf(out->conf, VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP, "\3");

  if (HAVE_IOV)
  {
    vstr_add_rep_chr(tst, 0, '-', strlen(STUFF) * num);
    vstr_export_iovec_ptr_all(tst, NULL, NULL);
  }
  vstr_del(tst, 1, tst->len);
  vstr_free_spare_nodes(tst->conf, VSTR_TYPE_NODE_BUF, 1000 * 1000 * 1000);
  
  TST_HDR_BEG();

  if (0) {
  del_alloc(tst); TST_BEG(1, 1);
  hand_inline(tst, num);
  TST_END("hand inline (alloc)");
  del(tst);       TST_BEG(1, 1);
  hand_inline(tst, num);
  TST_END("hand inline"); 
  
  del_alloc(tst); TST_BEG(1, 1);
  hand_iov(tst, num);
  TST_END("iov inline (alloc)");
  del(tst);       TST_BEG(1, 1);
  hand_iov(tst, num);
  TST_END("iov inline"); 
  
  del_alloc(tst); TST_BEG(1, num);
  vstr_add_cstr_buf(tst, tst->len, STUFF);
  TST_END("add_cstr_buf (alloc)");
  del(tst);       TST_BEG(1, num);
  vstr_add_cstr_buf(tst, tst->len, STUFF);
  TST_END("add_cstr_buf");
  }
  
  del_alloc(tst); TST_BEG(1, num);
  vstr_add_fmt(tst, tst->len, "%s", STUFF);
  TST_END("add_fmt(%s) (alloc)");
  del(tst);       TST_BEG(1, num);
  vstr_add_fmt(tst, tst->len, "%s", STUFF);
  TST_END("add_fmt(%s)");
  
  del_alloc(tst); TST_BEG(1, num);
  vstr_add_fmt(tst, tst->len, "${buf:%s%zu}", STUFF, strlen(STUFF));
  TST_END("add_fmt(${buf}) (alloc)");
  del(tst);       TST_BEG(1, num);
  vstr_add_fmt(tst, tst->len, "${buf:%s%zu}", STUFF, strlen(STUFF));
  TST_END("add_fmt(${buf})");
  
  TST_HDR_END();
  
  vstr_add_fmt(out, out->len, "data        = %c%s%c\n", '"', STUFF, '"');
  vstr_add_fmt(out, out->len, "iter        = %'13u\n", num);
  vstr_add_fmt(out, out->len, "len         = %'13zu\n", tst->len);
  vstr_add_fmt(out, out->len, "num         = %'13u\n", tst->num);
  vstr_add_fmt(out, out->len, "hand inline = %s\n", mcpy_type_map[MCPY_TYPE]);
  vstr_add_fmt(out, out->len, "have iov    = %s\n", YES_NO(HAVE_IOV));
  vstr_add_fmt(out, out->len, "vstr inline = %s\n",
               YES_NO(VSTR_COMPILE_INLINE));

  if (out->conf->malloc_bad)
    errno = ENOMEM, err(EXIT_FAILURE, "tst");
  
  while (out->len)
    if (!vstr_sc_write_fd(out, 1, out->len, 1, NULL))
      err(EXIT_FAILURE, "write");
  
  vstr_free_base(tst);
  vstr_free_base(out);
  vstr_exit();
  
  exit (EXIT_SUCCESS);
}
