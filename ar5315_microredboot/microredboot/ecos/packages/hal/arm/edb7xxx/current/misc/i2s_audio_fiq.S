// #========================================================================
// #
// #    i2s_audio_FIQ.S
// #
// #    FIQ handler for EB7209 audio stream
// #
// #========================================================================
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
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     gthomas
// # Contributors:  gthomas
// # Date:          1999-09-10
// # Purpose:       Support EB7209 audio interrupts
// # Description:   This routine will send data to the audio FIFOs.
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================
	
#include <cyg/hal/hal_edb7xxx.h>

//        .file   "i2s_audio_FIQ.S"

        .extern cur_buf
        .extern next_buf
// These defines must mimic the data structure used by the main program        
#define audio_buf_left_ptr   0x0
#define audio_buf_right_ptr  0x4
#define audio_buf_length     0x8                      

        .code   32
        .text

// Note: when handling an FIQ, registers r8-r14 are free to use
// since they are unique (banked).
                
        .globl  i2s_FIQ
i2s_FIQ:

        ldr     r11,=FIQ_count
        ldr     r12,[r11]
        add     r12,r12,#1
        str     r12,[r11]
        
        ldr     r11,=I2S_LEFT_FIFO
        ldr     r12,=I2S_RIGHT_FIFO

// Empty the record FIFOs
// Note: without this, the audio hardware gets stuck        
        ldr     r13,[r11]
        ldr     r13,[r12]        
        ldr     r13,[r11]
        ldr     r13,[r12]        
        ldr     r13,[r11]
        ldr     r13,[r12]        
        ldr     r13,[r11]
        ldr     r13,[r12]        
        
// See if there is a buffer to play        
        ldr     r8,=cur_buf
        ldr     r9,[r8,#audio_buf_length]
        cmp     r9,#0
        beq     05f
        
        ldr     r9,[r8,#audio_buf_left_ptr]
        ldr     r10,[r8,#audio_buf_right_ptr]

// Move 4 [16-bit] values to each FIFO
        ldrh    r13,[r9],#2
        str     r13,[r11]
        ldrh    r13,[r10],#2
        str     r13,[r12]
        
        ldrh    r13,[r9],#2
        str     r13,[r11]
        ldrh    r13,[r10],#2
        str     r13,[r12]
        
        ldrh    r13,[r9],#2
        str     r13,[r11]
        ldrh    r13,[r10],#2
        str     r13,[r12]
        
        ldrh    r13,[r9],#2
        str     r13,[r11]
        ldrh    r13,[r10],#2
        str     r13,[r12]

// Update pointers
        str     r9,[r8,#audio_buf_left_ptr]
        str     r10,[r8,#audio_buf_right_ptr]

// Update count
        ldr     r9,[r8,#audio_buf_length]
        sub     r9,r9,#4
        str     r9,[r8,#audio_buf_length]
        cmp     r9,#0
        bgt     10f
        
// Switch to next buffer
05:     ldr     r9,=next_buf
        ldr     r10,[r9,#audio_buf_left_ptr]
        str     r10,[r8,#audio_buf_left_ptr]
        ldr     r10,[r9,#audio_buf_right_ptr]
        str     r10,[r8,#audio_buf_right_ptr]
        ldr     r10,[r9,#audio_buf_length]
        str     r10,[r8,#audio_buf_length]
        mov     r13,#0
        str     r13,[r9,#audio_buf_length]      // Free buffer
        cmp     r10,#0
        bne     10f

// Disable further interrupts
        ldr     r8,=INTMR3
        ldr     r9,[r8]
        bic     r9,r9,#INTSR3_I2SINT
        str     r9,[r8]

10:
        ldr     r11,=I2S_STAT   // Clear status bits
//        ldr     r10,=(I2S_STAT_RCTSR|I2S_STAT_RCRS|I2S_STAT_LCTSR|I2S_STAT_LCRSR|I2S_STAT_RCTNF|I2S_STAT_RCRNE|I2S_STAT_LCTNF|I2S_STAT_LCRNE)
        ldr     r10,=0xFFF0F
        str     r10,[r11]
        
        subs	pc,lr,#4	// return from interrupt

        .global hal_disable_interrupts
hal_disable_interrupts:             
        mrs     r0,cpsr                 // current state
        orr     r1,r0,#0x80             // mask IRQ
        msr     cpsr,r1
        bx      lr                      // exit, _old_ in r0        

        .data
        .global FIQ_count
FIQ_count:      
        .long   0        
