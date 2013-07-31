/*
 *	BIRD Library -- IP address routines common for IPv4 and IPv6
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include "nest/bird.h"
#include "lib/ip.h"

/**
 * DOC: IP addresses
 *
 * BIRD uses its own abstraction of IP address in order to share the same
 * code for both IPv4 and IPv6. IP addresses are represented as entities
 * of type &ip_addr which are never to be treated as numbers and instead
 * they must be manipulated using the following functions and macros.
 */

/**
 * ip_scope_text - get textual representation of address scope
 * @scope: scope (%SCOPE_xxx)
 *
 * Returns a pointer to a textual name of the scope given.
 */
char *
ip_scope_text(unsigned scope)
{
  static char *scope_table[] = { "host", "link", "site", "org", "univ", "undef" };

  if (scope > SCOPE_UNDEFINED)
    return "?";
  else
    return scope_table[scope];
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
 * ipa_mkmask - calculate netmask length
 * @x: IP address
 *
 * This function checks whether @x represents a valid netmask and
 * returns the size of the associate network prefix or -1 for invalid
 * mask.
 */
int ipa_mklen(ip_addr x) { DUMMY }

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
 * ipa_class_mask - guess netmask according to address class
 * @x: IP address
 *
 * This function (available in IPv4 version only) returns a
 * network mask according to the address class of @x. Although
 * classful addressing is nowadays obsolete, there still live
 * routing protocols transferring no prefix lengths nor netmasks
 * and this function could be useful to them.
 */
ip_addr ipa_class_mask(ip_addr x) { DUMMY }

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
 * ipa_build - build an IPv6 address from parts
 * @a1: part #1
 * @a2: part #2
 * @a3: part #3
 * @a4: part #4
 *
 * ipa_build() takes @a1 to @a4 and assembles them to a single IPv6
 * address. It's used for example when a protocol wants to bind its
 * socket to a hard-wired multicast address.
 */
ip_addr ipa_build(u32 a1, u32 a2, u32 a3, u32 a4) { DUMMY }

/**
 * ipa_absolutize - convert link scope IPv6 address to universe scope
 * @x: link scope IPv6 address
 * @y: universe scope IPv6 prefix of the interface
 *
 * This function combines a link-scope IPv6 address @x with the universe
 * scope prefix @x of the network assigned to an interface to get a
 * universe scope form of @x.
 */
ip_addr ipa_absolutize(ip_addr x, ip_addr y) { DUMMY }

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
