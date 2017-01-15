/*
 *	BIRD Library -- IP address functions
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: IP addresses
 *
 * BIRD uses its own abstraction of IP address in order to share the same
 * code for both IPv4 and IPv6. IP addresses are represented as entities
 * of type &ip_addr which are never to be treated as numbers and instead
 * they must be manipulated using the following functions and macros.
 */

#include <stdlib.h>

#include "nest/bird.h"
#include "lib/ip.h"


int
ip6_compare(ip6_addr a, ip6_addr b)
{
  int i;
  for (i=0; i<4; i++)
    if (a.addr[i] > b.addr[i])
      return 1;
    else if (a.addr[i] < b.addr[i])
      return -1;
  return 0;
}

ip6_addr
ip6_mkmask(uint n)
{
  ip6_addr a;
  int i;

  for (i=0; i<4; i++)
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

int
ip6_masklen(ip6_addr *a)
{
  int i, j, n;

  for (i=0, n=0; i<4; i++, n+=32)
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
ip4_classify(ip4_addr ad)
{
  u32 a = _I(ad);
  u32 b = a >> 24U;

  if (b && b <= 0xdf)
  {
    if (b == 0x7f)
      return IADDR_HOST | SCOPE_HOST;
    else if ((b == 0x0a) ||
	     ((a & 0xffff0000) == 0xc0a80000) ||
	     ((a & 0xfff00000) == 0xac100000))
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

int
ip6_classify(ip6_addr *a)
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
    uint scope = (x >> 16) & 0x0f;
    switch (scope)
    {
    case 1:  return IADDR_MULTICAST | SCOPE_HOST;
    case 2:  return IADDR_MULTICAST | SCOPE_LINK;
    case 5:  return IADDR_MULTICAST | SCOPE_SITE;
    case 8:  return IADDR_MULTICAST | SCOPE_ORGANIZATION;
    case 14: return IADDR_MULTICAST | SCOPE_UNIVERSE;
    default: return IADDR_MULTICAST | SCOPE_UNDEFINED;
    }
  }

  if (!x && !a->addr[1])
  {
    u32 a2 = a->addr[2];
    u32 a3 = a->addr[3];

    if (a2 == 0 && a3 == 1)
      return IADDR_HOST | SCOPE_HOST;		/* Loopback address */
    if (a2 == 0)
      return ip4_classify(_MI4(a3));		/* IPv4 compatible addresses */
    if (a2 == 0xffff)
      return ip4_classify(_MI4(a3));		/* IPv4 mapped addresses */

    return IADDR_INVALID;
  }

  return IADDR_HOST | SCOPE_UNDEFINED;
}



/*
 *  Conversion of IPv6 address to presentation format and vice versa.
 *  Heavily inspired by routines written by Paul Vixie for the BIND project
 *  and of course by RFC 2373.
 */


char *
ip4_ntop(ip4_addr a, char *b)
{
  u32 x = _I(a);
  return b + bsprintf(b, "%d.%d.%d.%d", (x >> 24) & 0xff, (x >> 16) & 0xff, (x >> 8) & 0xff, x & 0xff);
}


char *
ip6_ntop(ip6_addr a, char *b)
{
  u16 words[8];
  int bestpos, bestlen, curpos, curlen, i;

  /* First of all, preprocess the address and find the longest run of zeros */
  bestlen = bestpos = curpos = curlen = 0;
  for (i=0; i<8; i++)
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
  if (!bestpos && ((bestlen == 5 && a.addr[2] == 0xffff) || (bestlen == 6)))
  {
    u32 x = a.addr[3];
    b += bsprintf(b, "::%s%d.%d.%d.%d",
		  a.addr[2] ? "ffff:" : "",
		  (x >> 24) & 0xff,
		  (x >> 16) & 0xff,
		  (x >> 8) & 0xff,
		  x & 0xff);
    return b;
  }

  /* Normal IPv6 formatting, compress the largest sequence of zeros */
  for (i=0; i<8; i++)
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

int
ip4_pton(const char *a, ip4_addr *o)
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
    if (((d != c) && *d) || (l > 255))
      return 0;
    ia = (ia << 8) | l;
    if (c)
      c++;
    a = c;
  }
  *o = ip4_from_u32(ia);
  return 1;
}

int
ip6_pton(const char *a, ip6_addr *o)
{
  u16 words[8];
  int i, j, k, l, hfil;
  const char *start;

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
    for (;;)
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
      ip4_addr x;
      if (!ip4_pton(start, &x))
	return 0;
      words[i++] = _I(x) >> 16;
      words[i++] = _I(x);
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
    for (i=7; i-j >= hfil; i--)
      words[i] = words[i-j];
    for (; i>=hfil; i--)
      words[i] = 0;
  }

  /* Convert the address to ip6_addr format */
  for (i=0; i<4; i++)
    o->addr[i] = (words[2*i] << 16) | words[2*i+1];

  return 1;
}


/**
 * ip_scope_text - get textual representation of address scope
 * @scope: scope (%SCOPE_xxx)
 *
 * Returns a pointer to a textual name of the scope given.
 */
char *
ip_scope_text(uint scope)
{
  static char *scope_table[] = { "host", "link", "site", "org", "univ", "undef" };

  if (scope > SCOPE_UNDEFINED)
    return "?";
  else
    return scope_table[scope];
}

ip4_addr
ip4_class_mask(ip4_addr ad)
{
  u32 m, a = _I(ad);

  if (a == 0x00000000)
    m = 0x00000000;
  else if (a < 0x80000000)
    m = 0xff000000;
  else if (a < 0xc0000000)
    m = 0xffff0000;
  else
    m = 0xffffff00;
  if (a & ~m)
    m = 0xffffffff;

  return _MI4(m);
}

#if 0
/**
 * ipa_equal - compare two IP addresses for equality
 * @x: IP address
 * @y: IP address
 *
 * ipa_equal() returns 1 if @x and @y represent the same IP address, else 0.
 */
int ipa_equal(ip_addr x, ip_addr y) { DUMMY }

/**
 * ipa_nonzero - test if an IP address is defined
 * @x: IP address
 *
 * ipa_nonzero returns 1 if @x is a defined IP address (not all bits are zero),
 * else 0.
 *
 * The undefined all-zero address is reachable as a |IPA_NONE| macro.
 */
int ipa_nonzero(ip_addr x) { DUMMY }

/**
 * ipa_and - compute bitwise and of two IP addresses
 * @x: IP address
 * @y: IP address
 *
 * This function returns a bitwise and of @x and @y. It's primarily
 * used for network masking.
 */
ip_addr ipa_and(ip_addr x, ip_addr y) { DUMMY }

/**
 * ipa_or - compute bitwise or of two IP addresses
 * @x: IP address
 * @y: IP address
 *
 * This function returns a bitwise or of @x and @y.
 */
ip_addr ipa_or(ip_addr x, ip_addr y) { DUMMY }

/**
 * ipa_xor - compute bitwise xor of two IP addresses
 * @x: IP address
 * @y: IP address
 *
 * This function returns a bitwise xor of @x and @y.
 */
ip_addr ipa_xor(ip_addr x, ip_addr y) { DUMMY }

/**
 * ipa_not - compute bitwise negation of two IP addresses
 * @x: IP address
 *
 * This function returns a bitwise negation of @x.
 */
ip_addr ipa_not(ip_addr x) { DUMMY }

/**
 * ipa_mkmask - create a netmask
 * @x: prefix length
 *
 * This function returns an &ip_addr corresponding of a netmask
 * of an address prefix of size @x.
 */
ip_addr ipa_mkmask(int x) { DUMMY }

/**
 * ipa_masklen - calculate netmask length
 * @x: IP address
 *
 * This function checks whether @x represents a valid netmask and
 * returns the size of the associate network prefix or -1 for invalid
 * mask.
 */
int ipa_masklen(ip_addr x) { DUMMY }

/**
 * ipa_hash - hash IP addresses
 * @x: IP address
 *
 * ipa_hash() returns a 16-bit hash value of the IP address @x.
 */
int ipa_hash(ip_addr x) { DUMMY }

/**
 * ipa_hton - convert IP address to network order
 * @x: IP address
 *
 * Converts the IP address @x to the network byte order.
 *
 * Beware, this is a macro and it alters the argument!
 */
void ipa_hton(ip_addr x) { DUMMY }

/**
 * ipa_ntoh - convert IP address to host order
 * @x: IP address
 *
 * Converts the IP address @x from the network byte order.
 *
 * Beware, this is a macro and it alters the argument!
 */
void ipa_ntoh(ip_addr x) { DUMMY }

/**
 * ipa_classify - classify an IP address
 * @x: IP address
 *
 * ipa_classify() returns an address class of @x, that is a bitwise or
 * of address type (%IADDR_INVALID, %IADDR_HOST, %IADDR_BROADCAST, %IADDR_MULTICAST)
 * with address scope (%SCOPE_HOST to %SCOPE_UNIVERSE) or -1 (%IADDR_INVALID)
 * for an invalid address.
 */
int ipa_classify(ip_addr x) { DUMMY }

/**
 * ip4_class_mask - guess netmask according to address class
 * @x: IPv4 address
 *
 * This function (available in IPv4 version only) returns a
 * network mask according to the address class of @x. Although
 * classful addressing is nowadays obsolete, there still live
 * routing protocols transferring no prefix lengths nor netmasks
 * and this function could be useful to them.
 */
ip4_addr ip4_class_mask(ip4_addr x) { DUMMY }

/**
 * ipa_from_u32 - convert IPv4 address to an integer
 * @x: IP address
 *
 * This function takes an IPv4 address and returns its numeric
 * representation.
 */
u32 ipa_from_u32(ip_addr x) { DUMMY }

/**
 * ipa_to_u32 - convert integer to IPv4 address
 * @x: a 32-bit integer
 *
 * ipa_to_u32() takes a numeric representation of an IPv4 address
 * and converts it to the corresponding &ip_addr.
 */
ip_addr ipa_to_u32(u32 x) { DUMMY }

/**
 * ipa_compare - compare two IP addresses for order
 * @x: IP address
 * @y: IP address
 *
 * The ipa_compare() function takes two IP addresses and returns
 * -1 if @x is less than @y in canonical ordering (lexicographical
 * order of the bit strings), 1 if @x is greater than @y and 0
 * if they are the same.
 */
int ipa_compare(ip_addr x, ip_addr y) { DUMMY }

/**
 * ipa_build6 - build an IPv6 address from parts
 * @a1: part #1
 * @a2: part #2
 * @a3: part #3
 * @a4: part #4
 *
 * ipa_build() takes @a1 to @a4 and assembles them to a single IPv6
 * address. It's used for example when a protocol wants to bind its
 * socket to a hard-wired multicast address.
 */
ip_addr ipa_build6(u32 a1, u32 a2, u32 a3, u32 a4) { DUMMY }

/**
 * ip_ntop - convert IP address to textual representation
 * @a: IP address
 * @buf: buffer of size at least %STD_ADDRESS_P_LENGTH
 *
 * This function takes an IP address and creates its textual
 * representation for presenting to the user.
 */
char *ip_ntop(ip_addr a, char *buf) { DUMMY }

/**
 * ip_ntox - convert IP address to hexadecimal representation
 * @a: IP address
 * @buf: buffer of size at least %STD_ADDRESS_P_LENGTH
 *
 * This function takes an IP address and creates its hexadecimal
 * textual representation. Primary use: debugging dumps.
 */
char *ip_ntox(ip_addr a, char *buf) { DUMMY }

/**
 * ip_pton - parse textual representation of IP address
 * @a: textual representation
 * @o: where to put the resulting address
 *
 * This function parses a textual IP address representation and
 * stores the decoded address to a variable pointed to by @o.
 * Returns 0 if a parse error has occurred, else 0.
 */
int ip_pton(char *a, ip_addr *o) { DUMMY }

#endif
