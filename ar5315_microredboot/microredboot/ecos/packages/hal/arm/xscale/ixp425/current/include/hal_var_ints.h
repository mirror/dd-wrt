#ifndef CYGONCE_HAL_VAR_INTS_H
#define CYGONCE_HAL_VAR_INTS_H
//==========================================================================
//
//      hal_var_ints.h
//
//      HAL Interrupt and clock support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-12-03
// Purpose:      Define Interrupt support
// Description:  The interrupt details for XScale CPUs are defined here.
// Usage:
//		 #include <pkgconf/system.h>
//		 #include CYGBLD_HAL_VARIANT_H
//               #include CYGBLD_HAL_VAR_INTS_H
//
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_ixp425.h>         // registers

//
// Values for the vector argument of cyg_drv_interrupt_create() and
// other interrupt related interfaces.
//
#define CYGNUM_HAL_INTERRUPT_NONE         -1
#define CYGNUM_HAL_INTERRUPT_NPEA         0
#define CYGNUM_HAL_INTERRUPT_NPEB         1
#define CYGNUM_HAL_INTERRUPT_NPEC         2
#define CYGNUM_HAL_INTERRUPT_QM1          3
#define CYGNUM_HAL_INTERRUPT_QM2          4
#define CYGNUM_HAL_INTERRUPT_TIMER0       5
#define CYGNUM_HAL_INTERRUPT_GPIO0        6
#define CYGNUM_HAL_INTERRUPT_GPIO1        7
#define CYGNUM_HAL_INTERRUPT_PCI_INT      8
#define CYGNUM_HAL_INTERRUPT_PCI_DMA1     9
#define CYGNUM_HAL_INTERRUPT_PCI_DMA2     10
#define CYGNUM_HAL_INTERRUPT_TIMER1       11
#define CYGNUM_HAL_INTERRUPT_USB          12
#define CYGNUM_HAL_INTERRUPT_UART2        13
#define CYGNUM_HAL_INTERRUPT_TIMESTAMP    14
#define CYGNUM_HAL_INTERRUPT_UART1        15
#define CYGNUM_HAL_INTERRUPT_WDOG         16
#define CYGNUM_HAL_INTERRUPT_AHB_PMU      17
#define CYGNUM_HAL_INTERRUPT_XSCALE_PMU   18
#define CYGNUM_HAL_INTERRUPT_GPIO2        19
#define CYGNUM_HAL_INTERRUPT_GPIO3        20
#define CYGNUM_HAL_INTERRUPT_GPIO4        21
#define CYGNUM_HAL_INTERRUPT_GPIO5        22
#define CYGNUM_HAL_INTERRUPT_GPIO6        23
#define CYGNUM_HAL_INTERRUPT_GPIO7        24
#define CYGNUM_HAL_INTERRUPT_GPIO8        25
#define CYGNUM_HAL_INTERRUPT_GPIO9        26
#define CYGNUM_HAL_INTERRUPT_GPIO10       27
#define CYGNUM_HAL_INTERRUPT_GPIO11       28           
#define CYGNUM_HAL_INTERRUPT_GPIO12       29
#define CYGNUM_HAL_INTERRUPT_SW_INT1      30
#define CYGNUM_HAL_INTERRUPT_SW_INT2      31

#define CYGNUM_HAL_VAR_ISR_MAX    31

#define CYGNUM_HAL_ISR_MIN        0
#define CYGNUM_HAL_ISR_MAX        CYGNUM_HAL_VAR_ISR_MAX

#define CYGNUM_HAL_ISR_COUNT      (CYGNUM_HAL_ISR_MAX+1)

#define CYGNUM_HAL_INTERRUPT_RTC  CYGNUM_HAL_INTERRUPT_TIMER0

// ------------------------------------------------------------------------
// Dynamically set the timer interrupt rate.
// Not for application use at all.

externC void
hal_clock_reinitialize(          int *pfreq,    /* inout */
                        unsigned int *pperiod,  /* inout */
                        unsigned int old_hz );  /* in */

#define HAL_CLOCK_REINITIALIZE( _freq, _period, _old_hz ) \
        hal_clock_reinitialize( &_freq, &_period, _old_hz )

#define HAL_PLATFORM_RESET_ENTRY 0x00000000


// ------------------------------------------------------------------------
// Used for to tell if interrupt belongs to GDB comm channel
//
#define CYGHWR_HAL_GDB_PORT_VECTORS_MATCH(_v_, _gv_) \
    (((_v_) == (_gv_)) || \
     ((_v_) == CYGNUM_HAL_INTERRUPT_QM1 && \
       ((_gv_) == CYGNUM_HAL_INTERRUPT_NPEB || \
        (_gv_) == CYGNUM_HAL_INTERRUPT_NPEC)))


#endif // CYGONCE_HAL_VAR_INTS_H
// EOF hal_var_ints.h
