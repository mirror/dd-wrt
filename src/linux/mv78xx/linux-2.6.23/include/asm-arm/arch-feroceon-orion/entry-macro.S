/*
 * include/asm-arm/arch-feroceon/entry-macro.S
 *
 * Low-level IRQ helper macros for Integrator platforms
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <asm/hardware.h>
#include <asm/arch/irqs.h>


        .macro  disable_fiq
        .endm
        
        .macro  get_irqnr_preamble, base, tmp
        .endm

         .macro  arch_ret_to_user, tmp1, tmp2
         .endm

#if defined(CONFIG_CPU_BIG_ENDIAN)
        	.macro	HTOLL sr, tt		@ sr   = A  ,B  ,C  ,D    
        	eor \tt, \sr, \sr, ror #16 ;    @ temp = A^C,B^	  
        	bic \tt, \tt, #0xff0000 ;       @ temp = A^C,0  ,C^A,D^B
        	mov \sr, \sr, ror #8 ;          @ sr   = D  ,A  ,B  ,C
        	eor \sr, \sr, \tt, lsr #8       @ sr   = D  ,C  ,B  ,A
        	.endm
#else		
        	.macro	HTOLL sr, tt	
        	.endm
#endif				

                /*
                 * This macro should retunr the follows:
                 *      irqnr - the number of the received irq
                 *      eq flag - not set if succeed and set if fail.
                 */
                .macro  get_irqnr_and_base, irqnr, irqstat, base, tmp
                ldr     \base, =MV_ASM_IRQ_BASE_REG
                ldr     \tmp, [\base, #MV_ASM_IRQ_MASK_REG]                     @ get mask
                HTOLL	\tmp, \base
                mov     \irqnr, #IRQ_START
                ldr	\base, =MV_ASM_IRQ_BASE_REG				
                ldr     \irqstat, [\base, #MV_ASM_IRQ_CAUSE_REG]                @ get cause
		HTOLL	\irqstat, \base
                ldr	\base, =MV_ASM_IRQ_BASE_REG
                ands    \irqstat, \irqstat, \tmp                        @ and mask and cause
                beq     1004f                                           @ if eq to 0 (flag set) error return, else go to 1001

1001:           tst     \irqstat, #0xff                                 @ find the first bit which is '1'
                moveq   \irqstat, \irqstat, lsr #8
                addeq   \irqnr, \irqnr, #8
                tsteq   \irqstat, #0xff
                moveq   \irqstat, \irqstat, lsr #8
                addeq   \irqnr, \irqnr, #8
                tsteq   \irqstat, #0xff
                moveq   \irqstat, \irqstat, lsr #8
                addeq   \irqnr, \irqnr, #8
                tst     \irqstat, #0x0f
                moveq   \irqstat, \irqstat, lsr #4
                addeq   \irqnr, \irqnr, #4
                tst     \irqstat, #0x03
                moveq   \irqstat, \irqstat, lsr #2
                addeq   \irqnr, \irqnr, #2
                tst     \irqstat, #0x01
                addeq   \irqnr, \irqnr, #1

1002:           teq     \irqnr, #IRQ_GPP_0_7                            @ check if it a Gpp int 0 - 7
                beq     1003f
                teq     \irqnr, #IRQ_GPP_8_15                           @ check if its a Gpp int 8 - 15
                beq     1003f
#if defined(CONFIG_ARCH_FEROCEON)
#if (CONFIG_MV_GPP_MAX_PINS == 32)
                teq     \irqnr, #IRQ_GPP_16_23                          @ check if it a Gpp int 16 - 23
                beq     1003f
                teq     \irqnr, #IRQ_GPP_24_31                          @ check if its a Gpp int 24 - 31
                beq     1003f
#endif
#endif
                teq     \irqnr, #64                                     @ if irq isn't 64 unset the flag
                b       1004f

1003:           ldr     \base, =MV_ASM_GPP_IRQ_BASE_REG
		ldr     \tmp, [\base, #MV_ASM_GPP_IRQ_MASK_REG]
		HTOLL	\tmp, \base
		ldr	\base, =MV_ASM_GPP_IRQ_BASE_REG				
                ldr     \irqstat, [\base, #MV_ASM_GPP_IRQ_CAUSE_REG]    @ get cause
		HTOLL	\irqstat, \base
                ldr	\base, =MV_ASM_GPP_IRQ_BASE_REG				
                mov     \irqnr, #IRQ_ASM_GPP_START
                ands    \irqstat, \irqstat, \tmp                        @ and mask and cause
                bne     1001b                                           @ if eq to 0 (flag set) error return, else go to 1001

1004:
                .endm

