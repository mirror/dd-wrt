#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
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
// Author(s):    gthomas
// Contributors: gthomas, jlarmour
// Date:         2000-03-10
// Purpose:      NEC CEB/V850 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>           // System-wide configuration info
#include <pkgconf/hal.h>              // Architecture independent configuration
#include <cyg/hal/v850_common.h>        
#include CYGBLD_HAL_PLATFORM_H        // Platform specific configuration

        .macro  lea     addr,reg
        movhi   hi(\addr),r0,\reg
        movea   lo(\addr),\reg,\reg
        .endm
        
        .macro  PLATFORM_SETUP1
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
        movhi   hi(V850_REGS),r0,r6       

        // set bus control signals
        mov     0x01,r1                        
        st.b    r1,lo(V850_REG_SYC)[r6]

        // Internal RAM, internal ROM and I/O - no wait states, regardless
        // of the setting of DWC
        // External RAM is 70ns, External ROM is 120ns. Therefore...
#if CYGHWR_HAL_V85X_CPU_FREQ < 14285714
        // External RAM      - 0 wait states
        movea   0x3F00,r0,r1
#elif CYGHWR_HAL_V85X_CPU_FREQ < 28571428
        // External RAM      - 1 wait state
        movea   0x7F00,r0,r1
#elif CYGHWR_HAL_V85X_CPU_FREQ < 42857142
        // External RAM      - 2 wait states
        movea   0xBF00,r0,r1
#else
        // External RAM      - 3 wait states
        movea   0xFF00,r0,r1
#endif
#if CYGHWR_HAL_V85X_CPU_FREQ < 8333333
        // External ROM      - 0 wait states
        ori     0x00FC,r1,r1
#elif CYGHWR_HAL_V85X_CPU_FREQ < 16666667
        // External ROM      - 1 wait states
        ori     0x00FD,r1,r1
#elif CYGHWR_HAL_V85X_CPU_FREQ < 25000000
        // External ROM      - 2 wait states
        ori     0x00FE,r1,r1
#else
        // External ROM      - 3 wait states
        ori     0x00FF,r1,r1
#endif
        st.h    r1,lo(V850_REG_DWC)[r6]

        // Internal RAM, ROM, I/O - always 0 idle regardless of the setting
        // of BCC
        // External RAM      - 0 idle
        // External ROM      - 0 idle
        movea   0x2AA8,r0,r1
        st.h    r1,lo(V850_REG_BCC)[r6]

        // No INTs on rising edge
        mov     0x00,r1                        
        st.b    r1,lo(V850_REG_EGP0)[r6]

        // enable INTP0 (serial) IRQ only, set for falling edge
        mov     0x01,r1                        
        st.b    r1,lo(V850_REG_EGN0)[r6]

        // Port 1 mode: set serial DSR, RXD and CTS as inputs, and others
        // as outputs
        movea   0x2C,r0,r1                        
        st.b    r1,lo(V850_REG_PM1)[r6]

        // Port 3 mode: SW2 read port: set to all input
        movea   0xFF,r0,r1                        
        st.b    r1,lo(V850_REG_PM3)[r6]

        // Set serial port control inputs (DSR, RXD, CTS) to 1
        // FIXME Why? Also I don't get why the top two bits are set.
        movea   0xEC,r0,r1                        
        st.b    r1,lo(V850_REG_P1)[r6]

        // Enable all outputs for 7-segment LED
        mov     0x00,r1                        
        st.b    r1,lo(V850_REG_PM10)[r6]

        // Set LED to 0
        mov     0x00,r1                        
        st.b    r1,lo(V850_REG_P10)[r6]

        // Init serial port 0 baud rate to divide clock down to 9600 baud
        // by setting baud count here
        // This may seem unnecessary, but setting up the serial allows
        // us to do diag output before HAL diag is initialized. The values
        // are clock dependent however, but this is only for debug so we
        // don't care.
        movea   0xDD,r0,r1                        
        st.b    r1,lo(V850_REG_BRGC0)[r6]

        // and divisor here
        mov     0x03,r1                        
        st.b    r1,lo(V850_REG_BRGMC00)[r6]

        // set serial 0 to enable tx/rx and 8-N-1
        movea   0xC8,r0,r1                        
        st.b    r1,lo(V850_REG_ASIM0)[r6]

        // disable reception of serial interrupts, and set serial interrupt
        // priority to level 7 (lowest)
        movea   0x47,r0,r1                        
        st.b    r1,lo(V850_REG_STIC0)[r6]

        // Memory expansion mode - set to 4MB
        // We could probably set this to 256K (MM==0x4), but there seems
        // no advantage
        mov     0x07,r1                        
        st.b    r1,lo(V850_REG_MM)[r6]

        // Setting the PCC register is tricky - it is a "specific register"
        // We set the CPU clock to full speed
        stsr    PSW,r7
        ori     CYGARC_PSW_NP,r7,r8
        ldsr    r8,PSW
        mov     0x00,r1
        st.b    r1,lo(V850_REG_PRCMD)[r6]
        st.b    r1,lo(V850_REG_PCC)[r6]
        ldsr    r7,PSW
        nop
        nop
        nop
        nop
        nop
#endif
        .endm

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
