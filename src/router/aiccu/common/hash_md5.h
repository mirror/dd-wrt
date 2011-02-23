/*
 * This is the header file for the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h'
 * header definitions; now uses stuff from dpkg's config.h
 *  - Ian Jackson <ian@chiark.greenend.org.uk>.
 * Still in the public domain.
 */

#ifndef MD5_H
#define MD5_H

#include "common.h"

#ifdef _WIN32
typedef unsigned long uint32;
#define UWORD32 uint32

#else /* _WIN32 */

#ifdef _SUNOS
typedef unsigned int uint32;
#define UWORD32 uint32
#else
#define UWORD32 u_int32_t
#endif /* _SUNOS */

#endif /* _WIN32 */

/* Determine Endianness */
#if BYTE_ORDER == LITTLE_ENDIAN
	/* 1234 machines */
#elif BYTE_ORDER == BIG_ENDIAN
	/* 4321 machines */
# define WORDS_BIGENDIAN 1
#elif BYTE_ORDER == PDP_ENDIAN
	/* 3412 machines */
#error PDP endianness not supported yet!
#else
#error unknown endianness!
#endif

#define md5byte unsigned char

struct MD5Context {
	UWORD32 buf[4];
	UWORD32 bytes[2];
	UWORD32 in[16];
};

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, md5byte const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(UWORD32 buf[4], UWORD32 const in[16]);

#endif /* !MD5_H */
