#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  const char *p1 = NULL;
  const char *p2 = NULL;
  Vstr_conf *conf = vstr_make_conf();
  Vstr_base *sx = NULL;
  
  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);

  sx = vstr_make_base(conf);
  vstr_free_conf(conf);
  
  vstr_add_cstr_buf(s1, s1->len, "12345678");
  vstr_add_cstr_buf(sx, sx->len, "12345678");
  
  ASSERT((p1 = vstr_export_cstr_ptr(s1, 1, 4)));
  ASSERT((p2 = vstr_export_cstr_ptr(sx, 1, 4)));

  ASSERT(p1 != p2);
  ASSERT(!memcmp(p1, p2, 4));
  vstr_del(sx, 1, sx->len);
  
  vstr_add_vstr(sx, 0, s1, 1, s1->len, VSTR_TYPE_ADD_DEF);
  
  ASSERT(vstr_cmp_eq(s1, 1, s1->len, sx, 1, sx->len));
  
  ASSERT((p2 = vstr_export_cstr_ptr(sx, 1, 4)));

  ASSERT(p1 == p2);

  vstr_del(sx, 1, sx->len);
  vstr_cache_cb_free(sx, 3);
  vstr_add_cstr_buf(sx, sx->len, "1");

  /* Hack ... no way to guarantee to do this anymore...
     (but it can happen, Eg. on malloc -- also old inlines of vstr_del()) */
  {
    struct Vstr__cache_data_cstr *data = vstr_cache_get(sx, 3);

    ASSERT(data);
    ASSERT(!data->ref);
    ASSERT(!data->len);

    data->pos = 1;
    data->len = 1;

    vstr_add_vstr(sx, sx->len, s1, 1, s1->len, VSTR_TYPE_ADD_DEF);
    
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, sx, 2, sx->len - 1));
    
    p2 = vstr_export_cstr_ptr(sx, 2, 4);
  }

  vstr_free_base(sx);
  
  return (p1 != p2);
}
