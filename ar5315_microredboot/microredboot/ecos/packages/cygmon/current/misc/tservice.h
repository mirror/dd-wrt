#ifndef __TSERVICE_H__
#define __TSERVICE_H__
//==========================================================================
//
//      tservice.h
//
//      These are the core functions are expected to be provided by the
//      target dependent services.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

/* These are the core functions are expected to be provided by the
   target dependent services. They are used by both cygmon and by
   libstub
*/


extern int read_memory (mem_addr_t *src, int size, int amt, char *dst);

extern int write_memory (mem_addr_t *dst, int size, int amt, char *src);


#ifndef USE_ECOS_HAL_BREAKPOINTS
extern void set_breakpoint (struct bp *bp);
extern void clear_breakpoint (struct bp *bp);
#endif // USE_ECOS_HAL_BREAKPOINTS

#ifndef HAVE_BSP
extern void set_pc (target_register_t pc);
#endif

extern target_register_t next_step_pc (void);

extern void enable_interrupts (void);

#endif // __TSERVICE_H__


extern void initialize_mon(void);
