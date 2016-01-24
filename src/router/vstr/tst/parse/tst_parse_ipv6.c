#include "tst-main.c"

static const char *rf = __FILE__;

#define TST_IP(flags, ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7, ci, num, er) do{ \
  int tret = 0; \
  memset(ips, -1, 4 * sizeof(unsigned int)); \
  cidr = -1; \
  \
  tret = vstr_parse_ipv6(s1, 1, s1->len, ips, &cidr, flags, &num_len, NULL); \
  ret  = vstr_parse_ipv6(s1, 1, s1->len, ips, &cidr, flags, &num_len, &err); \
  ASSERT(ret == tret); \
  \
  if (err != (er))      { PRNT_VSTR(s1); return (1); } \
  if (!err || (err == VSTR_TYPE_PARSE_IPV6_ERR_ONLY)) { \
  if (ips[0] != (ip0))  { PRNT_VSTR(s1); return (2); } \
  if (ips[1] != (ip1))  { PRNT_VSTR(s1); return (3); } \
  if (ips[2] != (ip2))  { PRNT_VSTR(s1); return (4); } \
  if (ips[3] != (ip3))  { PRNT_VSTR(s1); return (5); } \
  if (ips[4] != (ip4))  { PRNT_VSTR(s1); return (6); } \
  if (ips[5] != (ip5))  { PRNT_VSTR(s1); return (7); } \
  if (ips[6] != (ip6))  { PRNT_VSTR(s1); return (8); } \
  if (ips[7] != (ip7))  { PRNT_VSTR(s1); return (9); } \
  if (cidr != (ci))     { PRNT_VSTR(s1); return (10); } \
  if (num_len != (num)) { PRNT_VSTR(s1); return (11); } \
  } \
} while (FALSE);


int tst(void)
{
  unsigned int ips[8];
  unsigned int cidr = -1;
  size_t num_len = -1;
  unsigned int err = -1;
  int ret = -1;

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "::/64");

  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, 0, 0, 128, strlen("::"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0, 0, 0, 0, 0, 0, 0, 0,  64, strlen("::/64"), 0);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "::1/64");

  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, 0, 1, 128, strlen("::1"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0, 0, 0, 0, 0, 0, 0, 1,  64, strlen("::1/64"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV6, CIDR, CIDR_FULL),
         0, 0, 0, 0, 0, 0, 0, 1,  64, strlen("::1/64"), 0);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "0:FFF::/24");

  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0xFFF, 0, 0, 0, 0, 0, 0, 128, strlen("0:FFF::"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0, 0xFFF, 0, 0, 0, 0, 0, 0,  24, strlen("0:FFF::/24"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV6, CIDR, CIDR_FULL),
         0, 0xFFF, 0, 0, 0, 0, 0, 0,  24, strlen("0:FFF::/24"), 0);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "FFF::1/8");

  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0xFFF, 0, 0, 0, 0, 0, 0, 1, 128, strlen("FFF::1"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0xFFF, 0, 0, 0, 0, 0, 0, 1,   8, strlen("FFF::1/8"), 0);
  vstr_sc_reduce(s1, 1, s1->len, 1);
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0xFFF, 0, 0, 0, 0, 0, 0, 1, 128, strlen("FFF::1/"), 0);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "FFF::1/138");

  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV6_ERR_CIDR_OOB);


  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "FFFFF::1");

  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV6_ERR_IPV6_OOB);

  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "F:1:E:2:D:3:c:4");

  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0xF, 1, 0xE, 2, 0xD, 3, 0xC, 4, 128, s1->len, 0);

    /* Ipv4 end format testing... */
#define IPV4(n1, n2) (((n1) << 8) | (n2))

  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "0:1::0:2:254.255.255.0");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 1, 0, 0, 0, 2, IPV4(254U, 255), IPV4(255U, 0),    128, s1->len, 0);
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "0:1::0:2:255.255.1.1");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 1, 0, 0, 0, 2, IPV4(255U, 255), IPV4(1U, 1),      128, s1->len, 0);


  /* from rfc2372 */
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "1080:0:0:0:8:800:200C:417A");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0x1080, 0, 0, 0, 0x8, 0x800, 0x200C, 0x417A, 128, s1->len, 0);
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "1080::8:800:200C:417A");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0x1080, 0, 0, 0, 0x8, 0x800, 0x200C, 0x417A, 128, s1->len, 0);

  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "FF01:0:0:0:0:0:0:101");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0xFF01, 0, 0, 0, 0, 0, 0, 0x101, 128, s1->len, 0);
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "FF01::101");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0xFF01, 0, 0, 0, 0, 0, 0, 0x101, 128, s1->len, 0);

  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "0:0:0:0:0:0:13.1.68.3");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, IPV4(13U, 1), IPV4(68U, 3), 128, s1->len, 0);
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "::13.1.68.3");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, IPV4(13U, 1), IPV4(68U, 3), 128, s1->len, 0);

  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "0:0:0:0:0:FFFF:129.144.52.38");
  TST_IP(VSTR_FLAG_PARSE_IPV6_LOCAL,
         0, 0, 0, 0, 0, 0xFFFF, IPV4(129U, 144), IPV4(52U, 38),
         128, s1->len, 0);
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "::FFFF:129.144.52.38");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0xFFFF, IPV4(129U, 144), IPV4(52U, 38),
         128, s1->len, 0);

  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,"12AB:0000:0000:CD30:0000:0000:0000:0000/60");
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0x12AB, 0, 0, 0xCD30, 0, 0, 0, 0, 60, s1->len, 0);
  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,"12AB::CD30:0:0:0:0/60");
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR,
         0x12AB, 0, 0, 0xCD30, 0, 0, 0, 0, 60, s1->len, 0);
  TST_IP(VSTR_FLAG_PARSE_IPV6_ONLY,
         0x12AB, 0, 0, 0xCD30, 0, 0, 0, 0, 128, s1->len - 3,
         VSTR_TYPE_PARSE_IPV6_ERR_ONLY);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "1::1::3");
  TST_IP(VSTR_FLAG_PARSE_IPV6_LOCAL,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV6_ERR_IPV6_NULL);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "1/8");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV6_ERR_IPV6_FULL);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "1");
  TST_IP(VSTR_FLAG_PARSE_IPV6_DEF,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV6_ERR_IPV6_FULL);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "1:2:3:4:5:6:7:8/");
  TST_IP(VSTR_FLAG_PARSE_IPV6_CIDR | VSTR_FLAG_PARSE_IPV6_CIDR_FULL,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV6_ERR_CIDR_FULL);

  return (0);
}

/* tst_coverage
 *
 * VSTR_FLAG_PARSE_IPV6_LOCAL
 * VSTR_FLAG_PARSE_IPV6_CIDR_FULL
 * VSTR_TYPE_PARSE_IPV6_ERR_NONE
 *
 */
