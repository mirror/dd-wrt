#ifndef CYGONCE_SIGSETJMP_H
#define CYGONCE_SIGSETJMP_H
//=============================================================================
//
//      sigsetjmp.h
//
//      POSIX sigsetjmp header
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
// Author(s):     nickg, jlarmour
// Contributors:  
// Date:          2000-03-17
// Purpose:       POSIX sigsetjmp header
// Description:   This header contains all the definitions needed to support
//                the POSIX sigsetjmp/siglongjmp API under eCos.
//              
// Usage:         This file must be included indirectly via
//                the C library setjmp.h header.
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <signal.h>
#include <cyg/hal/hal_arch.h>       // hal_jmp_buf

//=============================================================================
// sigjmp_buf structure
// The API requires this to be an array type, but this array actually
// contains three fields:
// 0..sizeof(hal_jmp_buf)-1               HAL jump buffer
// sizeof(hal_jmp_buf)                    savemask value (an int)
// sizeof(hal_jmp_buf)+sizeof(int)...     sigset_t containing saved mask

typedef struct {
    hal_jmp_buf __jmp_buf;
    int  __savemask;
    sigset_t __sigsavemask;
} sigjmp_buf[1];

//=============================================================================
// sigsetjmp() macro

#define sigsetjmp( _env_, _savemask_ )                                        \
(                                                                             \
 ((_env_)[0].__savemask = _savemask_),                                        \
 ((_savemask_)?pthread_sigmask(SIG_BLOCK,NULL,&((_env_)[0].__sigsavemask)):0),\
 hal_setjmp((_env_)[0].__jmp_buf)                                             \
)

//=============================================================================
// siglongjmp function

__externC void siglongjmp( sigjmp_buf env, int val );

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_SIGSETJMP_H
// End of sigsetjmp.h
