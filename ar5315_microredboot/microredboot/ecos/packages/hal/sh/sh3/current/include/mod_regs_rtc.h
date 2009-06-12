//=============================================================================
//
//      mod_regs_rtc.h
//
//      RTC (real time clock) Module register definitions
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
// Date:        2000-10-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// RealTime Clock

#define CYGARC_REG_RC64CNT              0xfffffec0
#define CYGARC_REG_RSECCNT              0xfffffec2
#define CYGARC_REG_RMINCNT              0xfffffec4
#define CYGARC_REG_RHRCNT               0xfffffec6
#define CYGARC_REG_RWKCNT               0xfffffec8
#define CYGARC_REG_RDAYCNT              0xfffffeca
#define CYGARC_REG_RMONCNT              0xfffffecc
#define CYGARC_REG_RYRCNT               0xfffffece
#define CYGARC_REG_RSECAR               0xfffffed0
#define CYGARC_REG_RMINAR               0xfffffed2
#define CYGARC_REG_RHRAR                0xfffffed4
#define CYGARC_REG_RWKAR                0xfffffed6
#define CYGARC_REG_RDAYAR               0xfffffed8
#define CYGARC_REG_RMONAR               0xfffffeda
#define CYGARC_REG_RCR1                 0xfffffedc
#define CYGARC_REG_RCR2                 0xfffffede


#define CYGARC_REG_RCR1_CF              0x80 // carry flag
#define CYGARC_REG_RCR1_CIE             0x10 // carry interrupt enable
#define CYGARC_REG_RCR1_AIE             0x08 // alarm interrupt enable
#define CYGARC_REG_RCR1_AF              0x01 // alarm flag

#define CYGARC_REG_RCR2_PEF             0x80 // periodic interrupt flag
#define CYGARC_REG_RCR2_PES2            0x40 // periodic interrupt setting
#define CYGARC_REG_RCR2_PES1            0x20
#define CYGARC_REG_RCR2_PES0            0x10
#define CYGARC_REG_RCR2_RTCEN           0x08 // RTC enable
#define CYGARC_REG_RCR2_ADJ             0x04 // second adjustment
#define CYGARC_REG_RCR2_RESET           0x02 // reset
#define CYGARC_REG_RCR2_START           0x01 // start


