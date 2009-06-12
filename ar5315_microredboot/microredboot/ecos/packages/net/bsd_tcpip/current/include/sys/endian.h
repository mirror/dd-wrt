//==========================================================================
//
//      include/sys/endian.h
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// Portions created by Red Hat are
// Copyright (C) 2002 Red Hat, Inc. All Rights Reserved.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================

//==========================================================================
//
//      include/sys/endian.h
//
//      
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


/*	$OpenBSD: endian.h,v 1.4 1999/07/21 05:58:25 csapuntz Exp $	*/

/*-
 * Copyright (c) 1997 Niklas Hallqvist.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Niklas Hallqvist.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Generic definitions for little- and big-endian systems.  Other endianesses
 * has to be dealt with in the specific machine/endian.h file for that port.
 *
 * This file is meant to be included from a little- or big-endian port's
 * machine/endian.h after setting BYTE_ORDER to either 1234 for little endian
 * or 4321 for big..
 */

#ifndef _SYS_ENDIAN_H_
#define _SYS_ENDIAN_H_

#include <cyg/hal/basetype.h>

#if CYG_BYTEORDER == CYG_MSBFIRST
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifndef _POSIX_SOURCE

#define LITTLE_ENDIAN	1234
#define BIG_ENDIAN	4321
#define PDP_ENDIAN	3412

#ifdef __GNUC__

#define __swap16gen(x) ({						\
	cyg_uint16 __swap16gen_x = (x);					\
									\
	(cyg_uint16)((__swap16gen_x & 0xff) << 8 |			\
	    (__swap16gen_x & 0xff00) >> 8);				\
})

#define __swap32gen(x) ({						\
	cyg_uint32 __swap32gen_x = (x);					\
									\
	(cyg_uint32)((__swap32gen_x & 0xff) << 24 |			\
	    (__swap32gen_x & 0xff00) << 8 |				\
	    (__swap32gen_x & 0xff0000) >> 8 |				\
	    (__swap32gen_x & 0xff000000) >> 24);			\
})

#else /* __GNUC__ */

/* Note that these macros evaluate their arguments several times.  */
#define __swap16gen(x)							\
    (cyg_uint16)(((cyg_uint16)(x) & 0xff) << 8 | ((cyg_uint16)(x) & 0xff00) >> 8)

#define __swap32gen(x)							\
    (cyg_uint32)(((cyg_uint32)(x) & 0xff) << 24 |				\
    ((cyg_uint32)(x) & 0xff00) << 8 | ((cyg_uint32)(x) & 0xff0000) >> 8 |	\
    ((cyg_uint32)(x) & 0xff000000) >> 24)

#endif /* __GNUC__ */

/*
 * Define MD_SWAP if you provide swap{16,32}md functions/macros that are
 * optimized for your architecture,  These will be used for swap{16,32}
 * unless the argument is a constant and we are using GCC, where we can
 * take advantage of the CSE phase much better by using the generic version.
 */
#ifdef MD_SWAP
#if __GNUC__

#define swap16(x) ({							\
	cyg_uint16 __swap16_x = (x);					\
									\
	__builtin_constant_p(x) ? __swap16gen(__swap16_x) :		\
	    __swap16md(__swap16_x);					\
})

#define swap32(x) ({							\
	cyg_uint32 __swap32_x = (x);					\
									\
	__builtin_constant_p(x) ? __swap32gen(__swap32_x) :		\
	    __swap32md(__swap32_x);					\
})

#endif /* __GNUC__  */

#else /* MD_SWAP */
#define swap16 __swap16gen
#define swap32 __swap32gen
#endif /* MD_SWAP */

#define swap16_multi(v, n) do {					        \
	size_t __swap16_multi_n = (n);					\
	cyg_uint16 *__swap16_multi_v = (v);				\
									\
	while (__swap16_multi_n) {					\
		*__swap16_multi_v = swap16(*__swap16_multi_v);		\
		__swap16_multi_v++;					\
		__swap16_multi_n--;					\
	}								\
} while (0)

cyg_uint32	htobe32 __P((cyg_uint32));
cyg_uint16	htobe16 __P((cyg_uint16));
cyg_uint32	betoh32 __P((cyg_uint32));
cyg_uint16	betoh16 __P((cyg_uint16));

cyg_uint32	htole32 __P((cyg_uint32));
cyg_uint16	htole16 __P((cyg_uint16));
cyg_uint32	letoh32 __P((cyg_uint32));
cyg_uint16	letoh16 __P((cyg_uint16));

#if BYTE_ORDER == LITTLE_ENDIAN

/* Can be overridden by machine/endian.h before inclusion of this file.  */
#ifndef _QUAD_HIGHWORD
#define _QUAD_HIGHWORD 1
#endif
#ifndef _QUAD_LOWWORD
#define _QUAD_LOWWORD 0
#endif

#define htobe16 swap16
#define htobe32 swap32
#define betoh16 swap16
#define betoh32 swap32

#define htole16(x) (x)
#define htole32(x) (x)
#define letoh16(x) (x)
#define letoh32(x) (x)

#endif /* BYTE_ORDER */

#if BYTE_ORDER == BIG_ENDIAN

/* Can be overridden by machine/endian.h before inclusion of this file.  */
#ifndef _QUAD_HIGHWORD
#define _QUAD_HIGHWORD 0
#endif
#ifndef _QUAD_LOWWORD
#define _QUAD_LOWWORD 1
#endif

#define htole16 swap16
#define htole32 swap32
#define letoh16 swap16
#define letoh32 swap32

#define htobe16(x) (x)
#define htobe32(x) (x)
#define betoh16(x) (x)
#define betoh32(x) (x)

#endif /* BYTE_ORDER */

#define htons htobe16
#define htonl htobe32
#define ntohs betoh16
#define ntohl betoh32

#define	NTOHL(x) (x) = ntohl((cyg_uint32)(x))
#define	NTOHS(x) (x) = ntohs((cyg_uint16)(x))
#define	HTONL(x) (x) = htonl((cyg_uint32)(x))
#define	HTONS(x) (x) = htons((cyg_uint16)(x))

#endif /* _POSIX_SOURCE */
#endif /* _SYS_ENDIAN_H_ */
