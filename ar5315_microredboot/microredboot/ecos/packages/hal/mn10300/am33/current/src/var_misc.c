//==========================================================================
//
//      var_misc.c
//
//      HAL CPU variant miscellaneous functions
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
// Author(s):    nickg
// Contributors: nickg, jlarmour, dmoseley
// Date:         1999-01-21
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_cache.h>

#include <cyg/hal/hal_intr.h>           // HAL_CLOCK_READ
#include CYGHWR_MEMORY_LAYOUT_H

/*------------------------------------------------------------------------*/
/* Variant specific initialization routine.                               */

void hal_variant_init(void)
{
    // Let the timer run at a default rate (for delays)
    HAL_CLOCK_INITIALIZE(CYGNUM_HAL_RTC_PERIOD);
}

/*------------------------------------------------------------------------*/
/* Cache functions.                                                       */

#if 0 //!defined(CYG_HAL_MN10300_SIM)
void cyg_hal_dcache_store(CYG_ADDRWORD base, int size)
{
    volatile register CYG_BYTE *way0 = HAL_DCACHE_PURGE_WAY0;
    volatile register CYG_BYTE *way1 = HAL_DCACHE_PURGE_WAY1;
    volatile register CYG_BYTE *way2 = HAL_DCACHE_PURGE_WAY2;
    volatile register CYG_BYTE *way3 = HAL_DCACHE_PURGE_WAY3;
    register int i;
    register CYG_ADDRWORD state;

    HAL_DCACHE_IS_ENABLED(state);
    if (state)
        HAL_DCACHE_DISABLE();

    way0 += base & 0x000003f0;
    way1 += base & 0x000003f0;
    way2 += base & 0x000003f0;
    way3 += base & 0x000003f0;
    for( i = 0; i < size; i += HAL_DCACHE_LINE_SIZE )
    {
        *(CYG_ADDRWORD *)way0 = 0;
        *(CYG_ADDRWORD *)way1 = 0;
        *(CYG_ADDRWORD *)way2 = 0;
        *(CYG_ADDRWORD *)way3 = 0;
        way0 += HAL_DCACHE_LINE_SIZE;
        way1 += HAL_DCACHE_LINE_SIZE;
        way2 += HAL_DCACHE_LINE_SIZE;
        way3 += HAL_DCACHE_LINE_SIZE;
    }
    if (state)
        HAL_DCACHE_ENABLE();
}
#endif

/*------------------------------------------------------------------------*/
/* Clock functions.                                                       */

cyg_uint32 __hal_period__;

// Delay for some usecs.
void
hal_delay_us(cyg_int32 delay)
{
#define _TICKS_PER_USEC (CYGHWR_HAL_MN10300_PROCESSOR_OSC/1000000)
    cyg_uint32 now, prev, diff, usecs;

    diff = usecs = 0;
    HAL_CLOCK_READ(&prev);

    while (delay > usecs) {
	HAL_CLOCK_READ(&now);

	if (now < prev)
	    diff += (now + (__hal_period__ - prev));
	else
	    diff += (now - prev);

	prev = now;

	if (diff >= _TICKS_PER_USEC) {
	    usecs += (diff / _TICKS_PER_USEC);
	    diff %= _TICKS_PER_USEC;
	}
    }
}


/*------------------------------------------------------------------------*/
/* Memory top support                                                     */

#define SDBASE0       ((volatile unsigned *)0xDA000008)
#define SDBASE1       ((volatile unsigned *)0xDA00000C)
#define SDRAMBUS      ((volatile unsigned *)0xDA000000)

#if CYGINT_HAL_MN10300_MEM_REAL_REGION_TOP
externC cyg_uint8 *
hal_mn10300_mem_real_region_top( cyg_uint8 *regionend )
{
    unsigned dram_size, dram_base;

    // Figure out actual DRAM size from memory controller config.
    dram_size = 0x400000 << ((*SDRAMBUS >> 16) & 0x3);
    if (*SDBASE1 & 1)
	dram_size *= 2;
    dram_base = (*SDBASE0 & ((*SDBASE0 & 0xfff0) << 16));

    CYG_ASSERT( dram_size >= 8<<20, "Less than 8MB SDRAM reported!" );
    CYG_ASSERT( dram_size <=  256<<20, "More than 256MB SDRAM reported!" );

    // is it the "normal" end of the DRAM region? If so, it should be
    // replaced by the real size
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE) ) {
        regionend = (cyg_uint8 *)dram_base + dram_size;
    }
    return regionend;
}
#endif


#ifdef CYGPKG_CYGMON
/*------------------------------------------------------------------------*/
/* GDB Register functions.                                                */
#include <cyg/hal/var_arch.h>
#include <cyg/hal/hal_stub.h>

#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
int fpu_regs_read = 0;
#endif

int msp_read = 0;

extern int *_registers_valid;

void am33_get_gdb_extra_registers(CYG_ADDRWORD *registers, HAL_SavedRegisters *regs)
{
  register CYG_ADDRWORD epsw;
  asm volatile ("  mov epsw, %0 " : "=r" (epsw) : );

  registers[15] = regs->e0;
  registers[16] = regs->e1;
  registers[17] = regs->e2;
  registers[18] = regs->e3;
  registers[19] = regs->e4;
  registers[20] = regs->e5;
  registers[21] = regs->e6;
  registers[22] = regs->e7;

  registers[26] = regs->mcrh;
  registers[27] = regs->mcrl;
  registers[28] = regs->mcvf;

  registers[14] = regs->mdrq;

  {
    register CYG_ADDRWORD ssp, usp, msp;
    asm volatile (" mov usp,  %0 " : "=a" (usp) : );
    asm volatile (" mov ssp,  %0 " : "=a" (ssp) : );
    if ((epsw & HAL_ARCH_AM33_PSW_ML) == HAL_ARCH_AM33_PSW_ML)
      {
        // We are running in Monitor mode.  Go ahead and read the MSP.
        asm volatile (" mov msp, %0 " : "=a" (msp) : );
        msp_read = 1;
      } else {
        msp = 0;
        msp_read = 0;
      }

    // Now we need to determine which sp was in effect when we hit this exception,
    // since we want the register image to reflect the state at the time of the
    // exception.
    if ((regs->psw & HAL_ARCH_AM33_PSW_ML) == HAL_ARCH_AM33_PSW_ML)
      {
        msp = regs->sp;
      }
    else if ((regs->psw & HAL_ARCH_AM33_PSW_nSL) == 0)
      {
        ssp = regs->sp;
      }
    else
      {
        usp = regs->sp;
      }

    registers[23] = ssp;
    registers[24] = msp;
    registers[25] = usp;
  }

#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
  if ((regs->psw & HAL_ARCH_AM33_PSW_FE) == HAL_ARCH_AM33_PSW_FE)
    {
      // The FPU is enabled.  Go ahead and read the registers
      asm volatile (" fmov fpcr, %0  " : "=d" (registers[29]) : );
      asm volatile (" fmov fs0,  %0  " : "=m" (registers[32]) : );
      asm volatile (" fmov fs1,  %0  " : "=m" (registers[33]) : );
      asm volatile (" fmov fs2,  %0  " : "=m" (registers[34]) : );
      asm volatile (" fmov fs3,  %0  " : "=m" (registers[35]) : );
      asm volatile (" fmov fs4,  %0  " : "=m" (registers[36]) : );
      asm volatile (" fmov fs5,  %0  " : "=m" (registers[37]) : );
      asm volatile (" fmov fs6,  %0  " : "=m" (registers[38]) : );
      asm volatile (" fmov fs7,  %0  " : "=m" (registers[39]) : );
      asm volatile (" fmov fs8,  %0  " : "=m" (registers[40]) : );
      asm volatile (" fmov fs9,  %0  " : "=m" (registers[41]) : );
      asm volatile (" fmov fs10, %0  " : "=m" (registers[42]) : );
      asm volatile (" fmov fs11, %0  " : "=m" (registers[43]) : );
      asm volatile (" fmov fs12, %0  " : "=m" (registers[44]) : );
      asm volatile (" fmov fs13, %0  " : "=m" (registers[45]) : );
      asm volatile (" fmov fs14, %0  " : "=m" (registers[46]) : );
      asm volatile (" fmov fs15, %0  " : "=m" (registers[47]) : );
      asm volatile (" fmov fs16, %0  " : "=m" (registers[48]) : );
      asm volatile (" fmov fs17, %0  " : "=m" (registers[49]) : );
      asm volatile (" fmov fs18, %0  " : "=m" (registers[50]) : );
      asm volatile (" fmov fs19, %0  " : "=m" (registers[51]) : );
      asm volatile (" fmov fs20, %0  " : "=m" (registers[52]) : );
      asm volatile (" fmov fs21, %0  " : "=m" (registers[53]) : );
      asm volatile (" fmov fs22, %0  " : "=m" (registers[54]) : );
      asm volatile (" fmov fs23, %0  " : "=m" (registers[55]) : );
      asm volatile (" fmov fs24, %0  " : "=m" (registers[56]) : );
      asm volatile (" fmov fs25, %0  " : "=m" (registers[57]) : );
      asm volatile (" fmov fs26, %0  " : "=m" (registers[58]) : );
      asm volatile (" fmov fs27, %0  " : "=m" (registers[59]) : );
      asm volatile (" fmov fs28, %0  " : "=m" (registers[60]) : );
      asm volatile (" fmov fs29, %0  " : "=m" (registers[61]) : );
      asm volatile (" fmov fs30, %0  " : "=m" (registers[62]) : );
      asm volatile (" fmov fs31, %0  " : "=m" (registers[63]) : );
      fpu_regs_read = 1;
    } else {
      fpu_regs_read = 0;
    }
#endif

#ifdef CYGHWR_REGISTER_VALIDITY_CHECKING
  {
    int i;

    // Initially set all registers to valid
    for (i = 0; i < NUMREGS; i++)
      _registers_valid[i] = 1;

    if (msp_read == 0)
      _registers_valid[MSP] = 0;

    if (fpu_regs_read == 0)
      {
        for (i = FP_START; i <= FP_END; i++)
          _registers_valid[i] = 0;
      }
  }
#endif
}

void am33_set_gdb_extra_registers(CYG_ADDRWORD *registers, HAL_SavedRegisters *regs)
{
  regs->e0 = registers[15];
  regs->e1 = registers[16];
  regs->e2 = registers[17];
  regs->e3 = registers[18];
  regs->e4 = registers[19];
  regs->e5 = registers[20];
  regs->e6 = registers[21];
  regs->e7 = registers[22];

  regs->mcrh = registers[26];
  regs->mcrl = registers[27];
  regs->mcvf = registers[28];

  regs->mdrq = registers[14];

  {
    register CYG_ADDRWORD ssp, usp, msp;
    ssp = registers[23];
    msp = registers[24];
    usp = registers[25];
    if ((registers[11] & HAL_ARCH_AM33_PSW_ML) == HAL_ARCH_AM33_PSW_ML)
      {
        // We were running in monitor mode.
        // Go ahead and manually restore ssp and usp.
        // msp will be restored by the rti.
        asm volatile (" mov %0, usp " : : "a" (usp));
        asm volatile (" mov %0, ssp " : : "a" (ssp));
      }
    else if ((registers[11] & HAL_ARCH_AM33_PSW_nSL) == 0)
      {
        // We were running in system mode.
        // Go ahead and manually restore msp and usp.
        // ssp will be restored by the rti.
        asm volatile (" mov %0, usp " : : "a" (usp));
        if (msp_read)
          {
          asm volatile (" mov %0, msp " : : "a" (msp));
          }
      }
    else
      {
        // We were running in user mode.
        // Go ahead and manually restore msp and ssp.
        // usp will be restored by the rti.
        asm volatile (" mov %0, ssp " : : "a" (ssp));
        if (msp_read)
          {
          asm volatile (" mov %0, msp " : : "a" (msp));
          }
      }
  }

#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
  if (fpu_regs_read)
    {
      // The FPU registers have been read and the FPU is enabled.
      // Go ahead and write the registers
      asm volatile (" fmov %0, fpcr  " : : "d" (registers[29]) );
      asm volatile (" fmov %0, fs0   " : : "m" (registers[32]) );
      asm volatile (" fmov %0, fs1   " : : "m" (registers[33]) );
      asm volatile (" fmov %0, fs2   " : : "m" (registers[34]) );
      asm volatile (" fmov %0, fs3   " : : "m" (registers[35]) );
      asm volatile (" fmov %0, fs4   " : : "m" (registers[36]) );
      asm volatile (" fmov %0, fs5   " : : "m" (registers[37]) );
      asm volatile (" fmov %0, fs6   " : : "m" (registers[38]) );
      asm volatile (" fmov %0, fs7   " : : "m" (registers[39]) );
      asm volatile (" fmov %0, fs8   " : : "m" (registers[40]) );
      asm volatile (" fmov %0, fs9   " : : "m" (registers[41]) );
      asm volatile (" fmov %0, fs10  " : : "m" (registers[42]) );
      asm volatile (" fmov %0, fs11  " : : "m" (registers[43]) );
      asm volatile (" fmov %0, fs12  " : : "m" (registers[44]) );
      asm volatile (" fmov %0, fs13  " : : "m" (registers[45]) );
      asm volatile (" fmov %0, fs14  " : : "m" (registers[46]) );
      asm volatile (" fmov %0, fs15  " : : "m" (registers[47]) );
      asm volatile (" fmov %0, fs16  " : : "m" (registers[48]) );
      asm volatile (" fmov %0, fs17  " : : "m" (registers[49]) );
      asm volatile (" fmov %0, fs18  " : : "m" (registers[50]) );
      asm volatile (" fmov %0, fs19  " : : "m" (registers[51]) );
      asm volatile (" fmov %0, fs20  " : : "m" (registers[52]) );
      asm volatile (" fmov %0, fs21  " : : "m" (registers[53]) );
      asm volatile (" fmov %0, fs22  " : : "m" (registers[54]) );
      asm volatile (" fmov %0, fs23  " : : "m" (registers[55]) );
      asm volatile (" fmov %0, fs24  " : : "m" (registers[56]) );
      asm volatile (" fmov %0, fs25  " : : "m" (registers[57]) );
      asm volatile (" fmov %0, fs26  " : : "m" (registers[58]) );
      asm volatile (" fmov %0, fs27  " : : "m" (registers[59]) );
      asm volatile (" fmov %0, fs28  " : : "m" (registers[60]) );
      asm volatile (" fmov %0, fs29  " : : "m" (registers[61]) );
      asm volatile (" fmov %0, fs30  " : : "m" (registers[62]) );
      asm volatile (" fmov %0, fs31  " : : "m" (registers[63]) );
    }
#endif
}
#endif // CYGPKG_CYGMON

/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
