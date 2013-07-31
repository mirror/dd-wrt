/*
 *	BIRD Internet Routing Daemon -- The Internet Protocol
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_IP_H_
#define _BIRD_IP_H_

#ifndef IPV6
#include "ipv4.h"
#else
#include "ipv6.h"
#endif

#define ipa_zero(x) (!ipa_nonzero(x))
#define ip_is_prefix(a,l) (!ipa_nonzero(ipa_and(a, ipa_not(ipa_mkmask(l)))))
#define ipa_in_net(x,n,p) (ipa_zero(ipa_and(ipa_xor((n),(x)),ipa_mkmask(p))))
#define net_in_net(n1,l1,n2,l2) (((l1) >= (l2)) && (ipa_zero(ipa_and(ipa_xor((n1),(n2)),ipa_mkmask(l2)))))

/*
 *	ip_classify() returns either a negative number for invalid addresses
 *	or scope OR'ed together with address type.
 */

#define IADDR_INVALID		-1
#define IADDR_SCOPE_MASK       	0xfff
#define IADDR_HOST		0x1000
#define IADDR_BROADCAST		0x2000
#define IADDR_MULTICAST		0x4000

/*
 *	Address scope
 */

#define SCOPE_HOST 0
#define SCOPE_LINK 1
#define SCOPE_SITE 2
#define SCOPE_ORGANIZATION 3
#define SCOPE_UNIVERSE 4
#define SCOPE_UNDEFINED 5

char *ip_scope_text(unsigned);

/*
 *	Network prefixes
 */

struct prefix {
  ip_addr addr;
  unsigned int len;
};

static inline int ipa_classify_net(ip_addr a)
{ return ipa_zero(a) ? (IADDR_HOST | SCOPE_UNIVERSE) : ipa_classify(a); }

/*
 *	Conversions between internal and string representation
 */

char *ip_ntop(ip_addr a, char *);
char *ip_ntox(ip_addr a, char *);
int ip_pton(char *a, ip_addr *o);

#endif
