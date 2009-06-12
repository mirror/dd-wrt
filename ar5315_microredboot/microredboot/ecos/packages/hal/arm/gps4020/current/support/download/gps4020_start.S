//-------------------------------------------------------------------
//
// gp4020_start.S
//
//-------------------------------------------------------------------
//
// Copyright (C) 2003, MLB Associates.        
//
// This file contains basic startup code for the GPS4020 platform.
// After initialization, it will call a single "C" function 'main()'
//
// Note: this program assumes that it has been downloaded into the
// on-chip SRAM via the BOOTSTRAP mechanism, so on-chip SRAM is
// located at address 0x00000000 and external ROM/FLASH is at
// address 0x60000000
//

	.macro  lab,lbl
.\lbl:  .long   \lbl    
	.endm

	.macro  blr
	mov     pc,lr
        .endm
	    
	.equ    MPC_BASE_ADDRESS,         0xE0008000
	.equ    MPC_AREA1_CONFIG,         0x00000000
	.equ    MPC_AREA2_CONFIG,         0x00000004
	.equ    MPC_AREA3_CONFIG,         0x00000008
	.equ    MPC_AREA4_CONFIG,         0x0000000C
	.equ    GPIO_BASE_ADDRESS,        0xE0005000
	.equ    GPIO_DIRECTION_REG_OFFSET,0x00000000
	.equ    GPIO_READ_REG_OFFSET,     0x00000004
	.equ    GPIO_WRITE_REG_OFFSET,    0x00000008
	.equ    UART1_BASE_ADDRESS,       0xE0018000
	.equ    UART_TXBUF,               0x00000010
	.equ    UART_STATUS,              0x0000000C
	.equ    UART_STATUS_TXEMPTY,      0x00000002

	.text
	.global __exception_handlers
__exception_handlers:
	 b       reset                           // 0x00
	 ldr     pc,.undefined_instruction       // 0x04
	 ldr     pc,.software_interrupt          // 0x08 start && software int
	 ldr     pc,.abort_prefetch              // 0x0C
	 ldr     pc,.abort_data                  // 0x10
	 .word   0                               // unused
	 ldr     pc,.IRQ                         // 0x18
	 ldr     pc,.FIQ                         // 0x1C
	                        
	 .global vectors
vectors:
	 .long   reset                           // 0x20
	 lab     undefined_instruction           // 0x24
	 lab     software_interrupt              // 0x28
	 lab     abort_prefetch                  // 0x2C
	 lab     abort_data                      // 0x30
	 .word   0                               // 0x34
	 lab     IRQ                             // 0x38
	 lab     FIQ                             // 0x3C

	.arm        @ AKA ".code32"
	.align 2     @ Align code on 2^2 = 4 byte intervals

	.global start
start:
reset:          

/*--------------------------------------------------------------------------*/
// Setup the GPIO and the ARM registers

// make GPIO[0..7] outputs - note: GPIO4 is used for Rx on UART1!
	    
	ldr     r1,=GPIO_BASE_ADDRESS
	ldr    r0, [r1,#GPIO_DIRECTION_REG_OFFSET]
	and    r0, r0, #0b00010000    @ 0 = output, 1 = input
	str    r0, [r1,#GPIO_DIRECTION_REG_OFFSET]        @ Store value (DON'T use strb)

        ldr     r1,=MPC_BASE_ADDRESS
        ldr     r2,=0xFF00002D                  // 0x6xxxxxxx, 16bit R/W RAM
        str     r2,[r1,#MPC_AREA1_CONFIG]

        ldr     r1,=MPC_BASE_ADDRESS
        ldr     r2,=0x00000069                  // 0x2xxxxxxx, 16bit R/W RAM
        str     r2,[r1,#MPC_AREA2_CONFIG]

        ldr     r2,[r1,#MPC_AREA3_CONFIG]
        ldr     r2,=0x00000021                  // 0x4xxxxxxx, 16bit peripheral
        str     r2,[r1,#MPC_AREA3_CONFIG]

        ldr     r2,[r1,#MPC_AREA4_CONFIG]
        ldr     r2,=0x0000006E                  // 0x0xxxxxxx, 32bit memory
        str     r2,[r1,#MPC_AREA4_CONFIG]
        
	ldr     sp,=_stack                
	bl      main

10:     ldr     r1,=GPIO_BASE_ADDRESS
	mov     r0,#0
	str    r0, [r1,#GPIO_WRITE_REG_OFFSET]
	b       10b

//
// Exception handlers
//        
undefined_instruction:      // 0x24
	mov     r0,#0x24
	b       exception
software_interrupt:         // 0x28
	mov     r0,#0x28
	b       exception
abort_prefetch:             // 0x2C
	mov     r0,#0x2C
	b       exception
abort_data:                 // 0x30
	mov     r0,#0x30
	b       exception
IRQ:                        // 0x38
	mov     r0,#0x38
	b       exception
FIQ:                        // 0x3C
	mov     r0,#0x3C
	b       exception

exception:
        mov     r8,lr       // Location of failure
	mov     r9,r0
00:             
        ldr     r1,=GPIO_BASE_ADDRESS
	str    r9, [r1,#GPIO_WRITE_REG_OFFSET]
	ldr     r3,=0x100000
10:     nop
	sub     r3,r3,#1
	cmp     r3,#0
	bne     10b
	str     r3,[r1,#GPIO_WRITE_REG_OFFSET]
	ldr     r3,=0x80000
20:     nop
	sub     r3,r3,#1
	cmp     r3,#0
	bne     20b

	ldr     r1,=msg
	bl      _puts
	mov     r1,r8,lsr #24
	bl      _puthex
	mov     r1,r8,lsr #16
	bl      _puthex
	mov     r1,r8,lsr #8
	bl      _puthex
	mov     r1,r8
	bl      _puthex
	mov     r0,#'\n'
	bl      _putc
	mov     r0,#'\r'
	bl      _putc
	b       00b

//
// Write the character in r0
//        
_putc:          
	ldr     r2,=UART1_BASE_ADDRESS
10:     ldrb    r3,[r2,#UART_STATUS]
	and     r3,r3,#UART_STATUS_TXEMPTY
	cmp     r3,#0
	beq     10b
	strb    r0,[r2,#UART_TXBUF]
	blr

//
// Write the string pointed to by r1
//
_puts:
	mov     r7,lr
10:     ldrb    r0,[r1],#1
	cmp     r0,#0
	moveq   pc,r7
	bl      _putc
	b       10b                                        

//
// Print the byte in r1 in hex
//
_puthex:
	mov     r7,lr
	mov     r2,r1,lsr #4
	and     r2,r2,#0x0F
	cmp     r2,#0x0A
	addge   r2,r2,#'A'-'0'-0x0A
        add     r0,r2,#'0'
	bl      _putc
	and     r2,r1,#0x0F
	cmp     r2,#0x0A
	addge   r2,r2,#'A'-'0'-0x0A
        add     r0,r2,#'0'
	bl      _putc
	mov     lr,r7        
	blr

msg:    .string "PC=0x"        
	                                    
	.section ".data"
	.align  2
_leds:  .long   0x77        

        .section ".bss"
_stack_base:
        .rept 512
        .byte 0
        .endr
        .balign 16
_stack:

        .end
