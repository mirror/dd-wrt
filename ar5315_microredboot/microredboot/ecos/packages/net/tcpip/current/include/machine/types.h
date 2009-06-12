//==========================================================================
//
//      include/machine/types.h
//
//      Architecture/platform specific support for data types
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


#ifndef _MACHINE_TYPES_H_
#define _MACHINE_TYPES_H_

#include <sys/cdefs.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>

#define	__BIT_TYPES_DEFINED__
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
