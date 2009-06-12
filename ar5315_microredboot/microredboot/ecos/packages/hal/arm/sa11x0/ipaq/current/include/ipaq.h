#ifndef CYGONCE_IPAQ_H
#define CYGONCE_IPAQ_H

//=============================================================================
//
//      ipaq.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    gthomas
// Contributors: gthomas, richard.panton@3glab.com
// Date:         2001-02-24
// Purpose:      Intel SA1110/iPAQ platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/ipaq.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================


#ifndef __ASSEMBLER__
//
// Extended GPIO
// Note: This register is write-only.  Thus a shadow copy is provided so that
// it may be safely updated/shared by multiple threads.
//
extern unsigned long _ipaq_EGPIO;  // Shadow copy

extern unsigned short _ipaq_LCD_params[(4*2)+1];  // Various LCD parameters

extern void ipaq_EGPIO(unsigned long mask, unsigned long value);

#endif

// 
// Signal assertion levels
//
#define _LOGIC_ONE(m)  (m & 0xFFFF)
#define _LOGIC_ZERO(m) (m & 0x0000)

//
// iPAQ Extended GPIO definitions
//
#define SA1110_EGPIO			REG16_PTR(0x49000000)
#define SA1110_EIO_MIN			0x0080

#define SA1110_EIO_VPP			0x0001
# define SA1110_EIO_VPP_OFF			_LOGIC_ZERO(SA1110_EIO_VPP)
# define SA1110_EIO_VPP_ON			_LOGIC_ONE(SA1110_EIO_VPP)
#define SA1110_EIO_CF_RESET		0x0002
# define SA1110_EIO_CF_RESET_DISABLE		_LOGIC_ZERO(SA1110_EIO_CF_RESET)
# define SA1110_EIO_CF_RESET_ENABLE		_LOGIC_ONE(SA1110_EIO_CF_RESET)
#define SA1110_EIO_OPT_RESET		0x0004
# define SA1110_EIO_OPT_RESET_DISABLE		_LOGIC_ZERO(SA1110_EIO_OPT_RESET)
# define SA1110_EIO_OPT_RESET_ENABLE		_LOGIC_ONE(SA1110_EIO_OPT_RESET)
#define SA1110_EIO_CODEC_RESET		0x0008	// Active LOW
# define SA1110_EIO_CODEC_RESET_DISABLE		_LOGIC_ONE(SA1110_EIO_CODEC_RESET)
# define SA1110_EIO_CODEC_RESET_ENABLE		_LOGIC_ZERO(SA1110_EIO_CODEC_RESET)
#define SA1110_EIO_OPT_PWR		0x0010
# define SA1110_EIO_OPT_PWR_OFF			_LOGIC_ZERO(SA1110_EIO_OPT_PWR)
# define SA1110_EIO_OPT_PWR_ON			_LOGIC_ONE(SA1110_EIO_OPT_PWR)
#define SA1110_EIO_OPT			0x0020
# define SA1110_EIO_OPT_OFF			_LOGIC_ZERO(SA1110_EIO_OPT)
# define SA1110_EIO_OPT_ON			_LOGIC_ONE(SA1110_EIO_OPT)
#define SA1110_EIO_LCD_3V3		0x0040
# define SA1110_EIO_LCD_3V3_OFF			_LOGIC_ZERO(SA1110_EIO_LCD_3V3)
# define SA1110_EIO_LCD_3V3_ON			_LOGIC_ONE(SA1110_EIO_LCD_3V3)
#define SA1110_EIO_RS232		0x0080
# define SA1110_EIO_RS232_OFF			_LOGIC_ZERO(SA1110_EIO_RS232)
# define SA1110_EIO_RS232_ON			_LOGIC_ONE(SA1110_EIO_RS232)
#define SA1110_EIO_LCD_CTRL		0x0100
# define SA1110_EIO_LCD_CTRL_OFF		_LOGIC_ZERO(SA1110_EIO_LCD_CTRL)
# define SA1110_EIO_LCD_CTRL_ON			_LOGIC_ONE(SA1110_EIO_LCD_CTRL)
#define SA1110_EIO_IR			0x0200
# define SA1110_EIO_IR_OFF			_LOGIC_ZERO(SA1110_EIO_IR)
# define SA1110_EIO_IR_ON			_LOGIC_ONE(SA1110_EIO_IR)
#define SA1110_EIO_AMP			0x0400
# define SA1110_EIO_AMP_OFF			_LOGIC_ZERO(SA1110_EIO_AMP)
# define SA1110_EIO_AMP_ON			_LOGIC_ONE(SA1110_EIO_AMP)
#define SA1110_EIO_AUDIO		0x0800
# define SA1110_EIO_AUDIO_OFF			_LOGIC_ZERO(SA1110_EIO_AUDIO)
# define SA1110_EIO_AUDIO_ON			_LOGIC_ONE(SA1110_EIO_AUDIO)
#define SA1110_EIO_MUTE			0x1000
# define SA1110_EIO_MUTE_OFF			_LOGIC_ZERO(SA1110_EIO_MUTE)
# define SA1110_EIO_MUTE_ON			_LOGIC_ONE(SA1110_EIO_MUTE)
#define SA1110_EIO_IR_FSEL		0x2000
# define SA1110_EIO_SIR				_LOGIC_ZERO(SA1110_EIO_IR_FSEL)
# define SA1110_EIO_FIR				_LOGIC_ONE(SA1110_EIO_IR_FSEL)
#define SA1110_EIO_LCD_5V		0x4000
# define SA1110_EIO_LCD_5V_OFF			_LOGIC_ZERO(SA1110_EIO_LCD_5V)
# define SA1110_EIO_LCD_5V_ON			_LOGIC_ONE(SA1110_EIO_LCD_5V)
#define SA1110_EIO_LCD_VDD		0x8000
# define SA1110_EIO_LCD_VDD_OFF			_LOGIC_ZERO(SA1110_EIO_LCD_VDD)
# define SA1110_EIO_LCD_VDD_ON			_LOGIC_ONE(SA1110_EIO_LCD_VDD)

//
// Special purpose GPIO interrupt mappings
//
#define SA1110_CF_IRQ               CYGNUM_HAL_INTERRUPT_GPIO21
#define SA1110_CF_DETECT            CYGNUM_HAL_INTERRUPT_GPIO17

//
// GPIO layout
//
#define SA1110_GPIO_CF_DETECT       0x00020000     // 0 = Compact Flash detect
#define SA1110_GPIO_CF_PRESENT      _LOGIC_ZERO(SA1110_GPIO_CF_DETECT)
#define SA1110_GPIO_CF_ABSENT       _LOGIC_ONE(SA1110_GPIO_CF_DETECT)
#define SA1110_GPIO_CF2_DETECT      0x00000200     // 0 = Second slot on 2-slot sleeve
#define SA1110_GPIO_CF2_PRESENT     _LOGIC_ZERO(SA1110_GPIO_CF2_DETECT)
#define SA1110_GPIO_CF2_ABSENT      _LOGIC_ONE(SA1110_GPIO_CF2_DETECT)

//
// PCMCIA controller in external sleeve
//
#define IPAQ_CF_CTRL                0x1A000000    // Slot 0 (same as single slot)
#define IPAQ_CF2_CTRL               0x19000000    // Slot 1
#define IPAQ_CF_CTRL_V5               0x0001   // 5v power enable
#define IPAQ_CF_CTRL_V5_ENABLE        _LOGIC_ZERO(IPAQ_CF_CTRL_V5)
#define IPAQ_CF_CTRL_V5_DISABLE       _LOGIC_ONE(IPAQ_CF_CTRL_V5)
#define IPAQ_CF_CTRL_V3               0x0002   // 3v power enable
#define IPAQ_CF_CTRL_V3_ENABLE        _LOGIC_ZERO(IPAQ_CF_CTRL_V3)
#define IPAQ_CF_CTRL_V3_DISABLE       _LOGIC_ONE(IPAQ_CF_CTRL_V3)
#define IPAQ_CF_CTRL_VCC              0x0004   // Vcc power enable
#define IPAQ_CF_CTRL_V12              0x0008   // 12v power enable
#define IPAQ_CF_CTRL_RESET            0x0010   // Reset card
#define IPAQ_CF_CTRL_RESET_ENABLE     _LOGIC_ONE(IPAQ_CF_CTRL_RESET)
#define IPAQ_CF_CTRL_RESET_DISABLE    _LOGIC_ZERO(IPAQ_CF_CTRL_RESET)
#define IPAQ_CF_CTRL_APOE             0x0020   // Auto power off
#define IPAQ_CF_CTRL_APOE_ENABLE      _LOGIC_ONE(IPAQ_CF_CTRL_APOE)
#define IPAQ_CF_CTRL_APOE_DISABLE     _LOGIC_ZERO(IPAQ_CF_CTRL_APOE)
#define IPAQ_CF_CTRL_CFE              0x0040   // Compact Flash [addressing] enable
#define IPAQ_CF_CTRL_SOE              0x0080   // Enable signal outputs
#define IPAQ_CF_CTRL_SOE_ENABLE       _LOGIC_ONE(IPAQ_CF_CTRL_SOE)
#define IPAQ_CF_CTRL_SOE_DISABLE      _LOGIC_ZERO(IPAQ_CF_CTRL_SOE)
#define IPAQ_CF_CTRL_SSP              0x0100   // Slot polarity; slot0=0, slot1=1


//
// LCD Controller
//
#define SA1110_LCCR0                SA11X0_REGISTER(0x30100000)
#define SA1110_LCSR                 SA11X0_REGISTER(0x30100004)
#define SA1110_DBAR1                SA11X0_REGISTER(0x30100010)
#define SA1110_DCAR1                SA11X0_REGISTER(0x30100014)
#define SA1110_DBAR2                SA11X0_REGISTER(0x30100018)
#define SA1110_DCAR2                SA11X0_REGISTER(0x3010001C)
#define SA1110_LCCR1                SA11X0_REGISTER(0x30100020)
#define SA1110_LCCR2                SA11X0_REGISTER(0x30100024)
#define SA1110_LCCR3                SA11X0_REGISTER(0x30100028)

/* end of ipaq.h                                                          */
#endif /* CYGONCE_IPAQ_H */
