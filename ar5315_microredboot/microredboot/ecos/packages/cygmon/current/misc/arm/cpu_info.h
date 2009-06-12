#ifndef __ARM_CPU_INFO_H__
#define __ARM_CPU_INFO_H__
//==========================================================================
//
//      cpu_info.h
//
//      Architecture information for ARM processors
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
// Description:  ARM is a Registered Trademark of Advanced RISC Machines
//               Limited.
//               Other Brands and Trademarks are the property of their
//               respective owners.               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#define IS_ARM 1

#ifndef __CYGMON_TYPES
#define __CYGMON_TYPES
typedef unsigned int uint32;
typedef int          int32;
#endif // __CYGMON_TYPES


/* Temporary as long a multiple protypes are copied in multiple files */
/* This variation does NOT clone the prototypes */
#define NO_MALLOC 1
#define MAX_NUM_BP 32
#define MAX_HIST_ENTS 10

/* big enuf to store a trap in the BP structure */

#define BP_INST_T_DEFINED 1 
typedef unsigned long bp_inst_t ;

#define MEM_ADDR_DEFINED 1 
typedef struct mem_addr {
  unsigned long addr;
} mem_addr_t ;

#ifndef TARGET_REGISTER_T_DEFINED
#define TARGET_REGISTER_T_DEFINED
typedef unsigned long target_register_t;
#endif

#if defined(__ARMEB__)
#define PRINT_INSN print_insn_big_arm
#else
#define PRINT_INSN print_insn_little_arm
#endif

#define OTHERNAMES_CMD arm_othernames
extern void arm_othernames (void);

#undef BFD_MACH
#define BFD_MACH 0

#endif // __ARM_CPU_INFO_H__
