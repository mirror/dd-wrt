#include "tst-main.c"

static const char *rf = __FILE__;

#define TST_IP(flags, ip0, ip1, ip2, ip3, ci, num, er) do { \
  memset(ips, -1, 4); \
  cidr = -1; \
  \
  ret = vstr_parse_ipv4(s1, 1, s1->len, ips, &cidr, flags, &num_len, &err); \
  \
  if (err != (er)) return (1); \
  if (!err || (err == VSTR_TYPE_PARSE_IPV4_ERR_ONLY)) { \
  if (ips[0] != (ip0))  return (2); \
  if (ips[1] != (ip1))  return (3); \
  if (ips[2] != (ip2))  return (4); \
  if (ips[3] != (ip3))  return (5); \
  if (cidr != (ci))     return (6); \
  if (num_len != (num)) return (7); \
  } \
} while (FALSE);


int tst(void)
{
  unsigned char ips[4];
  unsigned int cidr = -1;
  size_t num_len = -1;
  unsigned int err = -1;
  int ret = 0;


  VSTR_ADD_CSTR_BUF(s1, 0, "127.0.0.1/8");

  TST_IP(VSTR_FLAG_PARSE_IPV4_DEF, 127, 0, 0, 1, 32, strlen("127.0.0.1"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV4_FULL, 127, 0, 0, 1, 32, strlen("127.0.0.1"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV4_CIDR, 127, 0, 0, 1, 8, strlen("127.0.0.1/8"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, CIDR_FULL),
         127, 0, 0, 1, 8, strlen("127.0.0.1/8"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 8, strlen("127.0.0.1/8"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV4_NETMASK,
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB);
  vstr_del(s1, s1->len, 1);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, CIDR_FULL),
         0, 0, 0, 0, 0, strlen("127.0.0.1/"),
         VSTR_TYPE_PARSE_IPV4_ERR_CIDR_FULL);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, NETMASK, NETMASK_FULL),
         0, 0, 0, 0, 0, strlen("127.0.0.1/"),
         VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_FULL);


  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.000.000.001/08");

  TST_IP(VSTR_FLAG03(PARSE_IPV4, ZEROS, CIDR, CIDR_FULL),
         127, 0, 0, 1, 8, strlen("127.000.000.001/08"), 0);


  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/33");

  TST_IP(VSTR_FLAG_PARSE_IPV4_CIDR,
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_CIDR_OOB);


  VSTR_SUB_CSTR_BUF(s1, 1,s1->len,  "256.0.0.1");

  TST_IP(VSTR_FLAG_PARSE_IPV4_DEF,
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_IPV4_OOB);


  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.128.0");

  TST_IP(VSTR_FLAG_PARSE_IPV4_NETMASK,
         127, 0, 0, 1, 17, strlen("127.0.0.1/255.255.128.0"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, NETMASK, NETMASK_FULL),
         127, 0, 0, 1, 17, strlen("127.0.0.1/255.255.128.0"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV4_ONLY,
         127, 0, 0, 1, 32, strlen("127.0.0.1"), VSTR_TYPE_PARSE_IPV4_ERR_ONLY);
  vstr_sub_cstr_buf(s1, s1->len, 1, "X");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 17, strlen("127.0.0.1/255.255.128."), 0);
  vstr_sc_reduce(s1, 1, s1->len, 1);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 17, strlen("127.0.0.1/255.255.128."), 0);
  vstr_sc_reduce(s1, 1, s1->len, 1);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 17, strlen("127.0.0.1/255.255.128"), 0);


  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.255");

  TST_IP(VSTR_FLAG_PARSE_IPV4_NETMASK,
         127, 0, 0, 1, 32, strlen("127.0.0.1/255.255.255.255"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, NETMASK, NETMASK_FULL),
         127, 0, 0, 1, 32, strlen("127.0.0.1/255.255.255.255"), 0);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 32, strlen("127.0.0.1/255.255.255.255"), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV4_ONLY,
         127, 0, 0, 1, 32, strlen("127.0.0.1"), VSTR_TYPE_PARSE_IPV4_ERR_ONLY);
  vstr_sc_reduce(s1, 1, s1->len, 1 * 4);
  TST_IP(VSTR_FLAG02(PARSE_IPV4, NETMASK, NETMASK_FULL),
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_FULL);
  vstr_sc_reduce(s1, 1, s1->len, (3 * 4) - 1);
  TST_IP(VSTR_FLAG03(PARSE_IPV4, CIDR, NETMASK, LOCAL),
         127, 0, 0, 1, 32, strlen("127.0.0.1/"), 0);
  vstr_sc_reduce(s1, 1, s1->len, 1);
  TST_IP(VSTR_FLAG03(PARSE_IPV4, CIDR, NETMASK, LOCAL),
         127, 0, 0, 1, 32, strlen("127.0.0.1"), 0);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.256.0");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.192.1");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_NETMASK_OOB);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.254");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 31, strlen("127.0.0.1/255.255.255.254"), 0);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.252");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 30, strlen("127.0.0.1/255.255.255.252"), 0);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.248");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 29, strlen("127.0.0.1/255.255.255.248"), 0);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.240");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 28, strlen("127.0.0.1/255.255.255.240"), 0);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.224");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 27, strlen("127.0.0.1/255.255.255.224"), 0);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.255.192");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 26, strlen("127.0.0.1/255.255.255.192"), 0);
  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0.0.1/255.255.192.0");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 18, strlen("127.0.0.1/255.255.192.0"), 0);

  vstr_sub_cstr_buf(s1, 1, s1->len, "127.0.0.1/255.255.192:80");
  TST_IP(VSTR_FLAG02(PARSE_IPV4, CIDR, NETMASK),
         127, 0, 0, 1, 18, strlen("127.0.0.1/255.255.192"), 0);

  VSTR_SUB_CSTR_BUF(s1, 1, s1->len, "127.0. 0.1 **");
  TST_IP(VSTR_FLAG_PARSE_IPV4_DEF, 127, 0, 0, 0, 32, strlen("127.0."), 0);
  TST_IP(VSTR_FLAG_PARSE_IPV4_FULL,
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_IPV4_FULL);
  TST_IP(VSTR_FLAG_PARSE_IPV4_ONLY,
         127, 0, 0, 0, 32, strlen("127.0."), VSTR_TYPE_PARSE_IPV4_ERR_ONLY);

  vstr_del(s1, strlen("127.0."), 1);
  TST_IP(VSTR_FLAG_PARSE_IPV4_FULL,
         0, 0, 0, 0, 0, 0, VSTR_TYPE_PARSE_IPV4_ERR_IPV4_FULL);
  TST_IP(VSTR_FLAG_PARSE_IPV4_ONLY,
         127, 0, 0, 0, 32, strlen("127.0"), VSTR_TYPE_PARSE_IPV4_ERR_ONLY);
  
  vstr_sc_reduce(s1, 1, s1->len, s1->len - strlen("127.0"));
  TST_IP(VSTR_FLAG_PARSE_IPV4_ONLY,
         127, 0, 0, 0, 32, strlen("127.0"), VSTR_TYPE_PARSE_IPV4_ERR_NONE);
  
  return (0);
}

/* tst_coverage
 *
 * VSTR_FLAG_PARSE_IPV4_LOCAL
 * VSTR_FLAG_PARSE_IPV4_CIDR_FULL
 * VSTR_FLAG_PARSE_IPV4_NETMASK_FULL
 * VSTR_FLAG_PARSE_IPV4_ZEROS
 *
 */
