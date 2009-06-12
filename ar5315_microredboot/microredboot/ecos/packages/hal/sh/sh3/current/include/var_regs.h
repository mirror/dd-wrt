#ifndef CYGONCE_VAR_SH_REGS_H
#define CYGONCE_VAR_SH_REGS_H
//=============================================================================
//
//      sh_regs.h
//
//      SH CPU definitions
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
// Author(s):   jskov
// Contributors:jskov
// Date:        1999-04-24
// Purpose:     Define CPU memory mapped registers etc.
// Usage:       #include <cyg/hal/sh_regs.h>
// Notes:
//   This file describes registers for on-core modules found in all
//   the SH3 CPUs supported by the HAL. For each CPU is defined a
//   module specification file (mod_<CPU model number>.h) which lists
//   modules (and their version if applicable) included in that
//   particular CPU model.  Note that the versioning is ad hoc;
//   it doesn't reflect Hitachi internal versioning in any way.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// Find out which modules are supported by the chosen CPU
#include <pkgconf/system.h>
#include CYGBLD_HAL_CPU_MODULES_H

//==========================================================================
//                             CPU Definitions
//==========================================================================

//--------------------------------------------------------------------------
// Status register
#define CYGARC_REG_SR_MD                0x40000000
#define CYGARC_REG_SR_RB                0x20000000
#define CYGARC_REG_SR_BL                0x10000000
#define CYGARC_REG_SR_M                 0x00000200
#define CYGARC_REG_SR_Q                 0x00000100
#define CYGARC_REG_SR_IMASK             0x000000f0
#define CYGARC_REG_SR_I3                0x00000080
#define CYGARC_REG_SR_I2                0x00000040
#define CYGARC_REG_SR_I1                0x00000020
#define CYGARC_REG_SR_I0                0x00000010
#define CYGARC_REG_SR_S                 0x00000002
#define CYGARC_REG_SR_T                 0x00000001


//==========================================================================
//                             Module Definitions
//==========================================================================

#include <cyg/hal/mod_regs_bsc.h>
#include <cyg/hal/mod_regs_cac.h>
#include <cyg/hal/mod_regs_cpg.h>
#include <cyg/hal/mod_regs_intc.h>
#include <cyg/hal/mod_regs_mmu.h>
#include <cyg/hal/mod_regs_rtc.h>
#include <cyg/hal/mod_regs_ser.h>
#include <cyg/hal/mod_regs_tmu.h>
#include <cyg/hal/mod_regs_ubc.h>

#ifdef CYGARC_SH_MOD_DMAC
#include <cyg/hal/mod_regs_dma.h>
#endif
#ifdef CYGARC_SH_MOD_PFC
#include <cyg/hal/mod_regs_pfc.h>
#endif

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_SH_REGS_H
// End of var_regs.h
