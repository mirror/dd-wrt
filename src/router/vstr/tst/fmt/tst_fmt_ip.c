#include "tst-main.c"

static const char *rf = __FILE__;

#ifdef HAVE_POSIX_HOST
static int tst_setup_inet_buf(const struct in_addr  *ipv4,
                              const struct in6_addr *ipv6, int fwd)
{
  size_t len = 0;
  const char *ptr = NULL;

  assert(256 >= (INET_ADDRSTRLEN + INET6_ADDRSTRLEN));
  assert(sizeof(buf) > 256);

  memset(buf, ' ', 256);
  buf[256] = 0;
  buf[257] = (char)0xFF;

  if (fwd)
  {
    if (!(ptr = inet_ntop(AF_INET,  ipv4, buf, 128)))
      return (FALSE);
  }
  else
  {
    if (!(ptr = inet_ntop(AF_INET6, ipv6, buf, 128)))
      return (FALSE);
  }
  
  len = strlen(ptr);
  assert(len < 16);

  assert(ptr == buf);

  /* turn '\0' into a space */
  assert(!buf[len]);
  buf[len] = ' ';

  if (fwd)
  {
    if (!(ptr = inet_ntop(AF_INET6, ipv6, buf + 128, 128)))
      return (FALSE);
  }
  else
  {
    if (!(ptr = inet_ntop(AF_INET,  ipv4, buf + 128, 128)))
      return (FALSE);
  }
  
  len = strlen(ptr);
  assert(len < 40);

  memmove(buf + (256 - len), buf + 128, len); /* move ipv6 "object" to end */
  memset(buf + 128, ' ', 128 - len);

  assert(!buf[256]);
  assert( buf[257] == (char)0xFF);

  return (TRUE);
}

#endif

int tst(void)
{
  int ret = 0;
  unsigned int ips[8];
  unsigned int scan = 0;
  int mfail_count = 0;

  vstr_cntl_conf(s3->conf, VSTR_CNTL_CONF_SET_FMT_CHAR_ESC, '$');

  vstr_sc_fmt_add_all(s3->conf);

  ASSERT(s3->conf->fmt_usr_curly_braces);

  /* output */

#ifdef HAVE_POSIX_HOST
  scan = 2;
  while (scan--)
  {
    struct in_addr  ipv4;
    struct in6_addr ipv6;
    unsigned int count = 0;
    int fmtret = 0;
    
    srand(time(NULL) ^ getpid());

    ipv4.s_addr = rand();

    ipv6.s6_addr[0] = rand();
    ipv6.s6_addr[1] = rand();
    ipv6.s6_addr[2] = rand();
    ipv6.s6_addr[3] = rand();

    if (!tst_setup_inet_buf(&ipv4, &ipv6, scan)) break;

    vstr_del(s3, 1, s3->len);
    if (scan)
    vstr_add_fmt(s3, 0, "$-128{ipv4.p:%p}$128{ipv6.p:%p}", &ipv4, &ipv6);
    else
    vstr_add_fmt(s3, 0, "$-128{ipv6.p:%p}$128{ipv4.p:%p}", &ipv6, &ipv4);

    TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

    ipv4.s_addr = 0xF00F;

    count = 0;
    while (count < 4)
    {
      ipv6.s6_addr[0] = 0xF0;
      ipv6.s6_addr[1] = 0x0F;
      ipv6.s6_addr[2] = 0x0F;
      ipv6.s6_addr[3] = 0xF0;
      ++count;
    }

    if (!tst_setup_inet_buf(&ipv4, &ipv6, scan)) break;

    vstr_del(s3, 1, s3->len);
    mfail_count = 0;
    do
    {
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count);
      if (scan)
      fmtret = vstr_add_fmt(s3, 0, "$-128{ipv4.p:%p}$128{ipv6.p:%p}", &ipv4, &ipv6);
      else
      fmtret = vstr_add_fmt(s3, 0, "$-128{ipv6.p:%p}$128{ipv4.p:%p}", &ipv6, &ipv4);
    } while (!fmtret);
    tst_mfail_num(0);

    TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

    ipv4.s_addr = 0xFFFF;

    count = 0;
    while (count < 16)
    {
      ipv6.s6_addr[0] = 0xFF;
      ++count;
    }

    if (!tst_setup_inet_buf(&ipv4, &ipv6, scan)) break;

    vstr_del(s3, 1, s3->len);
    if (scan)
    vstr_add_fmt(s3, 0, "$-128{ipv4.p:%p}$128{ipv6.p:%p}", &ipv4, &ipv6);
    else
    vstr_add_fmt(s3, 0, "$-128{ipv6.p:%p}$128{ipv4.p:%p}", &ipv6, &ipv4);

    TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

    ipv4.s_addr = 0;

    {
      struct in6_addr tmp = IN6ADDR_ANY_INIT;
      ipv6 = tmp;
    }

    if (!tst_setup_inet_buf(&ipv4, &ipv6, scan)) break;

    vstr_del(s3, 1, s3->len);
    if (scan)
    vstr_add_fmt(s3, 0, "$-128{ipv4.p:%p}$128{ipv6.p:%p}", &ipv4, &ipv6);
    else
    vstr_add_fmt(s3, 0, "$-128{ipv6.p:%p}$128{ipv4.p:%p}", &ipv6, &ipv4);

    TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));
    if (ret) { PRNT_VSTR(s3); PRNT_CSTR(buf); }

  }
#endif

  memset(ips, 0, sizeof(ips));

#define TST_IPV6N(num, sv, cstr, type) do {                             \
    vstr_del((sv), 1, (sv)->len);                                       \
    vstr_add_fmt((sv), 0, "${ipv6.v:%p%u}", ips, type);                 \
    TST_B_TST(ret, (num), !VSTR_CMP_CSTR_EQ((sv), 1, (sv)->len, cstr)); \
    if (ret) { PRNT_VSTR(sv); PRNT_CSTR(cstr); }                        \
 } while (FALSE)
#define TST_IPV6C(num, sv, cstr, type, cidr) do {                       \
    vstr_del((sv), 1, (sv)->len);                                       \
    vstr_add_fmt((sv), 0, "${ipv6.v+C:%p%u%u}", ips, type, cidr);       \
    TST_B_TST(ret, (num), !VSTR_CMP_CSTR_EQ((sv), 1, (sv)->len, cstr)); \
    if (ret) { PRNT_VSTR(sv); PRNT_CSTR(cstr); }                        \
 } while (FALSE)

  TST_IPV6N( 5, s3, "::", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C( 5, s3, "::/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N( 6, s3, "0:0:0:0:0:0:0:0", VSTR_TYPE_SC_FMT_CB_IPV6_STD);
  TST_IPV6C( 6, s3, "0:0:0:0:0:0:0:0/128", VSTR_TYPE_SC_FMT_CB_IPV6_STD, 128);
  TST_IPV6N( 7, s3, "0000:0000:0000:0000:0000:0000:0000:0000",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED);
  TST_IPV6C( 7, s3, "0000:0000:0000:0000:0000:0000:0000:0000/4",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 4);

  ips[0] = 0xF; ips[7] = 0x1;
  vstr_del(s3, 1, s3->len);
  strcpy(buf, "  F::1");
  vstr_add_fmt(s3, 0, "$*{ipv6.v:%*p%u}", 6, ips,
               VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_B_TST(ret, 8, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));
  vstr_del(s3, 1, s3->len);
  strcpy(buf, "F::1  ");
  vstr_add_fmt(s3, 0, "$*{ipv6.v:%*p%u}", -6, ips,
               VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_B_TST(ret, 8, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));
  TST_IPV6C( 8, s3, "F::1/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N( 9, s3, "F:0:0:0:0:0:0:1", VSTR_TYPE_SC_FMT_CB_IPV6_STD);
  TST_IPV6C( 9, s3, "F:0:0:0:0:0:0:1/128", VSTR_TYPE_SC_FMT_CB_IPV6_STD, 128);
  TST_IPV6N(10, s3, "000F:0000:0000:0000:0000:0000:0000:0001",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED);
  TST_IPV6C(10, s3, "000F:0000:0000:0000:0000:0000:0000:0001/4",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 4);

  ips[0] = 0x0; ips[1] = 0xA;
  TST_IPV6N(11, s3, "0:A::1", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(11, s3, "0:A::1/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(12, s3, "0:A:0:0:0:0:0:1", VSTR_TYPE_SC_FMT_CB_IPV6_STD);
  TST_IPV6C(12, s3, "0:A:0:0:0:0:0:1/128", VSTR_TYPE_SC_FMT_CB_IPV6_STD, 128);
  TST_IPV6N(13, s3, "0000:000A:0000:0000:0000:0000:0000:0001",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED);
  TST_IPV6C(13, s3, "0000:000A:0000:0000:0000:0000:0000:0001/4",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 4);

  scan = 0;
  while (scan++ < 7) ips[scan] = scan;

  TST_IPV6N(14, s3, "::1:2:3:4:5:6:7", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(14, s3, "::1:2:3:4:5:6:7/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(15, s3, "0:1:2:3:4:5:6:7", VSTR_TYPE_SC_FMT_CB_IPV6_STD);
  TST_IPV6C(15, s3, "0:1:2:3:4:5:6:7/128", VSTR_TYPE_SC_FMT_CB_IPV6_STD, 128);
  TST_IPV6N(16, s3, "0000:0001:0002:0003:0004:0005:0006:0007",
            VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED);
  TST_IPV6C(16, s3, "0000:0001:0002:0003:0004:0005:0006:0007/4",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 4);
  TST_IPV6N(16, s3, "0000:0001:0002:0003:0004:0005:0.6.0.7",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED);
  TST_IPV6C(16, s3, "0000:0001:0002:0003:0004:0005:0.6.0.7/4",
             VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED, 4);

  TST_IPV6N(17, s3, "::1:2:3:4:5:0.6.0.7",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(17, s3, "::1:2:3:4:5:0.6.0.7/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 64);

  ips[1] = 0;
  TST_IPV6N(18, s3, "::2:3:4:5:6:7", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(18, s3, "::2:3:4:5:6:7/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);

  scan = 0;
  while (scan < 8)
  {
    vstr_del(s3, 1, s3->len);
    mfail_count = 3;
    do
    {
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count / 4);    
    } while (!vstr_add_fmt(s3, 0, "%*s${ipv6.v:%p%u}", scan, "", ips,
                           VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT));
    tst_mfail_num(0);
    TST_B_TST(ret, 18, !vstr_cmp_cstr_eq(s3, 1 + scan, s3->len - scan,
                                         "::2:3:4:5:0.6.0.7"));
    
    vstr_del(s3, 1, s3->len);
    mfail_count = 3;
    do
    {
      vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
      tst_mfail_num(++mfail_count / 4);
    }
    while (!vstr_add_fmt(s3, 0, "%*s${ipv6.v+C:%p%u%u}", scan, "", ips,
                         VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 32));
    tst_mfail_num(0);
    TST_B_TST(ret, 18, !vstr_cmp_cstr_eq(s3, 1 + scan, s3->len - scan,
                                         "::2:3:4:5:0.6.0.7/32"));

    ++scan;
  }
  
  ips[6] = 0xFFFF;
  ips[7] = 0xFFFF;

  TST_IPV6N(18, s3, "::2:3:4:5:FFFF:FFFF",
            VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(18, s3, "::2:3:4:5:FFFF:FFFF/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(18, s3, "::2:3:4:5:255.255.255.255",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(18, s3, "::2:3:4:5:255.255.255.255/32",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 32);

  scan = 0;
  while (scan++ < 7) ips[scan - 1] = scan;

  ips[7] = 0;
  TST_IPV6N(19, s3, "1:2:3:4:5:6:7::", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(19, s3, "1:2:3:4:5:6:7::/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(20, s3, "1:2:3:4:5:6:7:0", VSTR_TYPE_SC_FMT_CB_IPV6_STD);
  TST_IPV6C(20, s3, "1:2:3:4:5:6:7:0/128", VSTR_TYPE_SC_FMT_CB_IPV6_STD, 128);
  TST_IPV6N(21, s3, "0001:0002:0003:0004:0005:0006:0007:0000",
            VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED);
  TST_IPV6C(21, s3, "0001:0002:0003:0004:0005:0006:0007:0000/4",
             VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED, 4);

  TST_IPV6N(22, s3, "1:2:3:4:5:6:0.7.0.0",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(22, s3, "1:2:3:4:5:6:0.7.0.0/127",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 127);

  ips[6] = 0;
  TST_IPV6N(23, s3, "1:2:3:4:5:6::", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(23, s3, "1:2:3:4:5:6::/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(24, s3, "1:2:3:4:5:6:0.0.0.0",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(24, s3, "1:2:3:4:5:6:0.0.0.0/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 64);

  ips[1] = 0;
  TST_IPV6N(25, s3, "1:0:3:4:5:6::", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(25, s3, "1:0:3:4:5:6::/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(26, s3, "1::3:4:5:6:0.0.0.0",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(26, s3, "1::3:4:5:6:0.0.0.0/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 64);

  ips[3] = 0; ips[4] = 0;
  TST_IPV6N(27, s3, "1:0:3::6:0:0", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(27, s3, "1:0:3::6:0:0/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(27, s3, "1:0:3::6:0.0.0.0",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(27, s3, "1:0:3::6:0.0.0.0/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 64);

  ips[0] = 0;
  TST_IPV6N(28, s3, "0:0:3::6:0:0", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(28, s3, "0:0:3::6:0:0/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(28, s3, "0:0:3::6:0.0.0.0",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(28, s3, "0:0:3::6:0.0.0.0/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 64);

  ips[3] = 4;

  TST_IPV6N(28, s3, "::3:4:0:6:0:0", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6C(28, s3, "::3:4:0:6:0:0/64", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT, 64);
  TST_IPV6N(28, s3, "::3:4:0:6:0.0.0.0",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6C(28, s3, "::3:4:0:6:0.0.0.0/64",
            VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT, 64);

  scan = 0;
  while (scan++ < 8) ips[scan - 1] = scan;

  ips[1] = 0; ips[3] = 0; ips[4] = 0;
  TST_IPV6N(29, s3, "1:0:3::6:7:8", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  ips[0] = 0;
  TST_IPV6N(29, s3, "0:0:3::6:7:8", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  ips[2] = 0; ips[3] = 4;
  TST_IPV6N(29, s3, "::4:0:6:7:8", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);

#define IPV4(n1, n2) (((n1) << 8) | (n2))
  /* test rfc */
  ips[0] = 0x1080; ips[1] = 0x0000; ips[2] = 0x0000; ips[3] = 0x0000;
  ips[4] = 0x0008; ips[5] = 0x0800; ips[6] = 0x200C; ips[7] = 0x417A;
  TST_IPV6N(29, s3, "1080::8:800:200C:417A", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6N(29, s3, "1080:0:0:0:8:800:200C:417A", VSTR_TYPE_SC_FMT_CB_IPV6_STD);

  ips[0] = 0xFF01; ips[1] = 0x0000; ips[2] = 0x0000; ips[3] = 0x0000;
  ips[4] = 0x0000; ips[5] = 0x0000; ips[6] = 0x0000; ips[7] = 0x0101;
  TST_IPV6N(29, s3, "FF01::101", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6N(29, s3, "FF01:0:0:0:0:0:0:101", VSTR_TYPE_SC_FMT_CB_IPV6_STD);

  ips[0] = 0x0000; ips[1] = 0x0000; ips[2] = 0x0000; ips[3] = 0x0000;
  ips[4] = 0x0000; ips[5] = 0x0000; ips[6] = IPV4(13U, 1); ips[7] = IPV4(68U,3);
  TST_IPV6N(29, s3, "::13.1.68.3", VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT);
  TST_IPV6N(29, s3, "0:0:0:0:0:0:13.1.68.3", VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD);

  ips[0] = 0x12AB; ips[1] = 0x0000; ips[2] = 0x0000; ips[3] = 0xCD30;
  ips[4] = 0x0000; ips[5] = 0x0000; ips[6] = 0x0000; ips[7] = 0x0000;
  TST_IPV6N(29, s3, "12AB:0:0:CD30::", VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT);
  TST_IPV6N(29, s3, "12AB:0000:0000:CD30:0000:0000:0000:0000",
            VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED);
  TST_IPV6N(29, s3, "12AB:0:0:CD30:0:0:0:0", VSTR_TYPE_SC_FMT_CB_IPV6_STD);

#define TST_IPV4N(num, sv, cstr) do { \
  vstr_del((sv), 1, (sv)->len); \
  strcpy(buf, cstr); \
  vstr_add_fmt((sv), 0, "${ipv4.v:%p}", ips); \
  TST_B_TST(ret, (num), !VSTR_CMP_CSTR_EQ((sv), 1, (sv)->len, buf)); \
 } while (FALSE)
#define TST_IPV4C(num, sv, cstr, cidr) do { \
  vstr_del((sv), 1, (sv)->len); \
  strcpy(buf, cstr); \
  vstr_add_fmt((sv), 0, "${ipv4.v+C:%p%u}", ips, cidr); \
  TST_B_TST(ret, (num), !VSTR_CMP_CSTR_EQ((sv), 1, (sv)->len, buf)); \
 } while (FALSE)

  scan = 0;
  while (scan++ < 4) ips[scan - 1] = scan;
  TST_IPV4N(30, s3, "1.2.3.4");
  TST_IPV4C(30, s3, "1.2.3.4/12", 12);
  ips[0] = 0;
  TST_IPV4N(30, s3, "0.2.3.4");
  TST_IPV4C(30, s3, "0.2.3.4/32", 32);
  ips[2] = 0;
  TST_IPV4N(30, s3, "0.2.0.4");
  TST_IPV4C(30, s3, "0.2.0.4/0", 0);

  scan = 0;
  while (scan < 4) ips[scan++] = 255;
  TST_IPV4N(30, s3, "255.255.255.255");
  TST_IPV4C(30, s3, "255.255.255.255/16", 16);

  return (TST_B_RET(ret));
}
