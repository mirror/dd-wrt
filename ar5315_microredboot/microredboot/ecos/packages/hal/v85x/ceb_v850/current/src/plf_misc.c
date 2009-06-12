//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Contributors: nickg, gthomas, jlarmour
// Date:         2000-03-10
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

#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/v850_common.h>        // Hardware definitions

#include <cyg/hal/hal_if.h>             // ROM monitor interfaces

#include <cyg/infra/diag.h>             // diag_printf

extern void show_hex4(unsigned long val);
extern void show_hex1(unsigned long val);
extern void show_8bit_reg(void *addr);
extern void show_led(int p);

void
cyg_hal_platform_hardware_init(void)
{
    hal_if_init();  // Initialize GDB[ROM]/eCos interfaces
    show_led(' ');
    show_led(' ');
}

void
show_hex4(unsigned long val)
{
    show_hex1(val>>24);
    show_hex1(val>>16);
    show_hex1(val>>8);
    show_hex1(val);
}

void
show_hex1(unsigned long val)
{
    static char *hex = "0123456789ABCDEF";
    show_led(hex[(val & 0xF0) >> 4]);
    show_led(hex[val & 0x0F]);
}

void
show_8bit_reg(void *addr)
{
    unsigned char *reg = (unsigned char *)addr;
    unsigned char val = *reg;
    show_led(' ');
    show_hex4((unsigned long)reg);
    show_led('=');
    show_hex1((unsigned long)val);
    show_led('/');
}

#define SEG_0 0x00
#define SEG_A 0x01
#define SEG_B 0x02
#define SEG_C 0x04
#define SEG_D 0x08
#define SEG_E 0x10
#define SEG_F 0x20
#define SEG_G 0x40
#define SEG_H 0x80
#define LED(a,b,c,d,e,f,g,h) SEG_##a|SEG_##b|SEG_##c|SEG_##d|SEG_##e|SEG_##f|SEG_##g|SEG_##h

static unsigned char led_bits[] = {
    LED(H,0,0,0,0,0,0,0), // 0x20
        0x30, // 0x21 !
        0x00, // 0x22 @
        0x00, // 0x23
        0x00, // 0x24
        0x00, // 0x25
        0x00, // 0x26
        0x00, // 0x27
        0x00, // 0x28
        0x00, // 0x29
        0x00, // 0x2A
        0x00, // 0x2B
        0x00, // 0x2C
        0x00, // 0x2D
        0x00, // 0x2E
        LED(E,G,B,0,0,0,0,0), // 0x2F
        LED(A,B,C,D,E,F,0,0), // 0x30
        LED(0,B,C,0,0,0,0,0), // 0x31
        LED(A,B,G,E,D,0,0,0), // 0x32
        LED(A,B,G,C,D,0,0,0), // 0x33
        LED(F,G,B,C,0,0,0,0), // 0x34
        LED(A,F,G,C,D,0,0,0), // 0x35
        LED(A,F,G,C,D,E,0,0), // 0x36
        LED(A,B,C,0,0,0,0,0), // 0x37
        LED(A,B,C,D,E,F,G,0), // 0x38
        LED(A,F,G,B,C,0,0,0), // 0x39
        LED(0,0,0,0,0,0,0,0), // 0x3A
        LED(0,0,0,0,0,0,0,0), // 0x3B
        LED(0,0,0,0,0,0,0,0), // 0x3C
        LED(D,G,0,0,0,0,0,0), // 0x3D - =
        LED(0,0,0,0,0,0,0,0), // 0x3E
        LED(0,0,0,0,0,0,0,0), // 0x3F
        LED(0,0,0,0,0,0,0,0), // 0x40
        LED(A,F,G,B,E,C,0,0), // 0x41 - A
        LED(F,G,E,D,C,0,0,0), // 0x42 - B
        LED(A,F,E,D,0,0,0,0), // 0x43 - C
        LED(B,G,E,D,C,0,0,0), // 0x44 - D
        LED(A,F,G,E,D,0,0,0), // 0x45 - E
        LED(A,F,G,E,0,0,0,0), // 0x46 - F
        LED(0,0,0,0,0,0,0,0), // 0x47
        LED(0,0,0,0,0,0,0,0), // 0x48
        LED(0,0,0,0,0,0,0,0), // 0x49
        LED(0,0,0,0,0,0,0,0), // 0x4A
        LED(0,0,0,0,0,0,0,0), // 0x4B
        LED(0,0,0,0,0,0,0,0), // 0x4C
        LED(0,0,0,0,0,0,0,0), // 0x4D
        LED(0,0,0,0,0,0,0,0), // 0x4E
        LED(0,0,0,0,0,0,0,0), // 0x4F
        LED(0,0,0,0,0,0,0,0), // 0x50
        LED(0,0,0,0,0,0,0,0), // 0x51
        LED(0,0,0,0,0,0,0,0), // 0x52
        LED(0,0,0,0,0,0,0,0), // 0x53
        LED(0,0,0,0,0,0,0,0), // 0x54
        LED(0,0,0,0,0,0,0,0), // 0x55
        LED(0,0,0,0,0,0,0,0), // 0x56
        LED(0,0,0,0,0,0,0,0), // 0x57
        LED(0,0,0,0,0,0,0,0), // 0x58
        LED(0,0,0,0,0,0,0,0), // 0x59
        LED(0,0,0,0,0,0,0,0), // 0x5A
        LED(0,0,0,0,0,0,0,0), // 0x5B
        LED(0,0,0,0,0,0,0,0), // 0x5C
        LED(0,0,0,0,0,0,0,0), // 0x5D
        LED(0,0,0,0,0,0,0,0), // 0x5E
        LED(0,0,0,0,0,0,0,0), // 0x5F
        LED(0,0,0,0,0,0,0,0), // 0x60
        LED(0,0,0,0,0,0,0,0), // 0x61
        LED(0,0,0,0,0,0,0,0), // 0x62
        LED(0,0,0,0,0,0,0,0), // 0x63
        LED(0,0,0,0,0,0,0,0), // 0x64
        LED(0,0,0,0,0,0,0,0), // 0x65
        LED(0,0,0,0,0,0,0,0), // 0x66
        LED(0,0,0,0,0,0,0,0), // 0x67
        LED(0,0,0,0,0,0,0,0), // 0x68
        LED(0,0,0,0,0,0,0,0), // 0x69
        LED(0,0,0,0,0,0,0,0), // 0x6A
        LED(0,0,0,0,0,0,0,0), // 0x6B
        LED(0,0,0,0,0,0,0,0), // 0x6C
        LED(0,0,0,0,0,0,0,0), // 0x6D
        LED(0,0,0,0,0,0,0,0), // 0x6E
        LED(0,0,0,0,0,0,0,0), // 0x6F
        LED(0,0,0,0,0,0,0,0), // 0x70
        LED(0,0,0,0,0,0,0,0), // 0x71
        LED(0,0,0,0,0,0,0,0), // 0x72
        LED(0,0,0,0,0,0,0,0), // 0x73
        LED(0,0,0,0,0,0,0,0), // 0x74
        LED(0,0,0,0,0,0,0,0), // 0x75
        LED(0,0,0,0,0,0,0,0), // 0x76
        LED(0,0,0,0,0,0,0,0), // 0x77
        LED(0,0,0,0,0,0,0,0), // 0x78
        LED(0,0,0,0,0,0,0,0), // 0x79
        LED(0,0,0,0,0,0,0,0), // 0x7A
        LED(0,0,0,0,0,0,0,0), // 0x7B
        LED(0,0,0,0,0,0,0,0), // 0x7C
        LED(0,0,0,0,0,0,0,0), // 0x7D
        LED(A,0,0,0,0,0,0,0), // 0x7E - ~
        LED(0,0,0,0,0,0,0,0), // 0x7F
};

#define DELAY 10000

void
show_led(int p)
{
    volatile unsigned char *led = (volatile unsigned char *)0xFFFFF014;
    int j;
    for (j = 0;  j < DELAY;  j++) {
        *led = led_bits[p-0x20];
    }
    for (j = 0;  j < DELAY*3;  j++) {
        *led = 0;
    }
}

void 
FAIL(char *m)
{
    static char *fail_reason;
    fail_reason = m;
#if 0
    while (1) {
        show_led('~');
        show_hex4(m);
        show_led('/');
    }
#else
    diag_printf("FAIL: %s\n", m);
#endif
}

void 
ALERT(char *m)
{
    static char *alert_reason;
    alert_reason = m;
#if 0
    show_led('~');
    show_led('~');
    show_hex4(m);
    show_led('/');
#else
    diag_printf("ALERT: %s\n", m);
#endif
}

//
// Clock support, using 16 bit timer TM1
//

CYG_WORD32 cyg_hal_clock_period;

void 
hal_clock_initialize(cyg_int32 period)
{
    volatile unsigned char *tmc = (volatile unsigned char *)V850_REG_TMC1;
    volatile unsigned char *crc = (volatile unsigned char *)V850_REG_CRC1;
    volatile unsigned char *prm = (volatile unsigned char *)V850_REG_PRM10;
    volatile unsigned short *cmp0 = (volatile unsigned short *)V850_REG_CR10;
    volatile unsigned short *cmp1 = (volatile unsigned short *)V850_REG_CR11;
    *tmc = 0x00;  // Halt timer
    *crc = 0x00;  // Select simple compare mode
    *prm = 0x01;  // System clock / 4
    *cmp0 = period;
    *cmp1 = period;
    *tmc = 0x0C;  // Continuous, reset mode (interval timer)
    cyg_hal_clock_period = period;
}

void 
hal_clock_reset(int vector, cyg_uint32 period)
{
    hal_clock_initialize(period);
}

//
// Return the number of clock "ticks" since last interrupt
//
cyg_uint32 hal_clock_read(void)
{
    volatile unsigned short *timer = (volatile unsigned short *)V850_REG_TM1;
    return (cyg_uint32)*timer;
}


extern void _hal_thread_load_context(HAL_SavedRegisters **to)
     CYGBLD_ATTRIB_NORET;
extern void _hal_thread_switch_context(HAL_SavedRegisters **to, HAL_SavedRegisters **from);

void hal_thread_load_context(CYG_ADDRESS to)
{
    HAL_SavedRegisters **new_context = (HAL_SavedRegisters **)to;
#if 0
    diag_printf("Load context: %x\n", *new_context);
    show_regs(*new_context);
#endif
    _hal_thread_load_context(new_context);
}

void hal_thread_switch_context(CYG_ADDRESS to, CYG_ADDRESS from)
{
    HAL_SavedRegisters **old_context = (HAL_SavedRegisters **)from;
    HAL_SavedRegisters **new_context = (HAL_SavedRegisters **)to;
#if 0
    diag_printf("Switch context - old: %x, new: %x\n", *old_context, *new_context);
    show_regs(*new_context);
#endif
    _hal_thread_switch_context(new_context, old_context);
}

