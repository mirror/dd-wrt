#ifndef __NIOS_CHECKSUM_H
#define __NIOS_CHECKSUM_H

/*  checksum.h:  IP/UDP/TCP checksum routines on the NIOS.
 *
 *  Copyright(C) 1995 Linus Torvalds
 *  Copyright(C) 1995 Miguel de Icaza
 *  Copyright(C) 1996 David S. Miller
 *  Copyright(C) 2001 Ken Hill
 *
 * derived from:
 *	Alpha checksum c-code
 *      ix86 inline assembly
 *      Spar nommu
 */


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * computes the checksum of the TCP/UDP pseudo-header
 * returns a 16-bit checksum, already complemented
 */

extern inline unsigned short csum_tcpudp_magic(unsigned long saddr,
					       unsigned long daddr,
					       unsigned short len,
					       unsigned short proto,
					       unsigned int sum)
{
	__asm__ __volatile__("
		add    	%0, %3
		skps	cc_nc
		addi	%0, 1
		add	%0, %4
		skps	cc_nc
		addi	%0, 1
		add	%0, %5
		skps	cc_nc
		addi	%0, 1

		; We need the carry from the addition of 16-bit
		; significant addition, so we zap out the low bits
		; in one half, zap out the high bits in another,
		; shift them both up to the top 16-bits of a word
		; and do the carry producing addition, finally
		; shift the result back down to the low 16-bits.

		; Actually, we can further optimize away two shifts
		; because we know the low bits of the original
		; value will be added to zero-only bits so cannot
		; affect the addition result nor the final carry
		; bit.

		mov	%1,%0			; Need a copy to fold with
		lsli	%1, 16			; Bring the LOW 16 bits up
		add	%0, %1			; add and set carry, neat eh?
		lsri	%0, 16			; shift back down the result
		skps	cc_nc			; get remaining carry bit
		addi	%0, 1
		not	%0			; negate
		"
	        : "=&r" (sum), "=&r" (saddr)
		: "0" (sum), "1" (saddr), "r" (ntohl(len+proto)), "r" (daddr));
		return ((unsigned short) sum); 
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


  extern inline unsigned short from32to16(unsigned long x)
  {
	__asm__ __volatile__("
		add	%0, %1
		lsri	%0, 16
		skps	cc_nc
		addi	%0, 1
		"
		: "=r" (x)
		: "r" (x << 16), "0" (x));
	return x;
  }


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


extern inline unsigned long do_csum(unsigned char * buff, int len)
{
 int odd, count;
 unsigned long result = 0;

 if (len <= 0)
 	goto out;
 odd = 1 & (unsigned long) buff;
 if (odd) {
////result = *buff;                     // dgt: Big    endian
 	result = *buff << 8;                // dgt: Little endian

 	len--;
 	buff++;
 }
 count = len >> 1;		/* nr of 16-bit words.. */
 if (count) {
 	if (2 & (unsigned long) buff) {
 		result += *(unsigned short *) buff;
 		count--;
 		len -= 2;
 		buff += 2;
 	}
 	count >>= 1;		/* nr of 32-bit words.. */
 	if (count) {
 	        unsigned long carry = 0;
 		do {
 			unsigned long w = *(unsigned long *) buff;
 			count--;
 			buff += 4;
 			result += carry;
 			result += w;
 			carry = (w > result);
 		} while (count);
 		result += carry;
 		result = (result & 0xffff) + (result >> 16);
 	}
 	if (len & 2) {
 		result += *(unsigned short *) buff;
 		buff += 2;
 	}
 }
 if (len & 1)
 	result += *buff;  /* This is little machine, byte is right */
 result = from32to16(result);
 if (odd)
 	result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);
out:
	return result;
  }


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/* ihl is always 5 or greater, almost always is 5, iph is always word
 * aligned but can fail to be dword aligned very often.
 */

  extern inline unsigned short ip_fast_csum(const unsigned char *iph, unsigned int ihl)
  {
	unsigned int sum;

	__asm__ __volatile__("
		mov	%%g1, %1	; Remember original alignment
		skp1	%1,1		; Aligned on 16 bit boundary, get first 16 bits
		br	1f		; Aligned on 32 bit boundary, go
		ld	%0, [%1]
		ext16d	%0, %1		; Get correct 16 bits
		subi	%2, 1		; Take off for 4 bytes, pickup last 2 at end
		addi	%1, 2		; Adjust pointer to 32 bit boundary
		br	2f
		 nop
1:
		subi	%2, 1
		addi	%1, 4		; Bump ptr a long word
2:
		ld      %%g2, [%1]
1:
		add     %0, %%g2
		skps	cc_nc
		addi	%0, 1
		addi	%1, 0x4
		subi	%2, 0x1
		ld      %%g2, [%1]
		skps	cc_z
		br      1b
		 nop
		skp1	%%g1,1		; Started on 16 bit boundary, pickup last 2 bytes
		br	1f		; 32 bit boundary read to leave
		 ext16d	%%g2, %1	; Get correct 16 bits
		add     %0, %%g2
		skps	cc_nc
		addi	%0, 1
1:
		mov     %2, %0
		lsli	%2, 16
		add     %0, %2
		lsri	%0, 16
		skps	cc_nc
		addi	%0, 1
		not     %0
		"
		: "=&r" (sum), "=&r" (iph), "=&r" (ihl)
		: "1" (iph), "2" (ihl)
		: "g1", "g2");
	return sum;
  }

/*these 2 functions are now in checksum.c */
unsigned int csum_partial(const unsigned char * buff, int len, unsigned int sum);
unsigned int csum_partial_copy(const char *src, char *dst, int len, int sum);

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * the same as csum_partial_copy, but copies from user space.
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */
extern inline unsigned int
csum_partial_copy_from_user(const char *src, char *dst, int len, int sum, int *csum_err)
{
	if (csum_err) *csum_err = 0;
	memcpy(dst, src, len);
	return csum_partial(dst, len, sum);
}

#define csum_partial_copy_nocheck(src, dst, len, sum)	\
	csum_partial_copy ((src), (dst), (len), (sum))


/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */

extern inline unsigned short ip_compute_csum(unsigned char * buff, int len)
{
 return ~from32to16(do_csum(buff,len));
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#define csum_partial_copy_fromuser(s, d, l, w)  \
                     csum_partial_copy((char *) (s), (d), (l), (w))


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
 *	Fold a partial checksum without adding pseudo headers
 */
extern inline unsigned int csum_fold(unsigned int sum)
{
	__asm__ __volatile__("
		add	%0, %1
		lsri	%0, 16
		skps	cc_nc
		addi	%0, 1
		not	%0
		"
		: "=r" (sum)
		: "r" (sum << 16), "0" (sum)); 
	return sum;
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//vic - FIXME - should check the constraints - they may not correct

extern __inline__ unsigned long csum_tcpudp_nofold(unsigned long saddr,
						   unsigned long daddr,
						   unsigned short len,
						   unsigned short proto,
						   unsigned int sum)
{
	__asm__ __volatile__("
		add		%0, %1
		skps	cc_nc
		addi	%0, 1		; add carry
		add	%0, %2
		skps	cc_nc
		addi	%0, 1		; add carry
		add	%0, %3
		skps	cc_nc
		addi	%0, 1		; add carry
		"
		: "=r" (sum), "=r" (saddr)
		: "r" (daddr), "r" ( (ntohs(len)<<16) + (proto*256) ),
		  "0" (sum),
		  "1" (saddr)
		: "cc");

	return sum;
}


#endif /* (__NIOS_CHECKSUM_H) */


