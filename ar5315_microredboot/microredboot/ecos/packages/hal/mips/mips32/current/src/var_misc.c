//==========================================================================
//
//      var_misc.c
//
//      HAL implementation miscellaneous functions
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
// Date:         2000-07-14
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

#include <cyg/hal/hal_intr.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/var_arch.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_cache.h>

/*------------------------------------------------------------------------*/
// Array which stores the configured priority levels for the configured
// interrupts.

volatile CYG_BYTE hal_interrupt_level[CYGNUM_HAL_ISR_COUNT];

/*------------------------------------------------------------------------*/

void hal_variant_init(void)
{
}

/*
 * Uncomment the following to allow for dynamic cache sizing.
 * Currently we are going to assume the exact part specified in the ecosconfig stuff.
 * Perhaps in the near future this can all be done dynamically.
 */
/* define DYNAMIC_CACHE_SIZING */

#if 0
#ifndef DYNAMIC_CACHE_SIZING
#warning "                                                                           \n\
STILL NEED TO IMPLEMENT DYNAMIC_CACHE_SIZING.                                        \n\
ALSO, the HAL_PLATFORM_CPU/etc defines need to be dynamic.                           \n\
ALSO, need to do big endian stuff as well.                                           \n\
Determine if network debug is necessary.                                             \n\
Remove MIPS memc_init code"
#endif
#endif

/*------------------------------------------------------------------------*/
// Initialize the caches

int hal_init_icache(unsigned long config1_val)
{
#ifdef DYNAMIC_CACHE_SIZING
  int icache_linesize, icache_assoc, icache_sets, icache_lines, icache_size;
  unsigned long cache_addr;

  switch (config1_val & CONFIG1_IL)
    {
    case CONFIG1_ICACHE_LINE_SIZE_16_BYTES: icache_linesize = 16;      break;
    case CONFIG1_ICACHE_NOT_PRESET:         return -1;                 break;
    default:      /* Error */               return -1;                 break;
    }

  switch (config1_val & CONFIG1_IA)
    {
    case CONFIG1_ICACHE_DIRECT_MAPPED:      icache_assoc = 1;          break;
    case CONFIG1_ICACHE_2_WAY:              icache_assoc = 2;          break;
    case CONFIG1_ICACHE_3_WAY:              icache_assoc = 3;          break;
    case CONFIG1_ICACHE_4_WAY:              icache_assoc = 4;          break;
    default:      /* Error */               return -1;                 break;
    }

  switch (config1_val & CONFIG1_IS)
    {
    case CONFIG1_ICACHE_64_SETS_PER_WAY:    icache_sets = 64;          break;
    case CONFIG1_ICACHE_128_SETS_PER_WAY:   icache_sets = 128;         break;
    case CONFIG1_ICACHE_256_SETS_PER_WAY:   icache_sets = 256;         break;
    default:      /* Error */               return -1;                 break;
    }

  icache_lines = icache_sets * icache_assoc;
  icache_size = icache_lines * icache_linesize;
#endif /* DYNAMIC_CACHE_SIZING */

  /*
   * Reset does not invalidate the cache so let's do so now.
   */
  HAL_ICACHE_INVALIDATE_ALL();

#ifdef DYNAMIC_CACHE_SIZING
  return icache_size;
#else
  return HAL_ICACHE_SIZE;
#endif
}

int hal_init_dcache(unsigned long config1_val)
{
#ifdef DYNAMIC_CACHE_SIZING
  int dcache_linesize, dcache_assoc, dcache_sets, dcache_lines, dcache_size;

  switch (config1_val & CONFIG1_DL)
    {
    case CONFIG1_DCACHE_LINE_SIZE_16_BYTES: dcache_linesize = 16;      break;
    case CONFIG1_DCACHE_NOT_PRESET:         return -1;                 break;
    default:      /* Error */               return -1;                 break;
    }

  switch (config1_val & CONFIG1_DA)
    {
    case CONFIG1_DCACHE_DIRECT_MAPPED:      dcache_assoc = 1;          break;
    case CONFIG1_DCACHE_2_WAY:              dcache_assoc = 2;          break;
    case CONFIG1_DCACHE_3_WAY:              dcache_assoc = 3;          break;
    case CONFIG1_DCACHE_4_WAY:              dcache_assoc = 4;          break;
    default:      /* Error */               return -1;                 break;
    }

  switch (config1_val & CONFIG1_DS)
    {
    case CONFIG1_DCACHE_64_SETS_PER_WAY:    dcache_sets = 64;          break;
    case CONFIG1_DCACHE_128_SETS_PER_WAY:   dcache_sets = 128;         break;
    case CONFIG1_DCACHE_256_SETS_PER_WAY:   dcache_sets = 256;         break;
    default:      /* Error */               return -1;                 break;
    }

  dcache_lines = dcache_sets * dcache_assoc;
  dcache_size = dcache_lines * dcache_linesize;
#endif /* DYNAMIC_CACHE_SIZING */

  /*
   * Reset does not invalidate the cache so let's do so now.
   */
  HAL_DCACHE_INVALIDATE_ALL();

#ifdef DYNAMIC_CACHE_SIZING
  return dcache_size;
#else
  return HAL_DCACHE_SIZE;
#endif
}

void hal_c_cache_init(unsigned long config1_val)
{
  volatile unsigned val;

  if (hal_init_icache(config1_val) == -1)
    {
        /* Error */
        ;
    }

  if (hal_init_dcache(config1_val) == -1)
    {
        /* Error */
        ;
    }

  // enable cached KSEG0
  asm volatile("mfc0 %0,$16;" : "=r"(val));
  val &= ~3;
  asm volatile("mtc0 %0,$16;" : : "r"(val));
}

void hal_icache_sync(void)
{
    HAL_ICACHE_INVALIDATE_ALL();
}

void hal_dcache_sync(void)
{
    HAL_DCACHE_INVALIDATE_ALL();
}

/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
