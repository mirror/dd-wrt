
/*
 * MD5.H - header file for MD5C.C 
 */

/*
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights
 * reserved.
 * 
 * License to copy and use this software is granted provided that it is
 * identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm"
 * in all material mentioning or referencing this software or this function.
 * 
 * License is also granted to make and use derivative works provided that
 * such works are identified as "derived from the RSA Data Security, Inc. MD5 
 * Message-Digest Algorithm" in all material mentioning or referencing the
 * derived work.
 * 
 * RSA Data Security, Inc. makes no representations concerning either the
 * merchantability of this software or the suitability of this software for
 * any particular purpose. It is provided "as is" without express or implied
 * warranty of any kind.
 * 
 * These notices must be retained in any copies of any part of this
 * documentation and/or software. 
 */

/*
 * MD5 context. 
 */
#include <string.h>
#include <byteswap.h>
#include <endian.h>
#include <sys/types.h>

#ifdef __BIG_ENDIAN__
#define BB_BIG_ENDIAN 1
#define BB_LITTLE_ENDIAN 0
#elif __BYTE_ORDER == __BIG_ENDIAN
#define BB_BIG_ENDIAN 1
#define BB_LITTLE_ENDIAN 0
#else
#define BB_BIG_ENDIAN 0
#define BB_LITTLE_ENDIAN 1
#endif

#if BB_BIG_ENDIAN
#define SWAP_BE16(x) (x)
#define SWAP_BE32(x) (x)
#define SWAP_BE64(x) (x)
#define SWAP_LE16(x) bswap_16(x)
#define SWAP_LE32(x) bswap_32(x)
#define SWAP_LE64(x) bswap_64(x)
#else
#define SWAP_BE16(x) bswap_16(x)
#define SWAP_BE32(x) bswap_32(x)
#define SWAP_BE64(x) bswap_64(x)
#define SWAP_LE16(x) (x)
#define SWAP_LE32(x) (x)
#define SWAP_LE64(x) (x)
#endif

#define FAST_FUNC

#define ALIGN1 __attribute__((aligned(1)))
#define ALIGN2 __attribute__((aligned(2)))

typedef u_int32_t uint32_t;
typedef u_int64_t uint64_t;

typedef struct md5_ctx_t {
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint64_t total;
	uint32_t buflen;
	char buffer[128];
} md5_ctx_t;
void dd_md5_begin(md5_ctx_t *ctx);
void dd_md5_hash(const void *data, uint32_t length, md5_ctx_t *ctx);
void *dd_md5_end(void *resbuf, md5_ctx_t *ctx);
