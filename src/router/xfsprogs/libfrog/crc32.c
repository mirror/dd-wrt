// SPDX-License-Identifier: GPL-2.0
/*
 * Aug 8, 2011 Bob Pearson with help from Joakim Tjernlund and George Spelvin
 * cleaned up code to current version of sparse and added the slicing-by-8
 * algorithm to the closely similar existing slicing-by-4 algorithm.
 * Oct 15, 2000 Matt Domsch <Matt_Domsch@dell.com>
 * Nicer crc32 functions/docs submitted by linux@horizon.com.  Thanks!
 * Code was from the public domain, copyright abandoned.  Code was
 * subsequently included in the kernel, thus was re-licensed under the
 * GNU GPL v2.
 * Oct 12, 2000 Matt Domsch <Matt_Domsch@dell.com>
 * Same crc32 function was used in 5 other places in the kernel.
 * I made one version, and deleted the others.
 * There are various incantations of crc32().  Some use a seed of 0 or ~0.
 * Some xor at the end with ~0.  The generic crc32() function takes
 * seed as an argument, and doesn't xor at the end.  Then individual
 * users can do whatever they need.
 *   drivers/net/smc9194.c uses seed ~0, doesn't xor with ~0.
 *   fs/jffs2 uses seed 0, doesn't xor with ~0.
 *   fs/partitions/efi.c uses seed ~0, xor's with ~0.
 */

/* see: Documentation/crc32.txt for a description of algorithms */

/*
 * lifted from the 3.8-rc2 kernel source for xfsprogs. Killed CONFIG_X86
 * specific bits for just the generic algorithm. Also removed the big endian
 * version of the algorithm as XFS only uses the little endian CRC version to
 * match the hardware acceleration available on Intel CPUs.
 */

#include <inttypes.h>
#include <asm/types.h>
#include <sys/time.h>
#include "platform_defs.h"
/* For endian conversion routines */
#include "xfs_arch.h"
#include "crc32defs.h"
#include "crc32c.h"

/* types specifc to this file */
typedef __u8	u8;
typedef __u16	u16;
typedef __u32	u32;
typedef __u32	u64;
#define __pure

#if CRC_LE_BITS > 8
# define tole(x) ((__force u32) __constant_cpu_to_le32(x))
#else
# define tole(x) (x)
#endif

#include "crc32table.h"

#if CRC_LE_BITS > 8

/* implements slicing-by-4 or slicing-by-8 algorithm */
static inline u32
crc32_body(u32 crc, unsigned char const *buf, size_t len, const u32 (*tab)[256])
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define DO_CRC(x) crc = t0[(crc ^ (x)) & 255] ^ (crc >> 8)
#  define DO_CRC4 (t3[(q) & 255] ^ t2[(q >> 8) & 255] ^ \
		   t1[(q >> 16) & 255] ^ t0[(q >> 24) & 255])
#  define DO_CRC8 (t7[(q) & 255] ^ t6[(q >> 8) & 255] ^ \
		   t5[(q >> 16) & 255] ^ t4[(q >> 24) & 255])
# elif __BYTE_ORDER == __BIG_ENDIAN
#  define DO_CRC(x) crc = t0[((crc >> 24) ^ (x)) & 255] ^ (crc << 8)
#  define DO_CRC4 (t0[(q) & 255] ^ t1[(q >> 8) & 255] ^ \
		   t2[(q >> 16) & 255] ^ t3[(q >> 24) & 255])
#  define DO_CRC8 (t4[(q) & 255] ^ t5[(q >> 8) & 255] ^ \
		   t6[(q >> 16) & 255] ^ t7[(q >> 24) & 255])
# else
#  error What endian are you?
# endif
	const u32 *b;
	size_t    rem_len;
	const u32 *t0=tab[0], *t1=tab[1], *t2=tab[2], *t3=tab[3];
# if CRC_LE_BITS != 32
	const u32 *t4 = tab[4], *t5 = tab[5], *t6 = tab[6], *t7 = tab[7];
# endif
	u32 q;

	/* Align it */
	if (((long)buf & 3) && len) {
		do {
			DO_CRC(*buf++);
		} while ((--len) && ((long)buf)&3);
	}

# if CRC_LE_BITS == 32
	rem_len = len & 3;
	len = len >> 2;
# else
	rem_len = len & 7;
	len = len >> 3;
# endif

	b = (const u32 *)buf;
	for (--b; len; --len) {
		q = crc ^ *++b; /* use pre increment for speed */
# if CRC_LE_BITS == 32
		crc = DO_CRC4;
# else
		crc = DO_CRC8;
		q = *++b;
		crc ^= DO_CRC4;
# endif
	}
	len = rem_len;
	/* And the last few bytes */
	if (len) {
		u8 *p = (u8 *)(b + 1) - 1;
		do {
			DO_CRC(*++p); /* use pre increment for speed */
		} while (--len);
	}
	return crc;
#undef DO_CRC
#undef DO_CRC4
#undef DO_CRC8
}
#endif

/**
 * crc32_le() - Calculate bitwise little-endian Ethernet AUTODIN II CRC32
 * @crc: seed value for computation.  ~0 for Ethernet, sometimes 0 for
 *	other uses, or the previous crc32 value if computing incrementally.
 * @p: pointer to buffer over which CRC is run
 * @len: length of buffer @p
 */
static inline u32 __pure crc32_le_generic(u32 crc, unsigned char const *p,
					  size_t len, const u32 (*tab)[256],
					  u32 polynomial)
{
#if CRC_LE_BITS == 1
	int i;
	while (len--) {
		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? polynomial : 0);
	}
# elif CRC_LE_BITS == 2
	while (len--) {
		crc ^= *p++;
		crc = (crc >> 2) ^ tab[0][crc & 3];
		crc = (crc >> 2) ^ tab[0][crc & 3];
		crc = (crc >> 2) ^ tab[0][crc & 3];
		crc = (crc >> 2) ^ tab[0][crc & 3];
	}
# elif CRC_LE_BITS == 4
	while (len--) {
		crc ^= *p++;
		crc = (crc >> 4) ^ tab[0][crc & 15];
		crc = (crc >> 4) ^ tab[0][crc & 15];
	}
# elif CRC_LE_BITS == 8
	/* aka Sarwate algorithm */
	while (len--) {
		crc ^= *p++;
		crc = (crc >> 8) ^ tab[0][crc & 255];
	}
# else
	crc = (__force u32) cpu_to_le32(crc);
	crc = crc32_body(crc, p, len, tab);
	crc = le32_to_cpu((__force __le32)crc);
#endif
	return crc;
}

#if CRC_LE_BITS == 1
u32 __pure crc32c_le(u32 crc, unsigned char const *p, size_t len)
{
	return crc32_le_generic(crc, p, len, NULL, CRC32C_POLY_LE);
}
#else
u32 __pure crc32c_le(u32 crc, unsigned char const *p, size_t len)
{
	return crc32_le_generic(crc, p, len,
			(const u32 (*)[256])crc32ctable_le, CRC32C_POLY_LE);
}
#endif


#ifdef CRC32_SELFTEST
# include "crc32cselftest.h"

/*
 * make sure we always return 0 for a successful test run, and non-zero for a
 * failed run. The build infrastructure is looking for this information to
 * determine whether to allow the build to proceed.
 */
int main(int argc, char **argv)
{
	int errors;

	printf("CRC_LE_BITS = %d\n", CRC_LE_BITS);

	errors = crc32c_test();

	return errors != 0;
}
#endif /* CRC32_SELFTEST */
