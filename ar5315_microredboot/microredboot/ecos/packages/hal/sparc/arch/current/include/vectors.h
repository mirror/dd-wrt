#ifndef CYGONCE_HAL_VECTORS_H
#define CYGONCE_HAL_VECTORS_H

//=============================================================================
//
//      vectors.h
//
//      SPARC Architecture specific vector numbers &c
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:hmt
// Date:        1998-12-10
// Purpose:     Define architecture abstractions and some shared info;
//              this file is included by assembler files as well as C/C++.
// Usage:       #include <cyg/hal/vectors.h>

//              
//####DESCRIPTIONEND####
//
//=============================================================================

#define __WINSIZE 8

#if __WINSIZE <= 8
# define __WINBITS 7
#else
# error __WINSIZE window size probably not supported
#endif

// These should be generic to all SPARCs:

#define __WINBITS_MAXIMAL 0x1f

#define __WIN_INIT (__WINSIZE - 1)
#define __WIM_INIT (1 << __WIN_INIT)

// ------------------------------------------------------------------------

#define TRAP_WUNDER     6       // Window Underflow trap number
#define TRAP_WOVER      5       // Window Overflow trap number

#define TRAP_INTR_MIN   17      // smallest interrupt trap number
#define TRAP_INTR_MAX   31      // largest interrupt trap number

#define TT_MASK         0xff0   // trap type mask from tbr
#define TT_SHL          4       // shift to get a tbr value

// Alternatively, detect an interrupt by testing tbr for being in the range
// 16-31 by masking &c:
#define TT_IS_INTR_MASK         0xf00
#define TT_IS_INTR_VALUE        0x100

#if TT_IS_INTR_VALUE != ((TRAP_INTR_MIN << TT_SHL) & TT_IS_INTR_MASK)
#error "Bad *_INTR_* symbol definition (1)"
#endif

#if TT_IS_INTR_VALUE != ((TRAP_INTR_MAX << TT_SHL) & TT_IS_INTR_MASK)
#error "Bad *_INTR_* symbol definition (2)"
#endif

#if TT_IS_INTR_VALUE != (((TRAP_INTR_MIN+1) << TT_SHL) & TT_IS_INTR_MASK)
#error "Bad *_INTR_* symbol definition (3)"
#endif

#if TT_IS_INTR_VALUE != (((TRAP_INTR_MAX-1) << TT_SHL) & TT_IS_INTR_MASK)
#error "Bad *_INTR_* symbol definition (4)"
#endif


        
//#define SCHED_LOCK_MANGLED_NAME _18Cyg_Scheduler_Base.sched_lock
#define SCHED_LOCK_MANGLED_NAME cyg_scheduler_sched_lock



#define SAVE_REGS_SIZE (4 * 32) // 32 words of 4 bytes each




#endif // CYGONCE_HAL_VECTORS_H
// EOF vectors.h
