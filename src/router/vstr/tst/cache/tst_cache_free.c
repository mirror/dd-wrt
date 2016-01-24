#include "tst-main.c"

static const char *rf = __FILE__;

static unsigned int tst_cache_pos = 0;

static int dummy = 4;

static void *tst_cache_cb(const Vstr_base *base __attribute__((unused)),
                          size_t pos, size_t len,
                          unsigned int type, void *passed_data)
{
  (void)pos;
  (void)len;
  (void)passed_data;
  
  if (type == VSTR_TYPE_CACHE_FREE)
    return (NULL);

  return (&dummy);
}

static void tst_base(Vstr_base *base)
{
  vstr_add_rep_chr(base, 0, 'X', 1);
  
  tst_cache_pos = vstr_cache_srch(base->conf, "/tst_usr/dummy");
  if (!tst_cache_pos)
    tst_cache_pos = vstr_cache_add(base->conf, "/tst_usr/dummy", tst_cache_cb);
  ASSERT(tst_cache_pos);

  if (vstr_cache_set(base, tst_cache_pos, &dummy))
  {
    ASSERT(vstr_cache_get(base, tst_cache_pos) == &dummy);

    vstr_cache_cb_sub(base, 1, 1);
    ASSERT(vstr_cache_get(base, tst_cache_pos) == &dummy);

    vstr_cache_cb_free(base, 3);
    vstr_cache_cb_free(base, 2);
    vstr_cache_cb_free(base, 1);
    ASSERT(vstr_cache_get(base, tst_cache_pos) == &dummy);
  }
  else
    vstr_cache_cb_sub(base, 1, 1);
  vstr_cache_cb_free(base, 0);
  ASSERT(!vstr_cache_get(base, tst_cache_pos));
}

int tst(void)
{ /* other stuff done in export_cstr_ptr */
  Vstr_base *tmp = NULL;
  
  tst_base(s1);
  tst_base(s2);
  tst_base(s3);
  tst_base(s4);

  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC);
  tmp = vstr_dup_cstr_buf(NULL, "abcd");
  tst_base(tmp);
  vstr_free_base(tmp);
  
  vstr_cntl_conf(NULL, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
  tmp = vstr_dup_cstr_buf(NULL, "abcd");
  tst_base(tmp);
  vstr_free_base(tmp);
  
  return (EXIT_SUCCESS);
}
