#ifndef CYGONCE_VAR_INTR_H
#define CYGONCE_VAR_INTR_H

//==========================================================================
//
//      var_intr.h
//
//      HAL variant interrupt and clock support
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
// Author(s):    jskov
// Contributors: jskov,
// Date:         1999-04-24
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock.
//              
// Usage:
//               #include <cyg/hal/hal_intr.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include CYGBLD_HAL_CPU_MODULES_H       // INTC module selection

//--------------------------------------------------------------------------
// Optional platform overrides and fallbacks
#include <cyg/hal/plf_intr.h>

#ifndef CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF
# define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)                  \
    case CYGNUM_HAL_INTERRUPT_NMI:                                          \
        /* fall through */                                                  \
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL14:          \
        /* Cannot change levels */                                          \
        break;                                                           
#endif

#ifndef CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF
# define CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vec)
#endif

#ifndef CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF
# define CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vec, level, up)
#endif

//----------------------------------------------------------------------------
// Additional vectors provided by INTC V2

#if (CYGARC_SH_MOD_INTC >= 2)
#define CYGNUM_HAL_INTERRUPT_RESERVED_5C0    32
#define CYGNUM_HAL_INTERRUPT_HUDI_HUDI       33
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ0        34
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ1        35
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ2        36
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ3        37
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ4        38
#define CYGNUM_HAL_INTERRUPT_IRQ_IRQ5        39
#define CYGNUM_HAL_INTERRUPT_RESERVED_6C0    40
#define CYGNUM_HAL_INTERRUPT_RESERVED_6E0    41
#define CYGNUM_HAL_INTERRUPT_PINT_PINT07     42
#define CYGNUM_HAL_INTERRUPT_PINT_PINT8F     43
#define CYGNUM_HAL_INTERRUPT_RESERVED_740    44
#define CYGNUM_HAL_INTERRUPT_RESERVED_760    45
#define CYGNUM_HAL_INTERRUPT_RESERVED_780    46
#define CYGNUM_HAL_INTERRUPT_RESERVED_7A0    47
#define CYGNUM_HAL_INTERRUPT_RESERVED_7C0    48
#define CYGNUM_HAL_INTERRUPT_RESERVED_7E0    59
#define CYGNUM_HAL_INTERRUPT_DMAC_DEI0       50
#define CYGNUM_HAL_INTERRUPT_DMAC_DEI1       51
#define CYGNUM_HAL_INTERRUPT_DMAC_DEI2       52
#define CYGNUM_HAL_INTERRUPT_DMAC_DEI3       53
#define CYGNUM_HAL_INTERRUPT_IRDA_ERI1       54
#define CYGNUM_HAL_INTERRUPT_IRDA_RXI1       55
#define CYGNUM_HAL_INTERRUPT_IRDA_BRI1       56
#define CYGNUM_HAL_INTERRUPT_IRDA_TXI1       57
#define CYGNUM_HAL_INTERRUPT_SCIF_ERI2       58
#define CYGNUM_HAL_INTERRUPT_SCIF_RXI2       59
#define CYGNUM_HAL_INTERRUPT_SCIF_BRI2       60
#define CYGNUM_HAL_INTERRUPT_SCIF_TXI2       61
#define CYGNUM_HAL_INTERRUPT_ADC_ADI         62

#undef  CYGNUM_HAL_ISR_MAX
#ifdef CYGNUM_HAL_ISR_PLF_MAX
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_ISR_PLF_MAX
#else
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_INTERRUPT_ADC_ADI
#endif

#endif // CYGARC_SH_MOD_INTC >= 2

//----------------------------------------------------------------------------
// Additional vectors provided by INTC V3

#if (CYGARC_SH_MOD_INTC >= 3)
#define CYGNUM_HAL_INTERRUPT_LCDC_LCDI       63
#define CYGNUM_HAL_INTERRUPT_PCC_PCC0        64
#define CYGNUM_HAL_INTERRUPT_PCC_PCC1        65

#undef  CYGNUM_HAL_ISR_MAX
#ifdef CYGNUM_HAL_ISR_PLF_MAX
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_ISR_PLF_MAX
#else
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_INTERRUPT_PCC_PCC1
#endif

#endif // CYGARC_SH_MOD_INTC >= 3


//----------------------------------------------------------------------------
// Platform may provide extensions to the interrupt configuration functions
// via these macros. The first macro is put inside the functions's
// switch statements, the last two called as functions.
#ifndef CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF
# define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)
# define CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vec)
# define CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vec)
#endif

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_INTR_H
// End of var_intr.h
