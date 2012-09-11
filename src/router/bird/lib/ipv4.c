/*
 *	BIRD Library -- IPv4 Address Manipulation Functions
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdlib.h>

#include "nest/bird.h"
#include "lib/ip.h"
#include "lib/string.h"

int
ipv4_classify(u32 a)
{
  u32 b = a >> 24U;

  if (b && b <= 0xdf)
    {
      if (b == 0x7f)
	return IADDR_HOST | SCOPE_HOST;
      else if (b == 0x0a ||
	       (a & 0xffff0000) == 0xc0a80000 ||
	       (a & 0xfff00000) == 0xac100000)
	return IADDR_HOST | SCOPE_SITE;
      else
	return IADDR_HOST | SCOPE_UNIVERSE;
    }
  if (b >= 0xe0 && b <= 0xef)
    return IADDR_MULTICAST | SCOPE_UNIVERSE;
  if (a == 0xffffffff)
    return IADDR_BROADCAST | SCOPE_LINK;
  return IADDR_INVALID;
}

char *
ip_ntop(ip_addr a, char *b)
{
  u32 x = _I(a);

  return b + bsprintf(b, "%d.%d.%d.%d",
		      ((x >> 24) & 0xff),
		      ((x >> 16) & 0xff),
		      ((x >> 8) & 0xff),
		      (x & 0xff));
}

char *
ip_ntox(ip_addr a, char *b)
{
  return b + bsprintf(b, "%08x", _I(a));
}

u32
ipv4_class_mask(u32 a)
{
	u32 m;

	if (a < 0x80000000)
		m = 0xff000000;
	else if (a < 0xc0000000)
		m = 0xffff0000;
	else
		m = 0xffffff00;
	while (a & ~m)
		m |= m >> 1;
	return m;
}

int
ip_pton(char *a, ip_addr *o)
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
  *o = ipa_from_u32(ia);
  return 1;
}

byte *
ipv4_skip_header(byte *pkt, int *len)
{
  int l = *len;
  int q;

  if (l < 20 || (*pkt & 0xf0) != 0x40)
    return NULL;
  q = (*pkt & 0x0f) * 4;
  if (q > l)
    return NULL;
  *len -= q;
  return pkt + q;
}
