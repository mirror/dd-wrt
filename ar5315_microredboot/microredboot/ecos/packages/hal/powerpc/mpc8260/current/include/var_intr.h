#ifndef CYGONCE_VAR_INTR_H
#define CYGONCE_VAR_INTR_H
//=============================================================================
//
//      var_intr.h
//
//      Variant HAL interrupt and clock support
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):   mtek, pfine
// Contributors:nickg, jskov, jlarmour, hmt
// Date:        2001-12-12
// Purpose:     Variant interrupt support
// Description: The macros defined here provide the HAL APIs for handling
//              interrupts and the clock on the MPC8260 PowerQUICCII CPU.
// Usage:       Is included via the architecture interrupt header:
//              #include <cyg/hal/hal_intr.h>
//              ...
//
//####DESCRIPTIONEND####
//
//=============================================================================
#include <pkgconf/hal.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/hal/mpc8260.h>            // Memory map
#include <cyg/infra/cyg_type.h>         // types
#include <cyg/hal/hal_io.h>             // io macros
#include <cyg/infra/cyg_ass.h>          // CYG_FAIL

#define CYGARC_IMM_BASE      0x04700000

// Interrupts

// The first level of external interrupts
#define CYGNUM_HAL_INTERRUPT_I2C                 1
#define CYGNUM_HAL_INTERRUPT_SPI                 2
#define CYGNUM_HAL_INTERRUPT_RISC_TIMERS         3
#define CYGNUM_HAL_INTERRUPT_SMC1                4
#define CYGNUM_HAL_INTERRUPT_SMC2                5
#define CYGNUM_HAL_INTERRUPT_IDMA1               6
#define CYGNUM_HAL_INTERRUPT_IDMA2               7
#define CYGNUM_HAL_INTERRUPT_IDMA3               8
#define CYGNUM_HAL_INTERRUPT_IDMA4               9
#define CYGNUM_HAL_INTERRUPT_SDMA               10

#define CYGNUM_HAL_INTERRUPT_TIMER1             12
#define CYGNUM_HAL_INTERRUPT_TIMER2             13
#define CYGNUM_HAL_INTERRUPT_TIMER3             14
#define CYGNUM_HAL_INTERRUPT_TIMER4             15
#define CYGNUM_HAL_INTERRUPT_TMCNT              16
#define CYGNUM_HAL_INTERRUPT_PIT                17

#define CYGNUM_HAL_INTERRUPT_IRQ1               19
#define CYGNUM_HAL_INTERRUPT_IRQ2               20
#define CYGNUM_HAL_INTERRUPT_IRQ3               21
#define CYGNUM_HAL_INTERRUPT_IRQ4               22
#define CYGNUM_HAL_INTERRUPT_IRQ5               23
#define CYGNUM_HAL_INTERRUPT_IRQ6               24
#define CYGNUM_HAL_INTERRUPT_IRQ7               25

#define CYGNUM_HAL_INTERRUPT_FCC1               32
#define CYGNUM_HAL_INTERRUPT_FCC2               33
#define CYGNUM_HAL_INTERRUPT_FCC3               34

#define CYGNUM_HAL_INTERRUPT_MCC1               36
#define CYGNUM_HAL_INTERRUPT_MCC2               37

#define CYGNUM_HAL_INTERRUPT_SCC1               40
#define CYGNUM_HAL_INTERRUPT_SCC2               41
#define CYGNUM_HAL_INTERRUPT_SCC3               42
#define CYGNUM_HAL_INTERRUPT_SCC4               43

#define CYGNUM_HAL_INTERRUPT_PC15               48
#define CYGNUM_HAL_INTERRUPT_PC14               49
#define CYGNUM_HAL_INTERRUPT_PC13               50
#define CYGNUM_HAL_INTERRUPT_PC12               51
#define CYGNUM_HAL_INTERRUPT_PC11               52
#define CYGNUM_HAL_INTERRUPT_PC10               53
#define CYGNUM_HAL_INTERRUPT_PC9                54
#define CYGNUM_HAL_INTERRUPT_PC8                55
#define CYGNUM_HAL_INTERRUPT_PC7                56
#define CYGNUM_HAL_INTERRUPT_PC6                57
#define CYGNUM_HAL_INTERRUPT_PC5                58
#define CYGNUM_HAL_INTERRUPT_PC4                59
#define CYGNUM_HAL_INTERRUPT_PC3                60
#define CYGNUM_HAL_INTERRUPT_PC2                61
#define CYGNUM_HAL_INTERRUPT_PC1                62
#define CYGNUM_HAL_INTERRUPT_PC0                63

#define CYGNUM_HAL_INTERRUPT_ERROR          0

#define CYGNUM_HAL_ISR_MAX                   CYGNUM_HAL_INTERRUPT_PC0

//--------------------------------------------------------------------------
// Interrupt controller access

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED
#ifdef CYGPKG_HAL_POWERPC_MPC8260



static __inline__ void
cyg_hal_interrupt_mask ( cyg_uint32 vector )
{
  volatile t_PQ2IMM    *IMM = (volatile t_PQ2IMM *) CYGARC_IMM_BASE;
  cyg_uint32 *reg_simr_h = (cyg_uint32 *) &(IMM->ic_simr_h);
  cyg_uint32 *reg_simr_l = (cyg_uint32 *) &(IMM->ic_simr_l);

  switch (vector) {
    
  case CYGNUM_HAL_INTERRUPT_PC15 ... CYGNUM_HAL_INTERRUPT_PC0:
    *reg_simr_h &= ~( (0x00010000) << (vector - CYGNUM_HAL_INTERRUPT_PC15) );
    break;

  case CYGNUM_HAL_INTERRUPT_IRQ1 ... CYGNUM_HAL_INTERRUPT_IRQ7:
    *reg_simr_h &= ~( (0x00004000) >> (vector - CYGNUM_HAL_INTERRUPT_IRQ1) );
    break;

  case CYGNUM_HAL_INTERRUPT_TMCNT:
    *reg_simr_h &= ~(0x00000004);
    break;

  case CYGNUM_HAL_INTERRUPT_PIT:
    *reg_simr_h &= ~(0x00000002);
    break;

  case CYGNUM_HAL_INTERRUPT_FCC1 ... CYGNUM_HAL_INTERRUPT_FCC3:
    *reg_simr_l &= ~( (0x20000000) << (CYGNUM_HAL_INTERRUPT_FCC3 - vector) );
    break;

  case CYGNUM_HAL_INTERRUPT_MCC1 ... CYGNUM_HAL_INTERRUPT_MCC2:
    *reg_simr_l &= ~( (0x08000000) >> (vector - CYGNUM_HAL_INTERRUPT_MCC1) );
    break;

  case CYGNUM_HAL_INTERRUPT_SCC1 ... CYGNUM_HAL_INTERRUPT_SCC4:
    *reg_simr_l &= ~( (0x00800000) >> (vector - CYGNUM_HAL_INTERRUPT_SCC1) );
    break;
    
  case  CYGNUM_HAL_INTERRUPT_I2C ... CYGNUM_HAL_INTERRUPT_SDMA:
    *reg_simr_l &= ~( (0x00008000) >> (vector - CYGNUM_HAL_INTERRUPT_I2C) );
    break;

  case CYGNUM_HAL_INTERRUPT_TIMER1 ... CYGNUM_HAL_INTERRUPT_TIMER4:
    *reg_simr_l &= ~( (0x00000010) >> (vector - CYGNUM_HAL_INTERRUPT_TIMER1) );
    break;

  default:
    CYG_FAIL("Unknown Interrupt in mask !!!");
    break;
  }

}

static __inline__ void
cyg_hal_interrupt_unmask ( cyg_uint32 vector )
{

  volatile t_PQ2IMM    *IMM = (volatile t_PQ2IMM *) CYGARC_IMM_BASE;
  cyg_uint32 *reg_simr_h = (cyg_uint32 *) &(IMM->ic_simr_h);
  cyg_uint32 *reg_simr_l = (cyg_uint32 *) &(IMM->ic_simr_l);

  switch (vector) {
    
  case CYGNUM_HAL_INTERRUPT_PC15 ... CYGNUM_HAL_INTERRUPT_PC0:
    *reg_simr_h |= ( (0x00010000) << (vector - CYGNUM_HAL_INTERRUPT_PC15) );
    break;

  case CYGNUM_HAL_INTERRUPT_IRQ1 ... CYGNUM_HAL_INTERRUPT_IRQ7:
    *reg_simr_h |= ( (0x00004000) >> (vector - CYGNUM_HAL_INTERRUPT_IRQ1) );
    break;

  case CYGNUM_HAL_INTERRUPT_TMCNT:
    *reg_simr_h |= (0x00000004);
    break;

  case CYGNUM_HAL_INTERRUPT_PIT:
    *reg_simr_h |= (0x00000002);
    break;

  case CYGNUM_HAL_INTERRUPT_FCC1 ... CYGNUM_HAL_INTERRUPT_FCC3:
    *reg_simr_l |= ( (0x20000000) << (CYGNUM_HAL_INTERRUPT_FCC3 - vector) );
    break;

  case CYGNUM_HAL_INTERRUPT_MCC1 ... CYGNUM_HAL_INTERRUPT_MCC2:
    *reg_simr_l |= ( (0x08000000) >> (vector - CYGNUM_HAL_INTERRUPT_MCC1) );
    break;

  case CYGNUM_HAL_INTERRUPT_SCC1 ... CYGNUM_HAL_INTERRUPT_SCC4:
    *reg_simr_l |= ( (0x00800000) >> (vector - CYGNUM_HAL_INTERRUPT_SCC1) );
    break;
    
  case  CYGNUM_HAL_INTERRUPT_I2C ... CYGNUM_HAL_INTERRUPT_SDMA:
    *reg_simr_l |= ( (0x00008000) >> (vector - CYGNUM_HAL_INTERRUPT_I2C) );
    break;

  case CYGNUM_HAL_INTERRUPT_TIMER1 ... CYGNUM_HAL_INTERRUPT_TIMER4:
    *reg_simr_l |= ( (0x00000010) >> (vector - CYGNUM_HAL_INTERRUPT_TIMER1) );
    break;

  default:
    CYG_FAIL("Unknown Interrupt in unmask !!!");
    break;
  }

}

static __inline__ void
cyg_hal_interrupt_acknowledge ( cyg_uint32 vector )
{

  volatile t_PQ2IMM    *IMM = (volatile t_PQ2IMM *) CYGARC_IMM_BASE;
  cyg_uint32 *reg_sipnr_h = (cyg_uint32 *) &(IMM->ic_sipnr_h);
  cyg_uint32 *reg_sipnr_l = (cyg_uint32 *) &(IMM->ic_sipnr_l);

  switch (vector) {
    
  case CYGNUM_HAL_INTERRUPT_PC15 ... CYGNUM_HAL_INTERRUPT_PC0:
    *reg_sipnr_h |= ( (0x00010000) << (vector - CYGNUM_HAL_INTERRUPT_PC15) );
    break;

  case CYGNUM_HAL_INTERRUPT_IRQ1 ... CYGNUM_HAL_INTERRUPT_IRQ7:
    *reg_sipnr_h |= ( (0x00004000) >> (vector - CYGNUM_HAL_INTERRUPT_IRQ1) );
    break;

  case CYGNUM_HAL_INTERRUPT_TMCNT:
    *reg_sipnr_h |= (0x00000004);
    break;

  case CYGNUM_HAL_INTERRUPT_PIT:
    *reg_sipnr_h |= (0x00000002);
    break;

  case CYGNUM_HAL_INTERRUPT_FCC1 ... CYGNUM_HAL_INTERRUPT_FCC3:
    *reg_sipnr_l |= ( (0x20000000) << (CYGNUM_HAL_INTERRUPT_FCC3 - vector) );
    break;

  case CYGNUM_HAL_INTERRUPT_MCC1 ... CYGNUM_HAL_INTERRUPT_MCC2:
    *reg_sipnr_l |= ( (0x08000000) >> (vector - CYGNUM_HAL_INTERRUPT_MCC1) );
    break;

  case CYGNUM_HAL_INTERRUPT_SCC1 ... CYGNUM_HAL_INTERRUPT_SCC4:
    *reg_sipnr_l |= ( (0x00800000) >> (vector - CYGNUM_HAL_INTERRUPT_SCC1) );
    break;
    
  case  CYGNUM_HAL_INTERRUPT_I2C ... CYGNUM_HAL_INTERRUPT_SDMA:
    *reg_sipnr_l |= ( (0x00008000) >> (vector - CYGNUM_HAL_INTERRUPT_I2C) );
    break;

  case CYGNUM_HAL_INTERRUPT_TIMER1 ... CYGNUM_HAL_INTERRUPT_TIMER4:
    *reg_sipnr_l |= ( (0x00000010) >> (vector - CYGNUM_HAL_INTERRUPT_TIMER1) );
    break;

  default:
    CYG_FAIL("Unknown Interrupt in unmask !!!");
    break;
  }

}

static __inline__ void
cyg_hal_interrupt_configure ( cyg_uint32 vector,
                              cyg_bool level,
                              cyg_bool up )
{
  // NOT IMPLEMENTED ...
}


static __inline__ void
cyg_hal_interrupt_set_level ( cyg_uint32 vector, cyg_uint32 level )
{

  // NOT IMPLEMENTED  ....
  // FACT : USER should not program the same interrupt to more than
  // one priority position. 
  // FACT : Every interrupt has an assigned default priority.

  // PROBLEM : One has to find the previous priority of the given vector
  // and swap(?) it with the requested priority owner (Not nice because
  // it changes another interrupt's priority inadvertently)

}

// The decrementer interrupt cannnot be masked, configured or acknowledged.

#define HAL_INTERRUPT_MASK( _vector_ )                    \
    CYG_MACRO_START                                       \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))   \
        cyg_hal_interrupt_mask ( (_vector_) );            \
    CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                  \
    CYG_MACRO_START                                       \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))   \
        cyg_hal_interrupt_unmask ( (_vector_) );          \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )             \
    CYG_MACRO_START                                       \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))   \
        cyg_hal_interrupt_acknowledge ( (_vector_) );     \
    CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )              \
    CYG_MACRO_START                                                     \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))                 \
        cyg_hal_interrupt_configure ( (_vector_), (_level_), (_up_) );  \
    CYG_MACRO_END

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )            \
    CYG_MACRO_START                                             \
    if (CYGNUM_HAL_INTERRUPT_DECREMENTER != (_vector_))         \
        cyg_hal_interrupt_set_level ( (_vector_) , (_level_) ); \
    CYG_MACRO_END

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#endif
#endif



//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_INTR_H
// End of var_intr.h
