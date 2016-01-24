#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  Vstr_conf *conf = vstr_make_conf();
  Vstr_base *sx = NULL;
  
  vstr_cntl_conf(conf, VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE,
                 VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR);
  
  sx = vstr_make_base(conf);
  vstr_free_conf(conf);
  vstr_free_base(sx);

  return (EXIT_SUCCESS);
}


