//==========================================================================
//
//      include/sys/bsdtypes.h
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

#ifndef _SYS_BSDTYPES_H_
#define _SYS_BSDTYPES_H_

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>

#define	__BIT_TYPES_DEFINED__
#undef uint8_t
#undef uint16_t
#undef uint32_t
typedef	__signed char		   int8_t;
typedef	unsigned char		 u_int8_t;
typedef	unsigned char		  uint8_t;
typedef	short			  int16_t;
typedef	unsigned short		u_int16_t;
typedef	unsigned short		 uint16_t;
typedef	int			  int32_t;
typedef	unsigned int		u_int32_t;
typedef	unsigned int		 uint32_t;
typedef	long long		  int64_t;
typedef	unsigned long long	u_int64_t;
typedef	unsigned long long	 uint64_t;

// Types inherited from HAL 

typedef CYG_ADDRESS               vaddr_t;
typedef CYG_ADDRWORD              vsize_t;
typedef CYG_ADDRESS               paddr_t;
typedef CYG_ADDRWORD              psize_t;

typedef CYG_ADDRESS           vm_offset_t;
typedef CYG_ADDRWORD            vm_size_t;

// No good HAL definition for this

typedef CYG_ADDRWORD           register_t;


// From <arch/ansi.h>
/*
 * Types which are fundamental to the implementation and may appear in
 * more than one standard header are defined here.  Standard headers
 * then use:
 *	#ifdef	_BSD_SIZE_T_
 *	typedef	_BSD_SIZE_T_ size_t;
 *	#undef	_BSD_SIZE_T_
 *	#endif
 */
#define	_BSD_SSIZE_T_	int		 /* byte count or error */
#define _BSD_CLOCKID_T_	int
#define _BSD_TIMER_T_	int
#ifndef __time_t_defined                 // As defined/used by eCos libc
#define	_BSD_CLOCK_T_	cyg_int64	 /* clock() */
#define	_BSD_TIME_T_	cyg_count32	 /* time() */
#endif

#endif // _MACHINE_TYPES_H_

// Standard BSD types
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef char           *caddr_t;

typedef u_int64_t       u_quad_t;       /* quads */
typedef int64_t         quad_t;
typedef quad_t *        qaddr_t;

/*
 * XPG4.2 states that inclusion of <netinet/in.h> must pull these
 * in and that inclusion of <sys/socket.h> must pull in sa_family_t.
 * We put there here because there are other headers that require
 * these types and <sys/socket.h> and <netinet/in.h> will indirectly
 * include <sys/types.h>.  Thus we are compliant without too many hoops.
 */
typedef u_int32_t       in_addr_t;      /* base type for internet address */
typedef u_int16_t       in_port_t;      /* IP port type */
typedef u_int8_t        sa_family_t;    /* sockaddr address family type */
typedef u_int32_t       socklen_t;      /* length type for network syscalls */
