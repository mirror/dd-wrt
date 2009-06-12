#ifndef __MN10300_CPU_INFO_H__
#define __MN10300_CPU_INFO_H__
//==========================================================================
//
//      cpu_info.h
//
//      Architecture information for MN10300 processors
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
// Date:         2000-08-11
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//=========================================================================

#define IS_MN10300 1

/* Temporary as long a multiple protypes are copied in multiple files */
/* This variation does NOT clone the prototypes */
#define NO_MALLOC 1

#ifndef USE_ECOS_HAL_BREAKPOINTS

/* big enuf to store a trap in the BP structure */

#define BP_INST_T_DEFINED 1
typedef unsigned char bp_inst_t ;

#else /* USE_ECOS_HAL_BREAKPOINTS */

#define MEM_ADDR_DEFINED 1 
typedef struct mem_addr {
  unsigned long addr;
} mem_addr_t ;

#endif /* USE_ECOS_HAL_BREAKPOINTS */

typedef unsigned long target_register_t;

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/basetype.h>
#define PRINT_INSN print_insn_mn10300

#undef BFD_MACH
#define BFD_MACH 0

// Ensure that the reg_valid field in regstruct is used.
#define REG_VALID_FIELD_IN_REGSTRUCT
#define INITIALIZE_MON_EACH_TIME() initialize_mon_each_time()
extern void initialize_mon_each_time(void);

#endif // __MN10300_CPU_INFO_H__
