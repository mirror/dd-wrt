#include "xfail-tst-main.c"
                                                                                
static const char *rf = __FILE__;
                                                                                
void xfail_tst(void)
{
  vstr_sc_conv_num10_uint(buf, 1, 1);
}
