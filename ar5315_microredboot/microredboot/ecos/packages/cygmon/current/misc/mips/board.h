#ifndef __CYGMON_MIPS_BOARD_H__
#define __CYGMON_MIPS_BOARD_H__
//==========================================================================
//
//      board.h
//
//      Cygmon board/platform configuration file
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
// Author(s):    dmoseley
// Contributors: dmoseley
// Date:         2000-07-11
// Purpose:      
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================
// Hardware/platform/configuration specifics

// These defines are only necessary for using target_reg union in monitor.h
// It should be possible to remove that once the HAL integration is complete.
#define HAVE_FLOAT_REGS         0
#define HAVE_DOUBLE_REGS        0

#define HAVE_CACHE              0
#define HAVE_USAGE              0
#define USE_CYGMON_PROTOTYPES   1
#define NOMAIN                  1
#define CYGMON_SYSTEM_SERVICES  0 // Not used, fall back to BSP/HAL support

#ifdef CYGDAT_CYGMON_USE_HELP
#define USE_HELP                1
#endif

#define USE_ECOS_HAL_EXCEPTIONS
#define USE_ECOS_HAL_BREAKPOINTS
#define USE_ECOS_HAL_SINGLESTEP

#include "cpu_info.h"
extern void bp_print (target_register_t bp_val);
extern int __set_breakpoint (target_register_t addr);
extern int __remove_breakpoint (target_register_t addr);
extern void __install_breakpoint_list (void);
extern void __clear_breakpoint_list (void);
extern int __display_breakpoint_list (void (*print_func)(target_register_t));

#define bsp_skip_instruction(regs)      __skipinst()
#define install_breakpoints()           __install_breakpoints()
#define add_mon_breakpoint(bpt)         __set_breakpoint((bpt).addr)
#define clear_mon_breakpoint(bpt)       __remove_breakpoint((bpt).addr)
#define show_breakpoints()              __display_breakpoint_list(bp_print)
#define clear_breakpoints()             __clear_breakpoint_list()

#define bsp_get_signal(exc_nr, regs)    __computeSignal(exc_nr)
#define bsp_get_pc(regs)                get_register(PC)

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/mips-regs.h>
#include "monitor.h"

#ifdef __mips64
#define bsp_set_pc(pc, regs)                \
    put_register(PC, pc);                   \
    if (regs->_sr & SR_ERL)                 \
        put_register(EPC, pc);              \
    else                                    \
        put_register(EEPC, pc);
#else
#define bsp_set_pc(pc, regs)                \
    put_register(PC, pc);
#endif

#endif //  __CYGMON_MIPS_BOARD_H__
