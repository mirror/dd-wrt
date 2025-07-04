/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017-2018, 2021, 2023
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************
 */

#include <util.c>
#include "test.h"

static volatile int handled_signal = 0;

static void
handle_signal(int signal)
{
  handled_signal = signal;
}

void
test_unit(void)
{
  char buf[16], buf2[16], *s, *s2, *words[3];
  struct timespec ts, ts2, ts3, ts4;
  NTP_int64 ntp_ts, ntp_ts2, ntp_fuzz;
  NTP_int32 ntp32_ts;
  struct timeval tv;
  double x, y, nan, inf;
  IPAddr ip, ip2, ip3;
  IPSockAddr ip_saddr;
  Integer64 integer64;
  Timespec tspec;
  Float f;
  int i, j, c;
  uid_t uid;
  gid_t gid;
  struct stat st;
  FILE *file;

  for (i = -31; i < 31; i++) {
    x = pow(2.0, i);
    y = UTI_Log2ToDouble(i);
    TEST_CHECK(y / x > 0.99999 && y / x < 1.00001);
  }

  for (i = -89; i < 63; i++) {
    x = pow(2.0, i);
    y = UTI_FloatNetworkToHost(UTI_FloatHostToNetwork(x));
    TEST_CHECK(y / x > 0.99999 && y / x < 1.00001);
  }

  for (i = 0; i < 100000; i++) {
    x = TST_GetRandomDouble(-1000.0, 1000.0);
    y = UTI_FloatNetworkToHost(UTI_FloatHostToNetwork(x));
    TEST_CHECK(y / x > 0.99999 && y / x < 1.00001);

    UTI_GetRandomBytes(&f, sizeof (f));
    x = UTI_FloatNetworkToHost(f);
    TEST_CHECK(x > 0.0 || x <= 0.0);
  }

  TEST_CHECK(UTI_DoubleToNtp32(1.0) == htonl(65536));
  TEST_CHECK(UTI_DoubleToNtp32(0.0) == htonl(0));
  TEST_CHECK(UTI_DoubleToNtp32(1.0 / (65536.0)) == htonl(1));
  TEST_CHECK(UTI_DoubleToNtp32(1.000001 / (65536.0)) == htonl(2));
  TEST_CHECK(UTI_DoubleToNtp32(1.000001) == htonl(65537));
  TEST_CHECK(UTI_DoubleToNtp32(1000000) == htonl(0xffffffff));
  TEST_CHECK(UTI_DoubleToNtp32(-1.0) == htonl(0));

  UTI_DoubleToTimeval(0.4e-6, &tv);
  TEST_CHECK(tv.tv_sec == 0);
  TEST_CHECK(tv.tv_usec == 0);
  UTI_DoubleToTimeval(-0.4e-6, &tv);
  TEST_CHECK(tv.tv_sec == 0);
  TEST_CHECK(tv.tv_usec == 0);
  UTI_DoubleToTimeval(0.5e-6, &tv);
  TEST_CHECK(tv.tv_sec == 0);
  TEST_CHECK(tv.tv_usec == 1);
  UTI_DoubleToTimeval(-0.5e-6, &tv);
  TEST_CHECK(tv.tv_sec == -1);
  TEST_CHECK(tv.tv_usec == 999999);

  UTI_DoubleToTimespec(0.9e-9, &ts);
  TEST_CHECK(ts.tv_sec == 0);
  TEST_CHECK(ts.tv_nsec == 0);
  UTI_DoubleToTimespec(1.0e-9, &ts);
  TEST_CHECK(ts.tv_sec == 0);
  TEST_CHECK(ts.tv_nsec == 1);
  UTI_DoubleToTimespec(-0.9e-9, &ts);
  TEST_CHECK(ts.tv_sec == 0);
  TEST_CHECK(ts.tv_nsec == 0);
  UTI_DoubleToTimespec(-1.0e-9, &ts);
  TEST_CHECK(ts.tv_sec == -1);
  TEST_CHECK(ts.tv_nsec == 999999999);

  ntp_ts.hi = htonl(JAN_1970);
  ntp_ts.lo = 0xffffffff;
  UTI_Ntp64ToTimespec(&ntp_ts, &ts);
#if defined(HAVE_LONG_TIME_T) && NTP_ERA_SPLIT > 0
  TEST_CHECK(ts.tv_sec == 0x100000000LL * (1 + (NTP_ERA_SPLIT - 1) / 0x100000000LL));
#else
  TEST_CHECK(ts.tv_sec == 0);
#endif
  TEST_CHECK(ts.tv_nsec == 999999999);

  ntp_ts.hi = htonl(JAN_1970 - 1);
  ntp_ts.lo = htonl(0xffffffff);
  ntp_ts2.hi = htonl(JAN_1970 + 1);
  ntp_ts2.lo = htonl(0x80000000);
  TEST_CHECK(fabs(UTI_DiffNtp64ToDouble(&ntp_ts, &ntp_ts2) + 1.5) < 1e-9);
  TEST_CHECK(fabs(UTI_DiffNtp64ToDouble(&ntp_ts2, &ntp_ts) - 1.5) < 1e-9);

  UTI_AddDoubleToTimespec(&ts, 1e-9, &ts);
#if defined(HAVE_LONG_TIME_T) && NTP_ERA_SPLIT > 0
  TEST_CHECK(ts.tv_sec == 1 + 0x100000000LL * (1 + (NTP_ERA_SPLIT - 1) / 0x100000000LL));
#else
  TEST_CHECK(ts.tv_sec == 1);
#endif
  TEST_CHECK(ts.tv_nsec == 0);

  ntp_fuzz.hi = 0;
  ntp_fuzz.lo = htonl(0xff1234ff);

  UTI_TimespecToNtp64(&ts, &ntp_ts, &ntp_fuzz);
  TEST_CHECK(ntp_ts.hi == htonl(JAN_1970 + 1));
  TEST_CHECK(ntp_ts.lo == ntp_fuzz.lo);

  ts.tv_sec = ts.tv_nsec = 1;
  UTI_ZeroTimespec(&ts);
  TEST_CHECK(ts.tv_sec == 0);
  TEST_CHECK(ts.tv_nsec == 0);
  TEST_CHECK(UTI_IsZeroTimespec(&ts));

  ntp_ts.hi = ntp_ts.lo == 1;
  UTI_ZeroNtp64(&ntp_ts);
  TEST_CHECK(ntp_ts.hi == 0);
  TEST_CHECK(ntp_ts.lo == 0);

  tv.tv_sec = tv.tv_usec = 1;
  UTI_TimevalToTimespec(&tv, &ts);
  TEST_CHECK(ts.tv_sec == 1);
  TEST_CHECK(ts.tv_nsec == 1000);

  UTI_TimespecToTimeval(&ts, &tv);
  TEST_CHECK(tv.tv_sec == 1);
  TEST_CHECK(tv.tv_usec == 1);

  ts.tv_sec = 1;
  ts.tv_nsec = 500000000;
  TEST_CHECK(fabs(UTI_TimespecToDouble(&ts) - 1.5) < 1.0e-15);

  UTI_DoubleToTimespec(2.75, &ts);
  TEST_CHECK(ts.tv_sec == 2);
  TEST_CHECK(ts.tv_nsec == 750000000);

  ts.tv_sec = 1;
  ts.tv_nsec = 1200000000;
  UTI_NormaliseTimespec(&ts);
  TEST_CHECK(ts.tv_sec == 2);
  TEST_CHECK(ts.tv_nsec == 200000000);

  ts.tv_sec = 1;
  ts.tv_nsec = -200000000;
  UTI_NormaliseTimespec(&ts);
  TEST_CHECK(ts.tv_sec == 0);
  TEST_CHECK(ts.tv_nsec == 800000000);

  tv.tv_sec = 1;
  tv.tv_usec = 500000;
  TEST_CHECK(fabs(UTI_TimevalToDouble(&tv) - 1.5) < 1.0e-15);

  UTI_DoubleToTimeval(2.75, &tv);
  TEST_CHECK(tv.tv_sec == 2);
  TEST_CHECK(tv.tv_usec == 750000);

  tv.tv_sec = 1;
  tv.tv_usec = 1200000;
  UTI_NormaliseTimeval(&tv);
  TEST_CHECK(tv.tv_sec == 2);
  TEST_CHECK(tv.tv_usec == 200000);

  tv.tv_sec = 1;
  tv.tv_usec = -200000;
  UTI_NormaliseTimeval(&tv);
  TEST_CHECK(tv.tv_sec == 0);
  TEST_CHECK(tv.tv_usec == 800000);

  UTI_ZeroTimespec(&ts);
  UTI_TimespecToNtp64(&ts, &ntp_ts, &ntp_fuzz);
  TEST_CHECK(ntp_ts.hi == 0);
  TEST_CHECK(ntp_ts.lo == 0);

  TEST_CHECK(UTI_IsZeroNtp64(&ntp_ts));

  ts.tv_sec = 1;
  ntp_ts.hi = htonl(1);

  TEST_CHECK(!UTI_IsZeroTimespec(&ts));
  TEST_CHECK(!UTI_IsZeroNtp64(&ntp_ts));

  ts.tv_sec = 0;
  ntp_ts.hi = 0;
  ts.tv_nsec = 1;
  ntp_ts.lo = htonl(1);

  TEST_CHECK(!UTI_IsZeroTimespec(&ts));
  TEST_CHECK(!UTI_IsZeroNtp64(&ntp_ts));

  ntp_ts.hi = 0;
  ntp_ts.lo = 0;

  UTI_Ntp64ToTimespec(&ntp_ts, &ts);
  TEST_CHECK(UTI_IsZeroTimespec(&ts));
  UTI_TimespecToNtp64(&ts, &ntp_ts, NULL);
  TEST_CHECK(UTI_IsZeroNtp64(&ntp_ts));

  ntp_fuzz.hi = htonl(1);
  ntp_fuzz.lo = htonl(3);
  ntp_ts.hi = htonl(1);
  ntp_ts.lo = htonl(2);

  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_ts) == 0);
  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_fuzz) < 0);
  TEST_CHECK(UTI_CompareNtp64(&ntp_fuzz, &ntp_ts) > 0);

  ntp_ts.hi = htonl(0x80000002);
  ntp_ts.lo = htonl(2);

  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_ts) == 0);
  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_fuzz) < 0);
  TEST_CHECK(UTI_CompareNtp64(&ntp_fuzz, &ntp_ts) > 0);

  ntp_fuzz.hi = htonl(0x90000001);

  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_ts) == 0);
  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_fuzz) < 0);
  TEST_CHECK(UTI_CompareNtp64(&ntp_fuzz, &ntp_ts) > 0);

  TEST_CHECK(UTI_IsEqualAnyNtp64(&ntp_ts, &ntp_ts, NULL, NULL));
  TEST_CHECK(UTI_IsEqualAnyNtp64(&ntp_ts, NULL, &ntp_ts, NULL));
  TEST_CHECK(UTI_IsEqualAnyNtp64(&ntp_ts, NULL, NULL, &ntp_ts));
  TEST_CHECK(!UTI_IsEqualAnyNtp64(&ntp_ts, &ntp_fuzz, &ntp_fuzz, &ntp_fuzz));

  ntp_ts.hi = htonl(0);
  ntp_ts.lo = htonl(0);
  x = UTI_Ntp64ToDouble(&ntp_ts);
  TEST_CHECK(fabs(x) < 1e-10);
  UTI_DoubleToNtp64(x, &ntp_ts2);
  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_ts2) == 0);

  ntp_ts.hi = htonl(0);
  ntp_ts.lo = htonl(0xffffffff);
  x = UTI_Ntp64ToDouble(&ntp_ts);
  TEST_CHECK(fabs(x - 1.0 + 0.23e-9) < 1e-10);
  UTI_DoubleToNtp64(x, &ntp_ts2);
  TEST_CHECK(fabs(UTI_DiffNtp64ToDouble(&ntp_ts, &ntp_ts2)) < 0.3e-9);

  ntp_ts.hi = htonl(0xffffffff);
  ntp_ts.lo = htonl(0xffffffff);
  x = UTI_Ntp64ToDouble(&ntp_ts);
  TEST_CHECK(fabs(x + 0.23e-9) < 1e-10);
  UTI_DoubleToNtp64(x, &ntp_ts2);
  TEST_CHECK(fabs(UTI_DiffNtp64ToDouble(&ntp_ts, &ntp_ts2)) < 0.3e-9);

  ntp_ts.hi = htonl(0x80000000);
  ntp_ts.lo = htonl(0);
  x = UTI_Ntp64ToDouble(&ntp_ts);
  TEST_CHECK(fabs(x + 0x80000000) < 1e-10);
  UTI_DoubleToNtp64(x, &ntp_ts2);
  TEST_CHECK(fabs(UTI_DiffNtp64ToDouble(&ntp_ts, &ntp_ts2)) < 0.3e-9);

  ntp_ts.hi = htonl(0x7fffffff);
  ntp_ts.lo = htonl(0xffffffff);
  x = UTI_Ntp64ToDouble(&ntp_ts);
  TEST_CHECK(fabs(x - 2147483648) < 1.0);

  ntp_ts.lo = htonl(0);
  ntp_ts.hi = htonl(0x7fffffff);
  UTI_DoubleToNtp64(0x7fffffff + 0.1, &ntp_ts2);
  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_ts2) == 0);
  ntp_ts.hi = htonl(0x80000000);
  UTI_DoubleToNtp64(0x80000000 - 0.1, &ntp_ts);
  TEST_CHECK(UTI_CompareNtp64(&ntp_ts, &ntp_ts2) == 0);

  ts.tv_sec = 1;
  ts.tv_nsec = 2;
  ts2.tv_sec = 1;
  ts2.tv_nsec = 3;

  TEST_CHECK(UTI_CompareTimespecs(&ts, &ts) == 0);
  TEST_CHECK(UTI_CompareTimespecs(&ts, &ts2) < 0);
  TEST_CHECK(UTI_CompareTimespecs(&ts2, &ts) > 0);

  ts2.tv_sec = 2;

  TEST_CHECK(UTI_CompareTimespecs(&ts, &ts) == 0);
  TEST_CHECK(UTI_CompareTimespecs(&ts, &ts2) < 0);
  TEST_CHECK(UTI_CompareTimespecs(&ts2, &ts) > 0);

  ts.tv_sec = 2;
  ts.tv_nsec = 250000000;
  ts2.tv_sec = 1;
  ts2.tv_nsec = 750000000;
  UTI_DiffTimespecs(&ts3, &ts, &ts2);
  TEST_CHECK(ts3.tv_sec == 0);
  TEST_CHECK(ts3.tv_nsec == 500000000);
  TEST_CHECK(fabs(UTI_DiffTimespecsToDouble(&ts, &ts2) - 0.5) < 1.0e-15);

  ts.tv_sec = 2;
  ts.tv_nsec = 250000000;
  ts2.tv_sec = 3;
  ts2.tv_nsec = 750000000;
  UTI_DiffTimespecs(&ts3, &ts, &ts2);
  TEST_CHECK(ts3.tv_sec == -2);
  TEST_CHECK(ts3.tv_nsec == 500000000);
  TEST_CHECK(fabs(UTI_DiffTimespecsToDouble(&ts, &ts2) - -1.5) < 1.0e-15);

  ts.tv_sec = 2;
  ts.tv_nsec = 250000000;
  UTI_AddDoubleToTimespec(&ts, 2.5, &ts2);
  TEST_CHECK(ts2.tv_sec == 4);
  TEST_CHECK(ts2.tv_nsec == 750000000);

  ts.tv_sec = 4;
  ts.tv_nsec = 500000000;
  ts2.tv_sec = 1;
  ts2.tv_nsec = 750000000;
  UTI_AverageDiffTimespecs(&ts, &ts2, &ts3, &x);
  TEST_CHECK(ts3.tv_sec == 3);
  TEST_CHECK(ts3.tv_nsec == 125000000);
  TEST_CHECK(x == -2.75);

  ts.tv_sec = 4;
  ts.tv_nsec = 500000000;
  ts2.tv_sec = 1;
  ts2.tv_nsec = 750000000;
  ts3.tv_sec = 5;
  ts3.tv_nsec = 250000000;
  UTI_AddDiffToTimespec(&ts, &ts2, &ts3, &ts4);
  TEST_CHECK(ts4.tv_sec == 8);
  TEST_CHECK(ts4.tv_nsec == 0);

  ts.tv_sec = 1600000000;
  ts.tv_nsec = 123;
  s = UTI_TimespecToString(&ts);
  TEST_CHECK(strcmp(s, "1600000000.000000123") == 0);

  ntp_ts.hi = 1;
  ntp_ts.hi = 2;
  UTI_Ntp64ToTimespec(&ntp_ts, &ts);
  s = UTI_Ntp64ToString(&ntp_ts);
  s2 = UTI_TimespecToString(&ts);
  TEST_CHECK(strcmp(s, s2) == 0);

  s = UTI_RefidToString(0x41424344);
  TEST_CHECK(strcmp(s, "ABCD") == 0);

  ip.family = IPADDR_UNSPEC;
  s = UTI_IPToString(&ip);
  TEST_CHECK(strcmp(s, "[UNSPEC]") == 0);
  TEST_CHECK(UTI_IPToRefid(&ip) == 0);
  TEST_CHECK(UTI_IPToHash(&ip) == UTI_IPToHash(&ip));

  ip.family = IPADDR_INET4;
  ip.addr.in4 = 0x7f010203;
  s = UTI_IPToString(&ip);
  TEST_CHECK(strcmp(s, "127.1.2.3") == 0);
  TEST_CHECK(UTI_IPToRefid(&ip) == 0x7f010203);
  TEST_CHECK(UTI_IPToHash(&ip) == UTI_IPToHash(&ip));

  ip.family = IPADDR_INET6;
  memset(&ip.addr.in6, 0, sizeof (ip.addr.in6));
  ip.addr.in6[0] = 0xab;
  ip.addr.in6[15] = 0xcd;
  s = UTI_IPToString(&ip);
#ifdef FEAT_IPV6
  TEST_CHECK(strcmp(s, "ab00::cd") == 0);
#else
  TEST_CHECK(strcmp(s, "ab00:0000:0000:0000:0000:0000:0000:00cd") == 0);
#endif
  TEST_CHECK(UTI_IPToRefid(&ip) == 0x5f9aa602);
  TEST_CHECK(UTI_IPToHash(&ip) == UTI_IPToHash(&ip));

  ip.family = IPADDR_ID;
  ip.addr.id = 12345;
  s = UTI_IPToString(&ip);
  TEST_CHECK(strcmp(s, "ID#0000012345") == 0);
  TEST_CHECK(UTI_IPToRefid(&ip) == 0);
  TEST_CHECK(UTI_IPToHash(&ip) == UTI_IPToHash(&ip));

  ip.family = IPADDR_UNSPEC + 10;
  s = UTI_IPToString(&ip);
  TEST_CHECK(strcmp(s, "[UNKNOWN]") == 0);
  TEST_CHECK(UTI_IPToRefid(&ip) == 0);
  TEST_CHECK(UTI_IPToHash(&ip) == UTI_IPToHash(&ip));

  TEST_CHECK(UTI_StringToIP("200.4.5.6", &ip));
  TEST_CHECK(ip.family == IPADDR_INET4);
  TEST_CHECK(ip.addr.in4 == 0xc8040506);

#ifdef FEAT_IPV6
  TEST_CHECK(UTI_StringToIP("1234::7890", &ip));
  TEST_CHECK(ip.family == IPADDR_INET6);
  TEST_CHECK(ip.addr.in6[0] == 0x12 && ip.addr.in6[1] == 0x34);
  TEST_CHECK(ip.addr.in6[2] == 0x00 && ip.addr.in6[13] == 0x00);
  TEST_CHECK(ip.addr.in6[14] == 0x78 && ip.addr.in6[15] == 0x90);
#else
  TEST_CHECK(!UTI_StringToIP("1234::7890", &ip));
#endif

  TEST_CHECK(!UTI_StringToIP("ID#0000012345", &ip));

  TEST_CHECK(UTI_IsStringIP("1.2.3.4"));
  TEST_CHECK(!UTI_IsStringIP("127.3.3"));
  TEST_CHECK(!UTI_IsStringIP("127.3"));
  TEST_CHECK(!UTI_IsStringIP("127"));
#ifdef FEAT_IPV6
  TEST_CHECK(UTI_IsStringIP("1234:5678::aaaa"));
#else
  TEST_CHECK(!UTI_IsStringIP("1234:5678::aaaa"));
#endif
  TEST_CHECK(!UTI_StringToIP("ID#0000012345", &ip));

  TEST_CHECK(!UTI_StringToIdIP("1.2.3.4", &ip));
  TEST_CHECK(UTI_StringToIdIP("ID#0000056789", &ip));
  TEST_CHECK(ip.family == IPADDR_ID);
  TEST_CHECK(ip.addr.id == 56789);

  for (i = IPADDR_UNSPEC; i <= IPADDR_ID + 1; i++) {
    ip.family = i;
    TEST_CHECK(UTI_IsIPReal(&ip) == (i == IPADDR_INET4 || i == IPADDR_INET6));
  }

  ip.family = IPADDR_UNSPEC;
  UTI_IPHostToNetwork(&ip, &ip2);
  TEST_CHECK(ip2.family == htons(IPADDR_UNSPEC));
  UTI_IPNetworkToHost(&ip2, &ip3);
  TEST_CHECK(ip3.family == IPADDR_UNSPEC);

  ip.family = IPADDR_INET4;
  ip.addr.in4 = 0x12345678;
  UTI_IPHostToNetwork(&ip, &ip2);
  TEST_CHECK(ip2.family == htons(IPADDR_INET4));
  TEST_CHECK(ip2.addr.in4 == htonl(0x12345678));
  UTI_IPNetworkToHost(&ip2, &ip3);
  TEST_CHECK(ip3.family == IPADDR_INET4);
  TEST_CHECK(ip3.addr.in4 == 0x12345678);

  ip.family = IPADDR_INET6;
  for (i = 0; i < 16; i++)
    ip.addr.in6[i] = i;
  UTI_IPHostToNetwork(&ip, &ip2);
  TEST_CHECK(ip2.family == htons(IPADDR_INET6));
  for (i = 0; i < 16; i++)
    TEST_CHECK(ip.addr.in6[i] == i);
  UTI_IPNetworkToHost(&ip2, &ip3);
  TEST_CHECK(ip3.family == IPADDR_INET6);
  for (i = 0; i < 16; i++)
    TEST_CHECK(ip.addr.in6[i] == i);

  ip.family = IPADDR_ID;
  ip.addr.in4 = 0x87654321;
  UTI_IPHostToNetwork(&ip, &ip2);
  TEST_CHECK(ip2.family == htons(IPADDR_ID));
  TEST_CHECK(ip2.addr.in4 == htonl(0x87654321));
  UTI_IPNetworkToHost(&ip2, &ip3);
  TEST_CHECK(ip3.family == IPADDR_ID);
  TEST_CHECK(ip3.addr.in4 == 0x87654321);

  for (i = 0; i < 16; i++)
    ip.addr.in6[i] = 0x80;
  ip2 = ip;

  for (i = IPADDR_UNSPEC; i <= IPADDR_ID; i++) {
    ip.family = i;
    ip2.family = i + 1;
    TEST_CHECK(UTI_CompareIPs(&ip, &ip2, NULL) < 0);
    TEST_CHECK(UTI_CompareIPs(&ip2, &ip, NULL) > 0);
    ip2 = ip;
    ip2.addr.in4++;
    if (i == IPADDR_UNSPEC) {
      TEST_CHECK(UTI_CompareIPs(&ip, &ip2, NULL) == 0);
      TEST_CHECK(UTI_CompareIPs(&ip, &ip2, &ip) == 0);
    } else {
      TEST_CHECK(UTI_CompareIPs(&ip, &ip2, NULL) < 0);
      TEST_CHECK(UTI_CompareIPs(&ip2, &ip, NULL) > 0);
      if (i == IPADDR_ID) {
        TEST_CHECK(UTI_CompareIPs(&ip, &ip2, &ip) < 0);
        TEST_CHECK(UTI_CompareIPs(&ip, &ip2, &ip2) < 0);
      } else {
        TEST_CHECK(UTI_CompareIPs(&ip, &ip2, &ip) == 0);
        TEST_CHECK(UTI_CompareIPs(&ip, &ip2, &ip2) < 0);
      }
    }
  }

  ip_saddr.ip_addr.family = IPADDR_INET4;
  ip_saddr.ip_addr.addr.in4 = 0x01020304;
  ip_saddr.port = 12345;
  s = UTI_IPSockAddrToString(&ip_saddr);
  TEST_CHECK(strcmp(s, "1.2.3.4:12345") == 0);

  ip = ip_saddr.ip_addr;
  s = UTI_IPSubnetToString(&ip, 10);
  TEST_CHECK(strcmp(s, "1.2.3.4/10") == 0);
  s = UTI_IPSubnetToString(&ip, 32);
  TEST_CHECK(strcmp(s, "1.2.3.4") == 0);
  ip.family = IPADDR_UNSPEC;
  s = UTI_IPSubnetToString(&ip, 0);
  TEST_CHECK(strcmp(s, "any address") == 0);

  s = UTI_TimeToLogForm(2000000000);
  TEST_CHECK(strcmp(s, "2033-05-18 03:33:20") == 0);

  ts.tv_sec = 3;
  ts.tv_nsec = 500000000;
  ts2.tv_sec = 4;
  ts2.tv_nsec = 250000000;
  UTI_AdjustTimespec(&ts, &ts2, &ts3, &x, 2.0, -5.0);
  TEST_CHECK(fabs(x - 6.5) < 1.0e-15);
  TEST_CHECK((ts3.tv_sec == 10 && ts3.tv_nsec == 0) ||
             (ts3.tv_sec == 9 && ts3.tv_nsec == 999999999));

  for (i = -32; i <= 32; i++) {
    for (j = c = 0; j < 1000; j++) {
      UTI_GetNtp64Fuzz(&ntp_fuzz, i);
      if (i <= 0)
        TEST_CHECK(ntp_fuzz.hi == 0);
      if (i < 0)
        TEST_CHECK(ntohl(ntp_fuzz.lo) < 1U << (32 + i));
      else if (i < 32)
        TEST_CHECK(ntohl(ntp_fuzz.hi) < 1U << i);
      if (ntohl(ntp_fuzz.lo) >= 1U << (31 + CLAMP(-31, i, 0)))
        c++;
    }

    if (i == -32)
      TEST_CHECK(c == 0);
    else
      TEST_CHECK(c > 400 && c < 600);
  }

  TEST_CHECK(UTI_DoubleToNtp32(-1.0) == htonl(0));
  TEST_CHECK(UTI_DoubleToNtp32(0.0) == htonl(0));
  TEST_CHECK(UTI_DoubleToNtp32(1e-9) == htonl(1));
  TEST_CHECK(UTI_DoubleToNtp32(32768.0) == htonl(0x80000000));
  TEST_CHECK(UTI_DoubleToNtp32(65536.0) == htonl(0xffffffff));
  TEST_CHECK(UTI_DoubleToNtp32(65537.0) == htonl(0xffffffff));

  TEST_CHECK(UTI_DoubleToNtp32f28(-1.0) == htonl(0));
  TEST_CHECK(UTI_DoubleToNtp32f28(0.0) == htonl(0));
  TEST_CHECK(UTI_DoubleToNtp32f28(1e-9) == htonl(1));
  TEST_CHECK(UTI_DoubleToNtp32f28(4e-9) == htonl(2));
  TEST_CHECK(UTI_DoubleToNtp32f28(8.0) == htonl(0x80000000));
  TEST_CHECK(UTI_DoubleToNtp32f28(16.0) == htonl(0xffffffff));
  TEST_CHECK(UTI_DoubleToNtp32f28(16.1) == htonl(0xffffffff));
  TEST_CHECK(UTI_DoubleToNtp32f28(16.1) == htonl(0xffffffff));

  TEST_CHECK(UTI_Ntp32f28ToDouble(htonl(0xffffffff)) >= 65535.999);
  for (i = 0; i < 100000; i++) {
    UTI_GetRandomBytes(&ntp32_ts, sizeof (ntp32_ts));
    TEST_CHECK(UTI_DoubleToNtp32(UTI_Ntp32ToDouble(ntp32_ts)) == ntp32_ts);
    TEST_CHECK(UTI_DoubleToNtp32f28(UTI_Ntp32f28ToDouble(ntp32_ts)) == ntp32_ts);
  }

  ts.tv_nsec = 0;

  ts.tv_sec = 10;
  TEST_CHECK(!UTI_IsTimeOffsetSane(&ts, -20.0));

#ifdef HAVE_LONG_TIME_T
  ts.tv_sec = NTP_ERA_SPLIT + (1LL << 32);
#else
  ts.tv_sec = 0x7fffffff - MIN_ENDOFTIME_DISTANCE;
#endif
  TEST_CHECK(!UTI_IsTimeOffsetSane(&ts, 10.0));
  TEST_CHECK(UTI_IsTimeOffsetSane(&ts, -20.0));

  TEST_CHECK(UTI_Log2ToDouble(-1) == 0.5);
  TEST_CHECK(UTI_Log2ToDouble(0) == 1.0);
  TEST_CHECK(UTI_Log2ToDouble(1) == 2.0);
  TEST_CHECK(UTI_Log2ToDouble(-31) < UTI_Log2ToDouble(-30));
  TEST_CHECK(UTI_Log2ToDouble(-32) == UTI_Log2ToDouble(-31));
  TEST_CHECK(UTI_Log2ToDouble(30) < UTI_Log2ToDouble(32));
  TEST_CHECK(UTI_Log2ToDouble(31) == UTI_Log2ToDouble(32));

  UTI_TimespecHostToNetwork(&ts, &tspec);
#ifdef HAVE_LONG_TIME_T
  TEST_CHECK(tspec.tv_sec_high == htonl(ts.tv_sec >> 32));
#else
  TEST_CHECK(tspec.tv_sec_high == htonl(TV_NOHIGHSEC));
#endif
  TEST_CHECK(tspec.tv_sec_low == htonl(ts.tv_sec));
  TEST_CHECK(tspec.tv_nsec == htonl(ts.tv_nsec));
  UTI_TimespecNetworkToHost(&tspec, &ts2);
  TEST_CHECK(!UTI_CompareTimespecs(&ts, &ts2));

  integer64 = UTI_Integer64HostToNetwork(0x1234567890ABCDEFULL);
  TEST_CHECK(memcmp(&integer64, "\x12\x34\x56\x78\x90\xab\xcd\xef", 8) == 0);
  TEST_CHECK(UTI_Integer64NetworkToHost(integer64) == 0x1234567890ABCDEFULL);

  TEST_CHECK(UTI_CmacNameToAlgorithm("AES128") == CMC_AES128);
  TEST_CHECK(UTI_CmacNameToAlgorithm("AES256") == CMC_AES256);
  TEST_CHECK(UTI_CmacNameToAlgorithm("NOSUCHCMAC") == CMC_INVALID);

  TEST_CHECK(UTI_HashNameToAlgorithm("MD5") == HSH_MD5);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA1") == HSH_SHA1);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA256") == HSH_SHA256);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA384") == HSH_SHA384);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA512") == HSH_SHA512);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA3-224") == HSH_SHA3_224);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA3-256") == HSH_SHA3_256);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA3-384") == HSH_SHA3_384);
  TEST_CHECK(UTI_HashNameToAlgorithm("SHA3-512") == HSH_SHA3_512);
  TEST_CHECK(UTI_HashNameToAlgorithm("TIGER") == HSH_TIGER);
  TEST_CHECK(UTI_HashNameToAlgorithm("WHIRLPOOL") == HSH_WHIRLPOOL);
  TEST_CHECK(UTI_HashNameToAlgorithm("NOSUCHHASH") == HSH_INVALID);

  i = open("/dev/null", 0);
  TEST_CHECK(UTI_FdSetCloexec(i));
  j = fcntl(i, F_GETFD);
  TEST_CHECK(j & F_GETFD);
  close(i);

  UTI_SetQuitSignalsHandler(handle_signal, 0);
  TEST_CHECK(handled_signal == 0);
  kill(getpid(), SIGPIPE);
  while (handled_signal == 0)
    ;
  TEST_CHECK(handled_signal == SIGPIPE);

  s = UTI_PathToDir("/aaa/bbb/ccc/ddd");
  TEST_CHECK(!strcmp(s, "/aaa/bbb/ccc"));
  Free(s);
  s = UTI_PathToDir("aaa");
  TEST_CHECK(!strcmp(s, "."));
  Free(s);
  s = UTI_PathToDir("/aaaa");
  TEST_CHECK(!strcmp(s, "/"));
  Free(s);

  nan = strtod("nan", NULL);
  inf = strtod("inf", NULL);

  TEST_CHECK(MIN(2.0, -1.0) == -1.0);
  TEST_CHECK(MIN(-1.0, 2.0) == -1.0);
  TEST_CHECK(MIN(inf, 2.0) == 2.0);

  TEST_CHECK(MAX(2.0, -1.0) == 2.0);
  TEST_CHECK(MAX(-1.0, 2.0) == 2.0);
  TEST_CHECK(MAX(inf, 2.0) == inf);

  TEST_CHECK(CLAMP(1.0, -1.0, 2.0) == 1.0);
  TEST_CHECK(CLAMP(1.0, 3.0, 2.0) == 2.0);
  TEST_CHECK(CLAMP(1.0, inf, 2.0) == 2.0);
  TEST_CHECK(CLAMP(1.0, nan, 2.0) == 2.0);

  TEST_CHECK(SQUARE(3.0) == 3.0 * 3.0);

  rmdir("testdir");

  uid = geteuid();
  gid = getegid();

  TEST_CHECK(UTI_CreateDirAndParents("testdir", 0700, uid, gid));

  TEST_CHECK(UTI_CheckDirPermissions("testdir", 0700, uid, gid));
  TEST_CHECK(!UTI_CheckDirPermissions("testdir", 0300, uid, gid));
  TEST_CHECK(!UTI_CheckDirPermissions("testdir", 0700, uid + 1, gid));
  TEST_CHECK(!UTI_CheckDirPermissions("testdir", 0700, uid, gid + 1));

  umask(0);

  unlink("testfile");
  file = UTI_OpenFile(NULL, "testfile", NULL, 'r', 0);
  TEST_CHECK(!file);
  TEST_CHECK(stat("testfile", &st) < 0);

  file = UTI_OpenFile(NULL, "testfile", NULL, 'w', 0644);
  TEST_CHECK(file);
  TEST_CHECK(stat("testfile", &st) == 0);
  TEST_CHECK((st.st_mode & 0777) == 0644);
  fclose(file);

  file = UTI_OpenFile(".", "test", "file", 'W', 0640);
  TEST_CHECK(file);
  TEST_CHECK(stat("testfile", &st) == 0);
  TEST_CHECK((st.st_mode & 0777) == 0640);
  fclose(file);

  file = UTI_OpenFile(NULL, "test", "file", 'r', 0);
  TEST_CHECK(file);
  fclose(file);

  TEST_CHECK(UTI_RenameTempFile(NULL, "testfil", "e", NULL));
  TEST_CHECK(stat("testfil", &st) == 0);
  file = UTI_OpenFile(NULL, "testfil", NULL, 'R', 0);
  TEST_CHECK(file);
  fclose(file);

  TEST_CHECK(UTI_RenameTempFile(NULL, "test", "fil", "file"));
  TEST_CHECK(stat("testfile", &st) == 0);
  file = UTI_OpenFile(NULL, "testfile", NULL, 'R', 0);
  TEST_CHECK(file);
  fclose(file);

  TEST_CHECK(UTI_RemoveFile(NULL, "testfile", NULL));
  TEST_CHECK(stat("testfile", &st) < 0);
  TEST_CHECK(!UTI_RemoveFile(NULL, "testfile", NULL));

  for (i = c = 0; i < 100000; i++) {
    j = random() % (sizeof (buf) + 1);
    UTI_GetRandomBytesUrandom(buf, j);
    if (j && buf[j - 1] % 2)
      c++;
    if (random() % 10000 == 0) {
      UTI_ResetGetRandomFunctions();
      TEST_CHECK(!urandom_file);
    }
  }
  TEST_CHECK(c > 46000 && c < 48000);

  for (i = c = 0; i < 100000; i++) {
    j = random() % (sizeof (buf) + 1);
    UTI_GetRandomBytes(buf, j);
    if (j && buf[j - 1] % 2)
      c++;
    if (random() % 10000 == 0) {
      UTI_ResetGetRandomFunctions();
#if HAVE_GETRANDOM
      TEST_CHECK(getrandom_buf_available == 0);
#endif
    }
  }
  TEST_CHECK(c > 46000 && c < 48000);

  assert(sizeof (buf) >= 16);
  TEST_CHECK(UTI_HexToBytes("", buf, sizeof (buf)) == 0);
  TEST_CHECK(UTI_HexToBytes("0", buf, sizeof (buf)) == 0);
  TEST_CHECK(UTI_HexToBytes("00123456789ABCDEF", buf, sizeof (buf)) == 0);
  TEST_CHECK(UTI_HexToBytes("00123456789ABCDEF0", buf, 8) == 0);
  TEST_CHECK(UTI_HexToBytes("00123456789ABCDEF0", buf, sizeof (buf)) == 9);
  TEST_CHECK(memcmp(buf, "\x00\x12\x34\x56\x78\x9A\xBC\xDE\xF0", 9) == 0);
  memcpy(buf, "AB123456780001", 15);
  TEST_CHECK(UTI_HexToBytes(buf, buf, sizeof (buf)) == 7);
  TEST_CHECK(memcmp(buf, "\xAB\x12\x34\x56\x78\x00\x01", 7) == 0);

  TEST_CHECK(UTI_BytesToHex("", 0, buf, 0) == 0);
  TEST_CHECK(UTI_BytesToHex("\xAB\x12\x34\x56\x78\x00\x01", 7, buf, 14) == 0);
  TEST_CHECK(UTI_BytesToHex("\xAB\x12\x34\x56\x78\x00\x01", 7, buf, 15) == 1);
  TEST_CHECK(strcmp(buf, "AB123456780001") == 0);
  TEST_CHECK(UTI_BytesToHex("\xAB\x12\x34\x56\x78\x00\x01", 0, buf, 15) == 1);
  TEST_CHECK(strcmp(buf, "") == 0);

  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", "") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 3) == 0);
  TEST_CHECK(!words[0]);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", "     ") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 3) == 0);
  TEST_CHECK(!words[0]);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", "a  \n ") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 3) == 1);
  TEST_CHECK(words[0] == buf + 0);
  TEST_CHECK(strcmp(words[0], "a") == 0);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", "  a  ") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 3) == 1);
  TEST_CHECK(words[0] == buf + 2);
  TEST_CHECK(strcmp(words[0], "a") == 0);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", " \n  a") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 3) == 1);
  TEST_CHECK(words[0] == buf + 4);
  TEST_CHECK(strcmp(words[0], "a") == 0);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", "a   b") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 1) == 2);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", "a   b") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 2) == 2);
  TEST_CHECK(words[0] == buf + 0);
  TEST_CHECK(words[1] == buf + 4);
  TEST_CHECK(strcmp(words[0], "a") == 0);
  TEST_CHECK(strcmp(words[1], "b") == 0);
  TEST_CHECK(snprintf(buf, sizeof (buf), "%s", " a b ") < sizeof (buf));
  TEST_CHECK(UTI_SplitString(buf, words, 3) == 2);
  TEST_CHECK(words[0] == buf + 1);
  TEST_CHECK(words[1] == buf + 3);
  TEST_CHECK(strcmp(words[0], "a") == 0);
  TEST_CHECK(strcmp(words[1], "b") == 0);

  for (i = 0; i < 1000; i++) {
    UTI_GetRandomBytes(buf, sizeof (buf));
    memcpy(buf2, buf, sizeof (buf));
    for (j = 0; j < sizeof (buf); j++)
      TEST_CHECK(UTI_IsMemoryEqual(buf, buf2, j));

    for (j = 0; j < 8 * sizeof (buf); j++) {
      buf2[j / 8] ^= 1U << j % 8;
      TEST_CHECK(!UTI_IsMemoryEqual(buf, buf2, sizeof (buf)));
      buf2[j / 8] ^= 1U << j % 8;
      TEST_CHECK(UTI_IsMemoryEqual(buf, buf2, sizeof (buf)));
    }
  }

  HSH_Finalise();
}
