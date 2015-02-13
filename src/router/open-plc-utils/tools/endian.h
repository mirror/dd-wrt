/*====================================================================*
 *
 *   endian.h - integer byte order declarations and definitions;
 *
 *   this header is included to support recent moves to expand and
 *   standardize endian conversion functions on Linux, FreeBSD and
 *   NetBSD; Linux has implemented the following functions but OSX
 *   and Microsoft have not yet done so;
 *
 *   These functions are similar to network byteorder functions.
 *   For example, be32toh() is identical to ntohl().
 *
 *   #define _BSD_SOURCE
 *   #include <endian.h>
 *
 *   uint16_t htobe16(uint16_t x);
 *   uint16_t htole16(uint16_t x);
 *   uint16_t be16toh(uint16_t x);
 *   uint16_t le16toh(uint16_t x);
 *
 *   uint32_t htobe32(uint32_t x);
 *   uint32_t htole32(uint32_t x);
 *   uint32_t be32toh(uint32_t x);
 *   uint32_t le32toh(uint32_t x);
 *
 *   uint64_t htobe64(uint64_t x);
 *   uint64_t htole64(uint64_t x);
 *   uint64_t be64toh(uint64_t x);
 *   uint64_t le64toh(uint64_t x);
 *
 *   The advantage of network byteorder functions is that they are
 *   available on all Unix systems but they were meant for network
 *   applications and lack little-endian and 64-bit variants;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef ENDIAN_HEADER
#define ENDIAN_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>

#if defined (__linux__)
#       include <endian.h>
#       include <byteswap.h>
#elif defined (__APPLE__)
#       include <netinet/in.h>
#       include <libkern/OSByteOrder.h>
#elif defined (__OpenBSD__)
#       include <sys/types.h>
#elif defined (WIN32)
#       include <stdint.h>
#       define BYTE_ORDER LITTLE_ENDIAN
#elif defined (__vxworks)
#       include <netinet/in.h>
#elif defined (__CYGWIN__)
#	error "Cygwin is unsupported"
#else
#	error "Unknown environment"
#endif

/*====================================================================*
 *   definitions;
 *--------------------------------------------------------------------*/

#if defined (BYTE_ORDER)
#       if BYTE_ORDER == LITTLE_ENDIAN
#               define BE16TOH(x) __bswap_16(x)
#               define BE32TOH(x) __bswap_32(x)
#               define BE64TOH(x) __bswap_64(x)
#               define HTOBE16(x) __bswap_16(x)
#               define HTOBE32(x) __bswap_32(x)
#               define HTOBE64(x) __bswap_64(x)
#               define LE16TOH(x) (x)
#               define LE32TOH(x) (x)
#               define LE64TOH(x) (x)
#               define HTOLE16(x) (x)
#               define HTOLE32(x) (x)
#               define HTOLE64(x) (x)
#       elif BYTE_ORDER == BIG_ENDIAN
#               define BE16TOH(x) (x)
#               define BE32TOH(x) (x)
#               define BE64TOH(x) (x)
#               define HTOBE16(x) (x)
#               define HTOBE32(x) (x)
#               define HTOBE64(x) (x)
#               define LE16TOH(x) __bswap_16(x)
#               define LE32TOH(x) __bswap_32(x)
#               define LE64TOH(x) __bswap_64(x)
#               define HTOLE16(x) __bswap_16(x)
#               define HTOLE32(x) __bswap_32(x)
#               define HTOLE64(x) __bswap_64(x)
#       else
#               error "Undefined host byte order (2)."
#       endif
#elif defined (__vxworks) && defined (_BYTE_ORDER)
#       if _BYTE_ORDER == _LITTLE_ENDIAN
#               define BE16TOH(x) __bswap_16(x)
#               define BE32TOH(x) __bswap_32(x)
#               define BE64TOH(x) __bswap_64(x)
#               define HTOBE16(x) __bswap_16(x)
#               define HTOBE32(x) __bswap_32(x)
#               define HTOBE64(x) __bswap_64(x)
#               define LE16TOH(x) (x)
#               define LE32TOH(x) (x)
#               define LE64TOH(x) (x)
#               define HTOLE16(x) (x)
#               define HTOLE32(x) (x)
#               define HTOLE64(x) (x)
#       elif _BYTE_ORDER == _BIG_ENDIAN
#               define __bswap_32(x)    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) <<  8) | (((x) & 0x00ff0000) >>  8) | (((x) & 0xff000000) >> 24))
#               define __bswap_16(x)    ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#               define BE16TOH(x) (x)
#               define BE32TOH(x) (x)
#               define BE64TOH(x) (x)
#               define HTOBE16(x) (x)
#               define HTOBE32(x) (x)
#               define HTOBE64(x) (x)
#               define LE16TOH(x) __bswap_16(x)
#               define LE32TOH(x) __bswap_32(x)
#               define LE64TOH(x) __bswap_64(x)
#               define HTOLE16(x) __bswap_16(x)
#               define HTOLE32(x) __bswap_32(x)
#               define HTOLE64(x) __bswap_64(x)
#       else
#               error "Undefined host byte order vxworks."
#       endif
#else
#error "Undefined host byte order (1)."
#endif

/*====================================================================*
 *   endian conversion functions;
 *--------------------------------------------------------------------*/

#if defined (WIN32)

uint16_t __bswap_16 (uint16_t x);
uint32_t __bswap_32 (uint32_t x);
uint64_t __bswap_64 (uint64_t x);

#elif defined (__OpenBSD__)

#define __bswap_16(x) swap16(x)
#define __bswap_32(x) swap32(x)
#define __bswap_64(x) swap64(x)

#elif defined (__APPLE__)

#define __bswap_16(x) OSSwapInt16(x)
#define __bswap_32(x) OSSwapInt32(x)
#define __bswap_64(x) OSSwapInt64(x)

#elif defined (__linux__)

#else
#error "Unknown Environment"
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

