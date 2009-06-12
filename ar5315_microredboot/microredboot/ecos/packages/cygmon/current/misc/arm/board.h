#ifndef __CYGMON_ARM_BOARD_H__
#define __CYGMON_ARM_BOARD_H__
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================
// Hardware/platform/configuration specifics

#include <pkgconf/hal.h>
#include <pkgconf/cygmon.h>

#define HAVE_FLOAT_REGS         0
#define HAVE_DOUBLE_REGS        0
#define HAVE_CACHE              0 // FIXME
#define HAVE_USAGE              0 // FIXME
#define USE_CYGMON_PROTOTYPES   1
#define NOMAIN                  1
#define CYGMON_SYSTEM_SERVICES  0 // Not used, fall back to BSP support
#ifdef CYGDAT_CYGMON_USE_HELP
#define USE_HELP                1
#endif

// For breakpoint support
#define NO_MALLOC               1
#define MAX_BP_NUM              8
#include "cpu_info.h"
#define TRAP_SIZE               4
#define __set_breakpoint        set_breakpoint
#define __remove_breakpoint     clear_breakpoint
#define __write_mem_safe        memcpy
#define WRITE_MEM_IS_MEMCPY
#define _breakinst              bsp_breakinsn

#endif //  __CYGMON_ARM_BOARD_H__
