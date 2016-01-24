#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  size_t pos = 0;
  size_t len = 0;
  size_t dlen = 0;
  unsigned int buf_pos = 0;

  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  vstr_add_fmt(s1, s1->len, "%zu:%n%s,", strlen(buf), &buf_pos, buf);
  ++buf_pos;

  len = vstr_parse_netstr(s1, 1, s1->len, &pos, &dlen);
  TST_B_TST(ret, 1,
            (len != s1->len));
  TST_B_TST(ret, 2,
            (dlen != strlen(buf)));
  TST_B_TST(ret, 3,
            (pos != buf_pos));

  TST_B_TST(ret, 4,
            (len != vstr_parse_netstr(s1, 1, s1->len, NULL, NULL)));

  TST_B_TST(ret, 5, !!vstr_parse_netstr(s1, s1->len, 1, NULL, NULL));
  TST_B_TST(ret, 6, !!vstr_parse_netstr(s1, 1, 1, NULL, NULL));
  TST_B_TST(ret, 7, !!vstr_parse_netstr(s1, 2, s1->len - 1, NULL, NULL));
  vstr_del(s1, s1->len, 1);
  TST_B_TST(ret, 8, !!vstr_parse_netstr(s1, 1, s1->len, NULL, NULL));
  vstr_del(s1, s1->len, 1);
  VSTR_ADD_CSTR_PTR(s1, s1->len, ".");
  TST_B_TST(ret, 9, !!vstr_parse_netstr(s1, 1, s1->len, NULL, NULL));

  VSTR_SUB_CSTR_PTR(s1, 1, s1->len, "4.");
  TST_B_TST(ret, 10, !!vstr_parse_netstr(s1, 1, s1->len, NULL, NULL));

  return (TST_B_RET(ret));
}
