//
// Simple test code framework
//
//-----------------------------------------------------------------
// Copyright (C) 2003, 2004 Gary Thomas <gary@mlbassoc.com>
//-----------------------------------------------------------------

#include "ppc.inc"        
        
#define GPIO0           (0xEF600700)
#define GPIO0_OR        (GPIO0+0x000)
#define GPIO0_TCR       (GPIO0+0x004)

        .text
        .globl  start
start:
        mfdcr   r3,1018
        mfdcr   r4,1019
        
        // Force caches to be totally clean
        iccci   0,r3

        lwi     r3,0
        lwi     r4,0x8000
10:     dccci   0,r3
        addi    r3,r3,16
        cmpw    r3,r4
        bne     10b        

#if 0 // Assume that 'boot' environment has set this up        
//
// Setup GPIO to drive LEDs
//
        lwi     r11,GPIO0_OR
        li      r4,0x80
        stw     r4,GPIO0_TCR-GPIO0(r11)
        stw     r4,GPIO0_OR-GPIO0(r11)
#endif        

        // Setup and clear a stack used by the C code
        lwi     sp,stack_base-4
        li      r3,(stack-stack_base)/4
        mtctr   r3
        li      r0,0
10:     stwu    r0,4(sp)
        bdnz    10b

        // Initialize the terminal environment
        bl      tty_init

        // Run the test code - it should never return                
        bl      run_test

        .section ".bss"
stack_base:             
        .rept   256
        .byte   0
        .endr        
stack:  

        .end
