#ifndef _FRIO_CHECKSUM_H
#define _FRIO_CHECKSUM_H

/*
 * MODIFIED FOR FRIO April 30, 2001 akbar.hussain@lineo.com
 *
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
unsigned int csum_partial(const unsigned char * buff, int len, unsigned int sum);

/*
 * the same as csum_partial, but copies from src while it
 * checksums
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

unsigned int csum_partial_copy(const char *src, char *dst, int len, int sum);


/*
 * the same as csum_partial_copy, but copies from user space.
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

extern unsigned int csum_partial_copy_from_user(const char *src, char *dst,
						int len, int sum, int *csum_err);

#define csum_partial_copy_nocheck(src, dst, len, sum)	\
	csum_partial_copy((src), (dst), (len), (sum))

unsigned short ip_fast_csum(unsigned char *iph, unsigned int ihl);


/*
 *	Fold a partial checksum
 */

static inline unsigned int csum_fold(unsigned int sum)
{
	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return ~sum;
}


/*
 * computes the checksum of the TCP/UDP pseudo-header
 * returns a 16-bit checksum, already complemented
 */

static inline unsigned int
csum_tcpudp_nofold(unsigned long saddr, unsigned long daddr, unsigned short len,
		  unsigned short proto, unsigned int sum)
{
	__asm__ ("%0 = %0 + %1;\n\t"
		 "%0 = %0 + %2;\n\t"
		 "%0 = %0 + %3;\n\t"
		 : "=d" (sum)
		 : "d" (daddr), "d" (saddr), "d" ((ntohs(len)<<16)+proto*256), "0"(sum));
	return sum;
}

static inline unsigned short int
csum_tcpudp_magic(unsigned long saddr, unsigned long daddr, unsigned short len,
		  unsigned short proto, unsigned int sum)
{
	return csum_fold(csum_tcpudp_nofold(saddr,daddr,len,proto,sum));
}

/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */

extern unsigned short ip_compute_csum(const unsigned char * buff, int len);

#define _HAVE_ARCH_IPV6_CSUM
static __inline__ unsigned short int
csum_ipv6_magic(struct in6_addr *saddr, struct in6_addr *daddr,
		__u32 len, unsigned short proto, unsigned int sum) 
{
	register unsigned long tmp;
	__asm__("R3 = [%2]; %0 = %0 + R3;\n\t"	/*"addl %2@,%0\n\t"*/
		"%1 = [%2+4];\n\t"		/* "movel %2@(4),%1\n\t"*/
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t"*/
		"%1 = [%2+8];\n\t"		/* "movel %2@(8),%1\n\t" */
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t"*/
		"%1 = [%2+12];\n\t"		/* "movel %2@(12),%1\n\t" */
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t" */
		"%1 = [%3];\n\t"		/* "movel %3@,%1\n\t" */
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t" */
		"%1 = [%3+4];\n\t"		/* movel %3@(4),%1\n\t"*/
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t" */
		"%1 = [%3+8];\n\t"		/* "movel %3@(8),%1\n\t"*/
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t" */
		"%1 = [%3+12];\n\t"		/* "movel %3@(12),%1\n\t" */
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0\n\t" */
		"%0 = %0 + %4; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %4,%0\n\t" */
		"%1 = 0;\n\t"			/* "clrl %1\n\t" */
		"%0 = %0 + %1; R3 = CC; %0 = %0 + R3;\n\t"	/*"addxl %1,%0"  */
		: "=&d" (sum), "=&d" (tmp)
		: "a" (saddr), "a" (daddr), "d" (len + proto),
		  "0" (sum)
		: "R3");

	return csum_fold(sum);
}

#endif /* _FRIO_CHECKSUM_H */
