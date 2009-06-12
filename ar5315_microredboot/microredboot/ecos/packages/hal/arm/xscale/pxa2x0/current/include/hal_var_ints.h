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
// Author(s):    <knud.woehler@microplex.de>
// Date:         2002-09-02
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_pxa2x0.h>

// 1st level
// 0-7 Reserved
#define CYGNUM_HAL_INTERRUPT_GPIO0	8
#define CYGNUM_HAL_INTERRUPT_GPIO1	9
#define CYGNUM_HAL_INTERRUPT_GPIO	10
#define CYGNUM_HAL_INTERRUPT_USB	11
#define CYGNUM_HAL_INTERRUPT_PMU	12
#define CYGNUM_HAL_INTERRUPT_I2S	13
#define CYGNUM_HAL_INTERRUPT_AC97	14
// 15,16 Reserved
#define CYGNUM_HAL_INTERRUPT_LCD	17
#define CYGNUM_HAL_INTERRUPT_I2C	18
#define CYGNUM_HAL_INTERRUPT_ICP	19
#define CYGNUM_HAL_INTERRUPT_STUART	20
#define CYGNUM_HAL_INTERRUPT_BTUART	21
#define CYGNUM_HAL_INTERRUPT_FFUART	22
#define CYGNUM_HAL_INTERRUPT_MMC	23
#define CYGNUM_HAL_INTERRUPT_SSP	24
#define CYGNUM_HAL_INTERRUPT_DMA	25
#define CYGNUM_HAL_INTERRUPT_TIMER0	26
#define CYGNUM_HAL_INTERRUPT_TIMER1	27
#define CYGNUM_HAL_INTERRUPT_TIMER2	28
#define CYGNUM_HAL_INTERRUPT_TIMER3	29
#define CYGNUM_HAL_INTERRUPT_HZ		30
#define CYGNUM_HAL_INTERRUPT_ALARM	31


// 2nd level
#define CYGNUM_HAL_INTERRUPT_GPIO2	(32+2)
#define CYGNUM_HAL_INTERRUPT_GPIO3	(32+3)
#define CYGNUM_HAL_INTERRUPT_GPIO4	(32+4)
#define CYGNUM_HAL_INTERRUPT_GPIO5	(32+5)
#define CYGNUM_HAL_INTERRUPT_GPIO6	(32+6)
#define CYGNUM_HAL_INTERRUPT_GPIO7	(32+7)
#define CYGNUM_HAL_INTERRUPT_GPIO8	(32+8)
#define CYGNUM_HAL_INTERRUPT_GPIO9	(32+9)
#define CYGNUM_HAL_INTERRUPT_GPIO10	(32+10)
#define CYGNUM_HAL_INTERRUPT_GPIO11	(32+11)
#define CYGNUM_HAL_INTERRUPT_GPIO12	(32+12)
#define CYGNUM_HAL_INTERRUPT_GPIO13	(32+13)
#define CYGNUM_HAL_INTERRUPT_GPIO14	(32+14)
#define CYGNUM_HAL_INTERRUPT_GPIO15	(32+15)
#define CYGNUM_HAL_INTERRUPT_GPIO16	(32+16)
#define CYGNUM_HAL_INTERRUPT_GPIO17	(32+17)
#define CYGNUM_HAL_INTERRUPT_GPIO18	(32+18)
#define CYGNUM_HAL_INTERRUPT_GPIO19	(32+19)
#define CYGNUM_HAL_INTERRUPT_GPIO20	(32+20)
#define CYGNUM_HAL_INTERRUPT_GPIO21	(32+21)
#define CYGNUM_HAL_INTERRUPT_GPIO22	(32+22)
#define CYGNUM_HAL_INTERRUPT_GPIO23	(32+23)
#define CYGNUM_HAL_INTERRUPT_GPIO24	(32+24)
#define CYGNUM_HAL_INTERRUPT_GPIO25	(32+25)
#define CYGNUM_HAL_INTERRUPT_GPIO26	(32+26)
#define CYGNUM_HAL_INTERRUPT_GPIO27	(32+27)
#define CYGNUM_HAL_INTERRUPT_GPIO28	(32+28)
#define CYGNUM_HAL_INTERRUPT_GPIO29	(32+29)
#define CYGNUM_HAL_INTERRUPT_GPIO30	(32+30)
#define CYGNUM_HAL_INTERRUPT_GPIO31	(32+31)

#define CYGNUM_HAL_INTERRUPT_GPIO32	(64+0)
#define CYGNUM_HAL_INTERRUPT_GPIO33	(64+1)
#define CYGNUM_HAL_INTERRUPT_GPIO34	(64+2)
#define CYGNUM_HAL_INTERRUPT_GPIO35	(64+3)
#define CYGNUM_HAL_INTERRUPT_GPIO36	(64+4)
#define CYGNUM_HAL_INTERRUPT_GPIO37	(64+5)
#define CYGNUM_HAL_INTERRUPT_GPIO38	(64+6)
#define CYGNUM_HAL_INTERRUPT_GPIO39	(64+7)
#define CYGNUM_HAL_INTERRUPT_GPIO40	(64+8)
#define CYGNUM_HAL_INTERRUPT_GPIO41	(64+9)
#define CYGNUM_HAL_INTERRUPT_GPIO42	(64+10)
#define CYGNUM_HAL_INTERRUPT_GPIO43	(64+11)
#define CYGNUM_HAL_INTERRUPT_GPIO44	(64+12)
#define CYGNUM_HAL_INTERRUPT_GPIO45	(64+13)
#define CYGNUM_HAL_INTERRUPT_GPIO46	(64+14)
#define CYGNUM_HAL_INTERRUPT_GPIO47	(64+15)
#define CYGNUM_HAL_INTERRUPT_GPIO48	(64+16)
#define CYGNUM_HAL_INTERRUPT_GPIO49	(64+17)
#define CYGNUM_HAL_INTERRUPT_GPIO50	(64+18)
#define CYGNUM_HAL_INTERRUPT_GPIO51	(64+19)
#define CYGNUM_HAL_INTERRUPT_GPIO52	(64+20)
#define CYGNUM_HAL_INTERRUPT_GPIO53	(64+21)
#define CYGNUM_HAL_INTERRUPT_GPIO54	(64+22)
#define CYGNUM_HAL_INTERRUPT_GPIO55	(64+23)
#define CYGNUM_HAL_INTERRUPT_GPIO56	(64+24)
#define CYGNUM_HAL_INTERRUPT_GPIO57	(64+25)
#define CYGNUM_HAL_INTERRUPT_GPIO58	(64+26)
#define CYGNUM_HAL_INTERRUPT_GPIO59	(64+27)
#define CYGNUM_HAL_INTERRUPT_GPIO60	(64+28)
#define CYGNUM_HAL_INTERRUPT_GPIO61	(64+29)
#define CYGNUM_HAL_INTERRUPT_GPIO62	(64+30)
#define CYGNUM_HAL_INTERRUPT_GPIO63	(64+31)

#define CYGNUM_HAL_INTERRUPT_GPIO64	(96+0)
#define CYGNUM_HAL_INTERRUPT_GPIO65	(96+1)
#define CYGNUM_HAL_INTERRUPT_GPIO66	(96+2)
#define CYGNUM_HAL_INTERRUPT_GPIO67	(96+3)
#define CYGNUM_HAL_INTERRUPT_GPIO68	(96+4)
#define CYGNUM_HAL_INTERRUPT_GPIO69	(96+5)
#define CYGNUM_HAL_INTERRUPT_GPIO70	(96+6)
#define CYGNUM_HAL_INTERRUPT_GPIO71	(96+7)
#define CYGNUM_HAL_INTERRUPT_GPIO72	(96+8)
#define CYGNUM_HAL_INTERRUPT_GPIO73	(96+9)
#define CYGNUM_HAL_INTERRUPT_GPIO74	(96+10)
#define CYGNUM_HAL_INTERRUPT_GPIO75	(96+11)
#define CYGNUM_HAL_INTERRUPT_GPIO76	(96+12)
#define CYGNUM_HAL_INTERRUPT_GPIO77	(96+13)
#define CYGNUM_HAL_INTERRUPT_GPIO78	(96+14)
#define CYGNUM_HAL_INTERRUPT_GPIO79	(96+15)
#define CYGNUM_HAL_INTERRUPT_GPIO80	(96+16)
#define CYGNUM_HAL_INTERRUPT_GPIO81	(96+17)
#define CYGNUM_HAL_INTERRUPT_GPIO82	(96+18)
#define CYGNUM_HAL_INTERRUPT_GPIO83	(96+19)
#define CYGNUM_HAL_INTERRUPT_GPIO84	(96+20)
#define CYGNUM_HAL_INTERRUPT_GPIO85	(96+21)


#define CYGNUM_HAL_INTERRUPT_NONE	-1

#define CYGNUM_HAL_INTERRUPT_RTC	CYGNUM_HAL_INTERRUPT_TIMER0

#define CYGNUM_HAL_ISR_MIN		0
#define CYGNUM_HAL_ISR_MAX		(96+21)
#define CYGNUM_HAL_ISR_COUNT		(CYGNUM_HAL_ISR_MAX-CYGNUM_HAL_ISR_MIN+1)

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
externC void hal_clock_latency(cyg_uint32 *);
# define HAL_CLOCK_LATENCY( _pvalue_ ) hal_clock_latency( (cyg_uint32 *)(_pvalue_) )
#endif


// Reset
#define HAL_PLATFORM_RESET()			\
    CYG_MACRO_START						\
	cyg_uint32 ctrl;					\
	HAL_DISABLE_INTERRUPTS(ctrl);		\
	*PXA2X0_OWER = PXA2X0_OWER_WME;		\
	*PXA2X0_OSMR3 = *PXA2X0_OSCR + 1000; \
	for(;;);							\
    CYG_MACRO_END

#define HAL_PLATFORM_RESET_ENTRY 0x00000000

#endif // CYGONCE_HAL_VAR_INTS_H

