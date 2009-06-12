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

#ifndef CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF
# define CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vec)
#endif

#ifndef CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF
# define CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vec, level, up)
#endif

//----------------------------------------------------------------------------
// Additional vectors provided by INTC V1

#if (CYGARC_SH_MOD_INTC == 1)

#ifndef CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF
# define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)                  \
    case CYGNUM_HAL_INTERRUPT_NMI:                                          \
        /* fall through */                                                  \
    case CYGNUM_HAL_INTERRUPT_LVL4 ... CYGNUM_HAL_INTERRUPT_LVL_MAX:        \
        /* Cannot change levels */                                          \
        break;                                                           
#endif

#define CYGNUM_HAL_INTERRUPT_LVL8           9 
#define CYGNUM_HAL_INTERRUPT_LVL9          10
#define CYGNUM_HAL_INTERRUPT_LVL10         11
#define CYGNUM_HAL_INTERRUPT_LVL11         12
#define CYGNUM_HAL_INTERRUPT_LVL12         13
#define CYGNUM_HAL_INTERRUPT_LVL13         14
#define CYGNUM_HAL_INTERRUPT_LVL14         15
#define CYGNUM_HAL_INTERRUPT_DMAC0_TE      16
#define CYGNUM_HAL_INTERRUPT_DMAC1_TE      17
#define CYGNUM_HAL_INTERRUPT_WDT_ITI       18
#define CYGNUM_HAL_INTERRUPT_REF_CMI       19
#define CYGNUM_HAL_INTERRUPT_EDMAC_EINT    20
#define CYGNUM_HAL_INTERRUPT_FRT_ICI       21
#define CYGNUM_HAL_INTERRUPT_FRT_OCI       22
#define CYGNUM_HAL_INTERRUPT_FRT_OVI       23
#define CYGNUM_HAL_INTERRUPT_TPU0_TGI0A    24
#define CYGNUM_HAL_INTERRUPT_TPU0_TGI0B    25
#define CYGNUM_HAL_INTERRUPT_TPU0_TGI0C    26
#define CYGNUM_HAL_INTERRUPT_TPU0_TGI0D    27
#define CYGNUM_HAL_INTERRUPT_TPU0_TGI0V    28
#define CYGNUM_HAL_INTERRUPT_TPU1_TGI1A    29
#define CYGNUM_HAL_INTERRUPT_TPU1_TGI1B    30
#define CYGNUM_HAL_INTERRUPT_TPU1_TGI1V    31
#define CYGNUM_HAL_INTERRUPT_TPU1_TGI1U    32
#define CYGNUM_HAL_INTERRUPT_TPU2_TGI2A    33
#define CYGNUM_HAL_INTERRUPT_TPU2_TGI2B    34
#define CYGNUM_HAL_INTERRUPT_TPU2_TGI2V    35
#define CYGNUM_HAL_INTERRUPT_TPU2_TGI2U    36
#define CYGNUM_HAL_INTERRUPT_SCIF1_ERI1    37
#define CYGNUM_HAL_INTERRUPT_SCIF1_RXI1    38
#define CYGNUM_HAL_INTERRUPT_SCIF1_BRI1    39
#define CYGNUM_HAL_INTERRUPT_SCIF1_TXI1    40
#define CYGNUM_HAL_INTERRUPT_SCIF2_ERI2    41
#define CYGNUM_HAL_INTERRUPT_SCIF2_RXI2    42
#define CYGNUM_HAL_INTERRUPT_SCIF2_BRI2    43
#define CYGNUM_HAL_INTERRUPT_SCIF2_TXI2    44
#define CYGNUM_HAL_INTERRUPT_SIO0_RERI0    45
#define CYGNUM_HAL_INTERRUPT_SIO0_TERI0    46
#define CYGNUM_HAL_INTERRUPT_SIO0_RDFI0    47
#define CYGNUM_HAL_INTERRUPT_SIO0_TDEI0    48
#define CYGNUM_HAL_INTERRUPT_SIO1_RERI1    49
#define CYGNUM_HAL_INTERRUPT_SIO1_TERI1    50
#define CYGNUM_HAL_INTERRUPT_SIO1_RDFI1    51
#define CYGNUM_HAL_INTERRUPT_SIO1_TDEI1    52
#define CYGNUM_HAL_INTERRUPT_SIO2_RERI2    53
#define CYGNUM_HAL_INTERRUPT_SIO2_TERI2    54
#define CYGNUM_HAL_INTERRUPT_SIO2_RDFI2    55
#define CYGNUM_HAL_INTERRUPT_SIO2_TDEI2    56

#define CYGNUM_HAL_INTERRUPT_LVL_MAX       CYGNUM_HAL_INTERRUPT_LVL14

#undef  CYGNUM_HAL_ISR_MAX
#ifdef CYGNUM_HAL_ISR_PLF_MAX
# define CYGNUM_HAL_ISR_MAX                CYGNUM_HAL_ISR_PLF_MAX
#else
# define CYGNUM_HAL_ISR_MAX                CYGNUM_HAL_INTERRUPT_SIO2_TDEI2
#endif

// The vector used by the Real time clock
#ifndef CYGNUM_HAL_INTERRUPT_RTC
# define CYGNUM_HAL_INTERRUPT_RTC             CYGNUM_HAL_INTERRUPT_FRT_OCI
#endif

#elif (CYGARC_SH_MOD_INTC == 2)

#ifndef CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF
# define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)                  \
    case CYGNUM_HAL_INTERRUPT_NMI:                                          \
        /* Cannot change levels */                                          \
        break;                                                           
#endif

#define CYGNUM_HAL_INTERRUPT_DMAC0_DEI0         9 
#define CYGNUM_HAL_INTERRUPT_DMAC1_DEI1        10
#define CYGNUM_HAL_INTERRUPT_DMAC2_DEI2        11
#define CYGNUM_HAL_INTERRUPT_DMAC3_DEI3        12
#define CYGNUM_HAL_INTERRUPT_MTU0_TGI0A        13
#define CYGNUM_HAL_INTERRUPT_MTU0_TGI0B        14
#define CYGNUM_HAL_INTERRUPT_MTU0_TGI0C        15
#define CYGNUM_HAL_INTERRUPT_MTU0_TGI0D        16
#define CYGNUM_HAL_INTERRUPT_MTU0_TGI0V        17
#define CYGNUM_HAL_INTERRUPT_MTU1_TGI1A        18
#define CYGNUM_HAL_INTERRUPT_MTU1_TGI1B        19
#define CYGNUM_HAL_INTERRUPT_MTU1_TGI1V        20
#define CYGNUM_HAL_INTERRUPT_MTU1_TGI1U        21
#define CYGNUM_HAL_INTERRUPT_MTU2_TGI2A        22
#define CYGNUM_HAL_INTERRUPT_MTU2_TGI2B        23
#define CYGNUM_HAL_INTERRUPT_MTU2_TGI2V        24
#define CYGNUM_HAL_INTERRUPT_MTU2_TGI2U        25
#define CYGNUM_HAL_INTERRUPT_MTU3_TGI3A        26
#define CYGNUM_HAL_INTERRUPT_MTU3_TGI3B        27
#define CYGNUM_HAL_INTERRUPT_MTU3_TGI3C        28
#define CYGNUM_HAL_INTERRUPT_MTU3_TGI3D        29
#define CYGNUM_HAL_INTERRUPT_MTU3_TGI3V        30
#define CYGNUM_HAL_INTERRUPT_MTU4_TGI4A        31
#define CYGNUM_HAL_INTERRUPT_MTU4_TGI4B        32
#define CYGNUM_HAL_INTERRUPT_MTU4_TGI4C        33
#define CYGNUM_HAL_INTERRUPT_MTU4_TGI4D        34
#define CYGNUM_HAL_INTERRUPT_MTU4_TGI4V        35
#define CYGNUM_HAL_INTERRUPT_SCI0_ERI0         36
#define CYGNUM_HAL_INTERRUPT_SCI0_RXI0         37
#define CYGNUM_HAL_INTERRUPT_SCI0_TXI0         38
#define CYGNUM_HAL_INTERRUPT_SCI0_TEI0         39
#define CYGNUM_HAL_INTERRUPT_SCI1_ERI1         40
#define CYGNUM_HAL_INTERRUPT_SCI1_RXI1         41
#define CYGNUM_HAL_INTERRUPT_SCI1_TXI1         42
#define CYGNUM_HAL_INTERRUPT_SCI1_TEI1         43
#define CYGNUM_HAL_INTERRUPT_AD_ADI0           44
#define CYGNUM_HAL_INTERRUPT_AD_ADI1           45
#define CYGNUM_HAL_INTERRUPT_DTC_SWDTCE        46
#define CYGNUM_HAL_INTERRUPT_CMT0_CMI0         47
#define CYGNUM_HAL_INTERRUPT_CMT1_CMI1         48
#define CYGNUM_HAL_INTERRUPT_WDT_ITI           49
#define CYGNUM_HAL_INTERRUPT_BSC_CMI           50
#define CYGNUM_HAL_INTERRUPT_IO_OEI            51
#define CYGNUM_HAL_INTERRUPT_VAR_RESERVED      52       // just to catch bad assignments

#undef  CYGNUM_HAL_ISR_MAX
#ifdef CYGNUM_HAL_ISR_PLF_MAX
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_ISR_PLF_MAX
#else
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_INTERRUPT_VAR_RESERVED
#endif

//                           eCos vector this entry decodes to , actual HW interrupt vector
#define CYGHWR_HAL_SH_SH2_CUSTOM_INTERRUPT_LAYOUT                         \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL0         ,  1 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL1         ,  2 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL2         ,  3 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL3         ,  4 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL4         ,  5 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL5         ,  6 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL6         ,  7 ;     \
        exception_vector_int CYGNUM_HAL_INTERRUPT_LVL7         ,  8 ;     \
                                                                          \
        exception_vector_int CYGNUM_HAL_INTERRUPT_DMAC0_DEI0   ,  9  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 10  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 11  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 12  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_DMAC1_DEI1   , 13  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 14  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 15  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 16  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_DMAC2_DEI2   , 17  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 18  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 19  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 20  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_DMAC3_DEI3   , 21  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 22  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 23  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 24  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU0_TGI0A   , 25  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU0_TGI0B   , 26  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU0_TGI0C   , 27  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU0_TGI0D   , 28  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU0_TGI0V   , 29  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 30  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 31  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 32  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU1_TGI1A   , 33  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU1_TGI1B   , 34  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 35  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 36  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU1_TGI1V   , 37  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU1_TGI1U   , 38  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 39  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 40  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU2_TGI2A   , 41  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU2_TGI2B   , 42  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 43  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 44  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU2_TGI2V   , 45  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU2_TGI2U   , 46  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 47  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 48  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU3_TGI3A   , 49  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU3_TGI3B   , 50  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU3_TGI3C   , 51  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU3_TGI3D   , 52  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU3_TGI3V   , 53  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 54  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 55  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 56  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU4_TGI4A   , 57  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU4_TGI4B   , 58  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU4_TGI4C   , 59  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU4_TGI4D   , 60  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_MTU4_TGI4V   , 61  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 62  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 63  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 64  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI0_ERI0    , 65  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI0_RXI0    , 66  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI0_TXI0    , 67  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI0_TEI0    , 68  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI1_ERI1    , 69  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI1_RXI1    , 70  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI1_TXI1    , 71  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_SCI1_TEI1    , 72  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_AD_ADI0      , 73  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_AD_ADI1      , 74  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 75  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 76  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_DTC_SWDTCE   , 77  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 78  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 79  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 80  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_CMT0_CMI0    , 81  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 82  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 83  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 84  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_CMT1_CMI1    , 85  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 86  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 87  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 88  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_WDT_ITI      , 89  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_BSC_CMI      , 90  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 91  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_VAR_RESERVED , 92  ;    \
        exception_vector_int CYGNUM_HAL_INTERRUPT_BSC_CMI      , 93  ;    \
        .set    vecno, CYGNUM_HAL_INTERRUPT_VAR_RESERVED             ;    \
        .rept   (256-CYGNUM_HAL_INTERRUPT_VAR_RESERVED)              ;    \
        exception_vector_int vecno                                   ;    \
        .set    vecno, vecno+1                                       ;    \
        .endr

// The vector used by the Real time clock
#ifndef CYGNUM_HAL_INTERRUPT_RTC
# define CYGNUM_HAL_INTERRUPT_RTC             CYGNUM_HAL_INTERRUPT_CMT0_CMI0
#endif

#else

# error "No vectors defined for this INTC type"

#endif

//----------------------------------------------------------------------------
// Platform may provide extensions to the interrupt configuration functions
// via these macros. The first macro is put inside the functions's
// switch statements, the last two called as functions.
#ifndef CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF
# define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)
# define CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vec)
# define CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vec)
#endif

#ifndef __ASSEMBLER__
//--------------------------------------------------------------------------
// Clock control, using Free-running Timer to match on compare register A

#include <cyg/hal/sh_regs.h>            // register definitions
#include <cyg/hal/hal_io.h>             // io macros
#include <cyg/infra/cyg_ass.h>          // CYG_FAIL

#if defined(CYGARC_SH_MOD_FRT)
#define HAL_CLOCK_VAR_INITIALIZE( _period_ )                                \
    CYG_MACRO_START                                                         \
    register cyg_uint8 _tier_;                                              \
                                                                            \
    /* Disable interrupts while programming the timer */                    \
    HAL_READ_UINT8(CYGARC_REG_TIER, _tier_);                                \
    HAL_WRITE_UINT8(CYGARC_REG_TIER, 0);                                    \
                                                                            \
    /* Clear counter register */                                            \
    HAL_WRITE_UINT8(CYGARC_REG_FRC, 0);                                     \
    /* Set compare A register */                                            \
    HAL_WRITE_UINT8(CYGARC_REG_TOCR, CYGARC_REG_TOCR_OLVLA);                \
    HAL_WRITE_UINT16(CYGARC_REG_OCR, _period_);                             \
                                                                            \
    /* Enable match A counter clear */                                      \
    HAL_WRITE_UINT8(CYGARC_REG_FTCSR, CYGARC_REG_FTCSR_CCLRA);              \
    /* Set interrupt prescaling */                                          \
    HAL_WRITE_UINT8(CYGARC_REG_TCR,                                         \
                     ((8==CYGHWR_HAL_SH_FRT_PRESCALE) ?                     \
                          CYGARC_REG_TCR_CLK_8 :                            \
                      (32==CYGHWR_HAL_SH_FRT_PRESCALE) ?                    \
                          CYGARC_REG_TCR_CLK_32:                            \
                      (128==CYGHWR_HAL_SH_FRT_PRESCALE) ?                   \
                          CYGARC_REG_TCR_CLK_128:CYGARC_REG_TCR_CLK_EXT));  \
                                                                            \
                                                                            \
    CYG_MACRO_END

// HAL_CLOCK_VAR_INITIALIZE is called from hal_variant_init since the
// FRT is also used by the hal_delay_us code.
#define HAL_CLOCK_INITIALIZE( _period_ )                                    \
    CYG_MACRO_START                                                         \
    register cyg_uint8 _tier_;                                              \
                                                                            \
    CYG_ASSERT((cyg_uint16)(_period_) < (cyg_uint16)0xfffe,                 \
               "period too large for 16bit FRT");                           \
                                                                            \
    /* Disable interrupts while programming the timer */                    \
    HAL_READ_UINT8(CYGARC_REG_TIER, _tier_);                                \
    HAL_WRITE_UINT8(CYGARC_REG_TIER, 0);                                    \
                                                                            \
    /* Set compare A register */                                            \
    HAL_WRITE_UINT8(CYGARC_REG_TOCR, CYGARC_REG_TOCR_OLVLA);                \
    HAL_WRITE_UINT16(CYGARC_REG_OCR, _period_);                             \
                                                                            \
    /* Enable interrupt */                                                  \
    _tier_ |= CYGARC_REG_TIER_OCIAE;                                        \
    HAL_WRITE_UINT8(CYGARC_REG_TIER, _tier_);                               \
                                                                            \
    CYG_MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ )   \
    CYG_MACRO_START                             \
    register cyg_uint8 _ftcsr_;                 \
                                                \
    /* Clear match flag */                      \
    HAL_READ_UINT8(CYGARC_REG_FTCSR, _ftcsr_);  \
    _ftcsr_ &= ~CYGARC_REG_FTCSR_OCFA;          \
    HAL_WRITE_UINT8(CYGARC_REG_FTCSR, _ftcsr_); \
                                                \
    CYG_MACRO_END

#define HAL_CLOCK_READ( _pvalue_ )              \
    CYG_MACRO_START                             \
    register cyg_uint16 _frc_;                  \
                                                \
    HAL_READ_UINT16(CYGARC_REG_FRC, _frc_);     \
                                                \
    *(_pvalue_) = _frc_;                        \
    CYG_MACRO_END

#elif defined(CYGARC_SH_MOD_CMT)

/* Compare Match Timer 0 is used for the eCos kernel RTC */
#define HAL_CLOCK_VAR_INITIALIZE( _period_ )                                \
    CYG_MACRO_START                                                         \
    register cyg_uint16 _tmp_;                                              \
                                                                            \
    /* Disable timer while programming it */                                \
    HAL_READ_UINT16(CYGARC_REG_CMSTR, _tmp_);                               \
    HAL_WRITE_UINT16(CYGARC_REG_CMSTR, _tmp_ & ~CYGARC_REG_CMSTR_STR0);     \
                                                                            \
    /* Clear counter register */                                            \
    HAL_WRITE_UINT16(CYGARC_REG_CMCNT0, 0);                                 \
    /* Set compare 0 register */                                            \
    HAL_WRITE_UINT16(CYGARC_REG_CMCOR0, _period_);                          \
                                                                            \
    /* Set prescaling and disable interrupts */                             \
    HAL_WRITE_UINT16(CYGARC_REG_CMCSR0,                                     \
                     ((8==CYGHWR_HAL_SH_CMT_PRESCALE) ?                     \
                          CYGARC_REG_CMCSR_CLK_8 :                          \
                      (32==CYGHWR_HAL_SH_CMT_PRESCALE) ?                    \
                          CYGARC_REG_CMCSR_CLK_32:                          \
                      (128==CYGHWR_HAL_SH_CMT_PRESCALE) ?                   \
                          CYGARC_REG_CMCSR_CLK_128:CYGARC_REG_CMCSR_CLK_512));  \
                                                                            \
    /* Start timer */                                                       \
    HAL_WRITE_UINT16(CYGARC_REG_CMSTR, _tmp_ | CYGARC_REG_CMSTR_STR0);      \
                                                                            \
    CYG_MACRO_END

// HAL_CLOCK_VAR_INITIALIZE is called from hal_variant_init since the
// CMT0 is also used by the hal_delay_us code.
#define HAL_CLOCK_INITIALIZE( _period_ )                                    \
    CYG_MACRO_START                                                         \
    register cyg_uint16 _tmp_;                                              \
                                                                            \
    CYG_ASSERT((cyg_uint16)(_period_) < (cyg_uint16)0xfffe,                 \
               "period too large for 16bit CMT");                           \
                                                                            \
    /* Disable timer while programming it */                                \
    HAL_READ_UINT16(CYGARC_REG_CMSTR, _tmp_);                               \
    HAL_WRITE_UINT16(CYGARC_REG_CMSTR, _tmp_ & ~CYGARC_REG_CMSTR_STR0);     \
                                                                            \
    /* Clear counter register */                                            \
    HAL_WRITE_UINT16(CYGARC_REG_CMCNT0, 0);                                 \
    /* Set compare 0 register */                                            \
    HAL_WRITE_UINT16(CYGARC_REG_CMCOR0, _period_);                          \
                                                                            \
    /* Set prescaling and enable interrupts */                              \
    HAL_WRITE_UINT16(CYGARC_REG_CMCSR0, CYGARC_REG_CMCSR_CMIE |             \
                     ((8==CYGHWR_HAL_SH_CMT_PRESCALE) ?                     \
                          CYGARC_REG_CMCSR_CLK_8 :                          \
                      (32==CYGHWR_HAL_SH_CMT_PRESCALE) ?                    \
                          CYGARC_REG_CMCSR_CLK_32:                          \
                      (128==CYGHWR_HAL_SH_CMT_PRESCALE) ?                   \
                          CYGARC_REG_CMCSR_CLK_128:CYGARC_REG_CMCSR_CLK_512));  \
                                                                            \
    /* Start timer */                                                       \
    HAL_WRITE_UINT16(CYGARC_REG_CMSTR, _tmp_ | CYGARC_REG_CMSTR_STR0);      \
                                                                            \
    CYG_MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ )   \
    CYG_MACRO_START                             \
    register cyg_uint16 _tmp_;                  \
                                                \
    /* Clear match flag */                      \
    HAL_READ_UINT16(CYGARC_REG_CMCSR0, _tmp_);  \
    _tmp_ &= ~CYGARC_REG_CMCSR_CMF;             \
    HAL_WRITE_UINT16(CYGARC_REG_CMCSR0, _tmp_); \
                                                \
    CYG_MACRO_END

#define HAL_CLOCK_READ( _pvalue_ )              \
    CYG_MACRO_START                             \
    register cyg_uint16 _tmp_;                  \
                                                \
    HAL_READ_UINT16(CYGARC_REG_CMCNT0, _tmp_);  \
                                                \
    *(_pvalue_) = _tmp_;                        \
    CYG_MACRO_END



#else
# error "No RTC handling defined"
#endif



#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
#define HAL_CLOCK_LATENCY( _pvalue_ ) HAL_CLOCK_READ(_pvalue_)
#endif


//----------------------------------------------------------------------------
// Reset.
#ifndef HAL_PLATFORM_RESET
#define HAL_PLATFORM_RESET()                                                                    \
 CYG_MACRO_START                                                                                \
 HAL_WRITE_UINT16(CYGARC_REG_RSTCSR_W, CYGARC_REG_RSTCSR_W_RSTx_MAGIC|CYGARC_REG_RSTCSR_RSTE);  \
 HAL_WRITE_UINT16(CYGARC_REG_WTCNT_W, CYGARC_REG_WTCNT_W_MAGIC|0xfe);                           \
 HAL_WRITE_UINT16(CYGARC_REG_WTCSR_W, CYGARC_REG_WTCSR_W_MAGIC|CYGARC_REG_WTCSR_TME|CYGARC_REG_WTCSR_WTIT);           \
 /* wait for it */                                                                              \
 for(;;) ;                                                                                      \
 CYG_MACRO_END
#else
#define HAL_PLATFORM_RESET()
#endif

#endif // __ASSEMBLER__

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_INTR_H
// End of var_intr.h
