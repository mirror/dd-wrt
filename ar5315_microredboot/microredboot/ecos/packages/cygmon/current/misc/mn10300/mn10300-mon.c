//==========================================================================
//
//      mn10300-mon.c
//
//      Support code to extend the generic monitor code to support
//      MN10300 processors.
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
// Purpose:      Support code to extend the generic monitor code to support
//               MN10300 processors.
// Description:  Further board specific support is in other files.
//               This file contains:
//                 register names lookup table
//
//               Empty Stubs:
//                 Interval timer - This should really belong to the application
//                 operating system.
//
//               Should not contain:
//                 low level uart getchar and putchar functions
//                 delay function to support uart
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include "monitor.h"

struct regstruct regtab[] =
{
    {"d0",       D0,          1},
    {"d1",       D1,          1},
    {"d2",       D2,          1},
    {"d3",       D3,          1},
    {"a0",       A0,          1},
    {"a1",       A1,          1},
    {"a2",       A2,          1},
    {"a3",       A3,          1},
    {"sp",       SP,          1},
    {"pc",       PC,          1},
    {"mdr",      MDR,         1},
    {"psw",      PSW,         1},
    {"lir",      LIR,         1},
    {"lar",      LAR,         1},
#ifdef CYGPKG_HAL_MN10300_AM33
    {"r0",       R0,          1},
    {"r1",       R1,          1},
    {"r2",       R2,          1},
    {"r3",       R3,          1},
    {"r4",       R4,          1},
    {"r5",       R5,          1},
    {"r6",       R6,          1},
    {"r7",       R7,          1},
    {"ssp",      SSP,         1},
    {"msp",      MSP,         1},
    {"usp",      USP,         1},
    {"mcrh",     MCRH,        1},
    {"mcrl",     MCRL,        1},
    {"mcvf",     MCVF,        1},
    {"mdrq",     MDRQ,        1},
#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
    {"fpcr",     FPCR,        1},
    {"fs0",      FS0,         1},
    {"fs1",      FS1,         1},
    {"fs2",      FS2,         1},
    {"fs3",      FS3,         1},
    {"fs4",      FS4,         1},
    {"fs5",      FS5,         1},
    {"fs6",      FS6,         1},
    {"fs7",      FS7,         1},
    {"fs8",      FS8,         1},
    {"fs9",      FS9,         1},
    {"fs10",     FS10,        1},
    {"fs11",     FS11,        1},
    {"fs12",     FS12,        1},
    {"fs13",     FS13,        1},
    {"fs14",     FS14,        1},
    {"fs15",     FS15,        1},
    {"fs16",     FS16,        1},
    {"fs17",     FS17,        1},
    {"fs18",     FS18,        1},
    {"fs19",     FS19,        1},
    {"fs20",     FS20,        1},
    {"fs21",     FS21,        1},
    {"fs22",     FS22,        1},
    {"fs23",     FS23,        1},
    {"fs24",     FS24,        1},
    {"fs25",     FS25,        1},
    {"fs26",     FS26,        1},
    {"fs27",     FS27,        1},
    {"fs28",     FS28,        1},
    {"fs29",     FS29,        1},
    {"fs30",     FS30,        1},
    {"fs31",     FS31,        1},
#endif
#endif  
    { 0,         0,           1}, /* Terminating element must be last */
} ;

void
initialize_mon(void)
{
} /* initialize_mon */

#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
extern int fpu_regs_read;
#endif

#ifdef CYGPKG_HAL_MN10300_AM33
extern int msp_read;
#endif

void
initialize_mon_each_time(void)
{
  int i;
#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
  // Make sure the regtab[] indicates the valid status of the FPU registers
  for (i = 0; regtab[i].registername != NULL; i++)
    {
      if ((regtab[i].registernumber >= FP_START) && (regtab[i].registernumber <= FP_END))
        regtab[i].registervalid = fpu_regs_read;
    }
#endif

#ifdef CYGPKG_HAL_MN10300_AM33
  // Make sure the regtab[] indicates the valid status of the MSP
  for (i = 0; regtab[i].registername != NULL; i++)
    {
      if (regtab[i].registernumber == MSP)
          regtab[i].registervalid = msp_read;
    }
#endif
} /* initialize_mon_each_time */


#include <cyg/hal/hal_arch.h>
#include <bsp/common/bsp_if.h>
int
machine_syscall(HAL_SavedRegisters *regs)
{
    int res, err;
    target_register_t d0, d1, d2, d3;

    d0 = get_register(D0);
    d1 = get_register(D1);
    d2 = get_register(D2);
    d3 = get_register(D3);

    err = _bsp_do_syscall(d0, // Function
                          d1, d2, d3, 0, // arguments,
                          &res);
    if (err)
    {
        // This was a syscall.  It has now been handled, so update the registers appropriately
        put_register(D0, res);
        bsp_skip_instruction(regs);
    }

    return err;
}


// Utility function for printing breakpoints
void bp_print(target_register_t bp_val)
{
    bsp_printf("0x%08lx\n", (unsigned long)bp_val);
}
