/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	header.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Paul Lin	1-2-2004
*/
#ifndef	__RTMP_TYPE_H__
#define	__RTMP_TYPE_H__

// Put platform dependent declaration here
// For example, linux type definition
//#ifdef	Linux

typedef	unsigned short		UINT16;
typedef	unsigned long		UINT32;
typedef unsigned long long	UINT64;

//#endif  // Linux

/*
#ifdef	Win32

#undef	BIG_ENDIAN			// Only little endian foe WIN32 system

typedef	unsigned short		UINT16;
typedef	unsigned long		UINT32;
typedef unsigned __int64	UINT64;

#endif  // Win32
*/

#define PACKED  __attribute__ ((packed))

// Endian byte swapping codes
#ifdef	BIG_ENDIAN
#define SWAP16(x) \
	((UINT16)( \
	(((UINT16)(x) & (UINT16) 0x00ffU) << 8) | \
	(((UINT16)(x) & (UINT16) 0xff00U) >> 8) ))

#define SWAP32(x) \
	((UINT32)( \
	(((UINT32)(x) & (UINT32) 0x000000ffUL) << 24) | \
	(((UINT32)(x) & (UINT32) 0x0000ff00UL) <<  8) | \
	(((UINT32)(x) & (UINT32) 0x00ff0000UL) >>  8) | \
	(((UINT32)(x) & (UINT32) 0xff000000UL) >> 24) ))

#define SWAP64(x) \
	((UINT64)( \
	(UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) | \
	(UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))
#else

#define SWAP16(x)
#define SWAP32(x)
#define	SWAP64(x)

#endif  // BIG_ENDIAN


#ifdef BIG_ENDIAN

#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((UINT64)(x))
#define be2cpu64(x) ((UINT64)(x))
#define cpu2be32(x) ((UINT32)(x))
#define be2cpu32(x) ((UINT32)(x))
#define cpu2be16(x) ((UINT16)(x))
#define be2cpu16(x) ((UINT16)(x))

#else	// Little_Endian

#define cpu2le64(x) ((UINT64)(x))
#define le2cpu64(x) ((UINT64)(x))
#define cpu2le32(x) ((UINT32)(x))
#define le2cpu32(x) ((UINT32)(x))
#define cpu2le16(x) ((UINT16)(x))
#define le2cpu16(x) ((UINT16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))

#endif	// BIG_ENDIAN

typedef enum { FALSE = 0, TRUE = 1 } Boolean;

#endif	// __RTMP_TYPE_H__

