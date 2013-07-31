/*
 *	BIRD Library -- IPv6 Address Manipulation Functions
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdlib.h>

#include "nest/bird.h"
#include "lib/ip.h"
#include "lib/bitops.h"
#include "lib/endian.h"
#include "lib/string.h"

/*
 *  See RFC 2373 for explanation of IPv6 addressing issues.
 */

ip_addr
ipv6_mkmask(unsigned n)
{
  ip_addr a;
  int i;

  for(i=0; i<4; i++)
    {
      if (!n)
	a.addr[i] = 0;
      else if (n >= 32)
	{
	  a.addr[i] = ~0;
	  n -= 32;
	}
      else
	{
	  a.addr[i] = u32_mkmask(n);
	  n = 0;
	}
    }
  return a;
}

unsigned
ipv6_mklen(ip_addr *a)
{
  int i, j, n;

  for(i=0, n=0; i<4; i++, n+=32)
    if (a->addr[i] != ~0U)
      {
	j = u32_masklen(a->addr[i]);
	if (j < 0)
	  return j;
	n += j;
	while (++i < 4)
	  if (a->addr[i])
	    return -1;
	break;
      }
  return n;
}

int
ipv6_classify(ip_addr *a)
{
  u32 x = a->addr[0];

  if ((x & 0xe0000000) == 0x20000000)		/* 2000::/3  Aggregatable Global Unicast Address */
    return IADDR_HOST | SCOPE_UNIVERSE;
  if ((x & 0xffc00000) == 0xfe800000)		/* fe80::/10 Link-Local Address */
    return IADDR_HOST | SCOPE_LINK;
  if ((x & 0xffc00000) == 0xfec00000)		/* fec0::/10 Site-Local Address */
    return IADDR_HOST | SCOPE_SITE;
  if ((x & 0xfe000000) == 0xfc000000)		/* fc00::/7  Unique Local Unicast Address (RFC 4193) */
    return IADDR_HOST | SCOPE_SITE;
  if ((x & 0xff000000) == 0xff000000)		/* ff00::/8  Multicast Address */
    {
      unsigned int scope = (x >> 16) & 0x0f;
      switch (scope)
	{
	case 1:	 return IADDR_MULTICAST | SCOPE_HOST;
	case 2:  return IADDR_MULTICAST | SCOPE_LINK;
	case 5:  return IADDR_MULTICAST | SCOPE_SITE;
	case 8:  return IADDR_MULTICAST | SCOPE_ORGANIZATION;
	case 14: return IADDR_MULTICAST | SCOPE_UNIVERSE;
	default: return IADDR_MULTICAST | SCOPE_UNDEFINED;
	}
    }
  if (!x && !a->addr[1] && !a->addr[2])
    {
      u32 y = a->addr[3];
      if (y == 1)
	return IADDR_HOST | SCOPE_HOST;		/* Loopback address */
      /* IPv4 compatible addresses */
      if (y >= 0x7f000000 && y < 0x80000000)
	return IADDR_HOST | SCOPE_HOST;
      if ((y & 0xff000000) == 0x0a000000 ||
	  (y & 0xffff0000) == 0xc0a80000 ||
	  (y & 0xfff00000) == 0xac100000)
	return IADDR_HOST | SCOPE_SITE;
      if (y >= 0x01000000 && y < 0xe0000000)
	return IADDR_HOST | SCOPE_UNIVERSE;
    }
  return IADDR_HOST | SCOPE_UNDEFINED;
}

void
ipv6_hton(ip_addr *a)
{
  int i;

  for(i=0; i<4; i++)
    a->addr[i] = htonl(a->addr[i]);
}

void
ipv6_ntoh(ip_addr *a)
{
  int i;

  for(i=0; i<4; i++)
    a->addr[i] = ntohl(a->addr[i]);
}

int
ipv6_compare(ip_addr X, ip_addr Y)
{
  int i;
  ip_addr *x = &X;
  ip_addr *y = &Y;

  for(i=0; i<4; i++)
    if (x->addr[i] > y->addr[i])
      return 1;
    else if (x->addr[i] < y->addr[i])
      return -1;
  return 0;
}

/*
 *  Conversion of IPv6 address to presentation format and vice versa.
 *  Heavily inspired by routines written by Paul Vixie for the BIND project
 *  and of course by RFC 2373.
 */

char *
ip_ntop(ip_addr a, char *b)
{
  u16 words[8];
  int bestpos, bestlen, curpos, curlen, i;

  /* First of all, preprocess the address and find the longest run of zeros */
  bestlen = bestpos = curpos = curlen = 0;
  for(i=0; i<8; i++)
    {
      u32 x = a.addr[i/2];
      words[i] = ((i%2) ? x : (x >> 16)) & 0xffff;
      if (words[i])
	curlen = 0;
      else
	{
	  if (!curlen)
	    curpos = i;
	  curlen++;
	  if (curlen > bestlen)
	    {
	      bestpos = curpos;
	      bestlen = curlen;
	    }
	}
    }
  if (bestlen < 2)
    bestpos = -1;

  /* Is it an encapsulated IPv4 address? */
  if (!bestpos &&
      (bestlen == 5 && a.addr[2] == 0xffff ||
       bestlen == 6))
    {
      u32 x = a.addr[3];
      b += bsprintf(b, "::%s%d.%d.%d.%d",
		    a.addr[2] ? "ffff:" : "",
		    ((x >> 24) & 0xff),
		    ((x >> 16) & 0xff),
		    ((x >> 8) & 0xff),
		    (x & 0xff));
      return b;
    }

  /* Normal IPv6 formatting, compress the largest sequence of zeros */
  for(i=0; i<8; i++)
    {
      if (i == bestpos)
	{
	  i += bestlen - 1;
	  *b++ = ':';
	  if (i == 7)
	    *b++ = ':';
	}
      else
	{
	  if (i)
	    *b++ = ':';
	  b += bsprintf(b, "%x", words[i]);
	}
    }
  *b = 0;
  return b;
}

char *
ip_ntox(ip_addr a, char *b)
{
  int i;

  for(i=0; i<4; i++)
    {
      if (i)
	*b++ = '.';
      b += bsprintf(b, "%08x", a.addr[i]);
    }
  return b;
}

int
ipv4_pton_u32(char *a, u32 *o)
{
  int i;
  unsigned long int l;
  u32 ia = 0;

  i=4;
  while (i--)
    {
      char *d, *c = strchr(a, '.');
      if (!c != !i)
	return 0;
      l = strtoul(a, &d, 10);
      if (d != c && *d || l > 255)
	return 0;
      ia = (ia << 8) | l;
      if (c)
	c++;
      a = c;
    }
  *o = ia;
  return 1;
}

int
ip_pton(char *a, ip_addr *o)
{
  u16 words[8];
  int i, j, k, l, hfil;
  char *start;

  if (a[0] == ':')			/* Leading :: */
    {
      if (a[1] != ':')
	return 0;
      a++;
    }
  hfil = -1;
  i = 0;
  while (*a)
    {
      if (*a == ':')			/* :: */
	{
	  if (hfil >= 0)
	    return 0;
	  hfil = i;
	  a++;
	  continue;
	}
      j = 0;
      l = 0;
      start = a;
      for(;;)
	{
	  if (*a >= '0' && *a <= '9')
	    k = *a++ - '0';
	  else if (*a >= 'A' && *a <= 'F')
	    k = *a++ - 'A' + 10;
	  else if (*a >= 'a' && *a <= 'f')
	    k = *a++ - 'a' + 10;
	  else
	    break;
	  j = (j << 4) + k;
	  if (j >= 0x10000 || ++l > 4)
	    return 0;
	}
      if (*a == ':' && a[1])
	a++;
      else if (*a == '.' && (i == 6 || i < 6 && hfil >= 0))
	{				/* Embedded IPv4 address */
	  u32 x;
	  if (!ipv4_pton_u32(start, &x))
	    return 0;
	  words[i++] = x >> 16;
	  words[i++] = x;
	  break;
	}
      else if (*a)
	return 0;
      if (i >= 8)
	return 0;
      words[i++] = j;
    }

  /* Replace :: with an appropriate number of zeros */
  if (hfil >= 0)
    {
      j = 8 - i;
      for(i=7; i-j >= hfil; i--)
	words[i] = words[i-j];
      for(; i>=hfil; i--)
	words[i] = 0;
    }

  /* Convert the address to ip_addr format */
  for(i=0; i<4; i++)
    o->addr[i] = (words[2*i] << 16) | words[2*i+1];
  return 1;
}

void ipv6_absolutize(ip_addr *a, ip_addr *ifa)
{
  if ((a->addr[0] & 0xffc00000) == 0xfe800000 &&	/* a is link-scope */
      ((ifa->addr[0] & 0xe0000000) == 0x20000000 |	/* ifa is AGU ... */
       (ifa->addr[0] & 0xffc00000) == 0xfec00000))	/* ... or site-scope */
    {
      a->addr[0] = ifa->addr[0];	/* Copy the prefix, leave interface ID */
      a->addr[1] = ifa->addr[1];
    }
}

#ifdef TEST

#include "bitops.c"

static void test(char *x)
{
  ip_addr a;
  char c[STD_ADDRESS_P_LENGTH+1];

  printf("%-40s ", x);
  if (!ip_pton(x, &a))
    {
      puts("BAD");
      return;
    }
  ip_ntop(a, c);
  printf("%-40s %04x\n", c, ipv6_classify(&a));
}

int main(void)
{
  puts("Positive tests:");
  test("1:2:3:4:5:6:7:8");
  test("dead:beef:DEAD:BEEF::f00d");
  test("::");
  test("::1");
  test("1::");
  test("::1.234.5.6");
  test("::ffff:1.234.5.6");
  test("::fffe:1.234.5.6");
  test("1:2:3:4:5:6:7::8");
  test("2080::8:800:200c:417a");
  test("ff01::101");

  puts("Negative tests:");
  test(":::");
  test("1:2:3:4:5:6:7:8:");
  test("1::2::3");
  test("::12345");
  test("::1.2.3.4:5");
  test(":1:2:3:4:5:6:7:8");
  test("g:1:2:3:4:5:6:7");
  return 0;
}

#endif
