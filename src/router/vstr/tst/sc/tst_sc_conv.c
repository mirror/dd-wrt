#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t len = sizeof(buf);
  char buf2[1024];
  
  TST_B_TST(ret,  1, vstr_sc_conv_num_uint(buf, len,
                                           (3 * 4 * 4 * 4) +
                                           (2 * 4 * 4) +
                                           (1 * 4) + 0, "dcba", 4) != 4);
  
  TST_B_TST(ret,  2, !!strcmp(buf, "abcd"));
  
  TST_B_TST(ret,  3, vstr_sc_conv_num10_uint(buf, len, 1234) != 4);
  TST_B_TST(ret,  4, !!strcmp(buf, "1234"));
  
  TST_B_TST(ret,  5,
            vstr_sc_conv_num_ulong(buf, len, 0xDEADBEEF, "01", 2) != 32);
  TST_B_TST(ret,  6, !!strcmp(buf, "11011110101011011011111011101111"));
  
  TST_B_TST(ret,  7, vstr_sc_conv_num10_ulong(buf, len, 0xDEADBEEF) != 10);
  TST_B_TST(ret,  8, !!strcmp(buf, "3735928559"));
  
  TST_B_TST(ret,  9,
            vstr_sc_conv_num_size(buf, len, 0xDEADBEEF, "XXXXXXXXXXABCDEF", 16)
            != 8);
  TST_B_TST(ret, 10, !!strcmp(buf, "DEADBEEF"));
  
  TST_B_TST(ret, 11, vstr_sc_conv_num10_size(buf, len, 4321) != 4);
  TST_B_TST(ret, 12, !!strcmp(buf, "4321"));
  
  len = sprintf(buf2, "%jx", UINTMAX_MAX);
  if (strcmp("jx", buf2)) /* Solaris POS */
  {
    TST_B_TST(ret, 13, vstr_sc_conv_num_uintmax(buf, len + 1, UINTMAX_MAX,
                                                "0123456789abcdef", 16) != len);
    TST_B_TST(ret, 14, !!strcmp(buf, buf2));
  
    len = sprintf(buf2, "%ju", UINTMAX_MAX);
    TST_B_TST(ret, 15, vstr_sc_conv_num10_uintmax(buf, len + 1,
                                                  UINTMAX_MAX) != len);
    TST_B_TST(ret, 16, !!strcmp(buf, buf2));
  }
  
  TST_B_TST(ret, 17, vstr_sc_conv_num10_uintmax(buf, 5, 1234) != 4);
  TST_B_TST(ret, 18, !!strcmp(buf, "1234"));
  
  TST_B_TST(ret, 19, !!vstr_sc_conv_num10_uint(buf, 4, 1234));
  TST_B_TST(ret, 20, !!strcmp(buf, ""));

  TST_B_TST(ret, 21, !!vstr_sc_conv_num10_uint(buf, 3, 1234));
  TST_B_TST(ret, 22, !!strcmp(buf, ""));

  TST_B_TST(ret, 23, vstr_sc_conv_num10_uint(buf, 3, 34) != 2);
  TST_B_TST(ret, 24, !!strcmp(buf, "34"));

  TST_B_TST(ret, 25, !!vstr_sc_conv_num10_uint(buf, 2, 34));
  TST_B_TST(ret, 26, !!strcmp(buf, ""));
  
  TST_B_TST(ret, 27, vstr_sc_conv_num10_uint(buf, 2, 4) != 1);
  TST_B_TST(ret, 28, !!strcmp(buf, "4"));

  TST_B_TST(ret, 29, vstr_sc_conv_num_uint(buf, 2, 80,
"                                                                                X", 81) != 1);
  TST_B_TST(ret, 29, !!strcmp(buf, "X"));
  
  return (TST_B_RET(ret));
}
