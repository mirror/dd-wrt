#include "tst-main.c"

static const char *rf = __FILE__;

static void *tst_cache_cb(const Vstr_base *base __attribute__((unused)),
                          size_t pos __attribute__((unused)),
                          size_t len __attribute__((unused)),
                          unsigned int type __attribute__((unused)),
                          void *passed_data __attribute__((unused)))
{
  return (NULL);
}

int tst(void)
{
  unsigned int tst_srch = vstr_cache_add(s1->conf, "/tst_srch/tst",
                                         tst_cache_cb);

  /* vstr internal stuff... */
  if (vstr_cache_srch(s1->conf, "/vstr__/pos") != 1)
    return (1);

  if (vstr_cache_srch(s1->conf, "/vstr__/iovec") != 2)
    return (2);

  if (vstr_cache_srch(s1->conf, "/vstr__/cstr") != 3)
    return (3);

  /* make sure it's the same ... */
  if (vstr_cache_srch(s1->conf, "/tst_srch/tst") != tst_srch)
    return (4);

  if (vstr_cache_srch(s1->conf, "/tst_srch/tst") != 4)
    return (5);

  if (vstr_cache_srch(s1->conf, "/foo") != 0)
    return (6);

  return (EXIT_SUCCESS);
}
