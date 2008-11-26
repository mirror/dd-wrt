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

#define CPU_CORE1       0x4000

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

        .macro  update_base_for_cpu, b, t
#if defined(CONFIG_MV78200)
         add	\b, \b, r2
#endif
        .endm

         /*
         * This macro should return the follows:
         *      irqnr - the number of the received irq
         *      eq flag - not set if succeed and set if fail.
         */
         .macro  get_irqnr_and_base, irqnr, irqstat, base, tmp
#if defined(CONFIG_MV78200)
	/*we can use r2 here*/
	mrc	p15, 1, r2, c15, c1, 0	 @ get core Id
	and 	r2, r2, #CPU_CORE1
#endif
         ldr     \base, =MV_ASM_CPU_MASK_LOW_REG
	update_base_for_cpu \base, \tmp
         ldr     \tmp, [\base]    			                @ get low mask
         HTOLL   \tmp, \base
         mov     \irqnr, #IRQ_START
         ldr     \base, =MV_ASM_CPU_CAUSE_LOW_REG				
	update_base_for_cpu \base, \tmp
         ldr     \irqstat, [\base]                			@ get low cause
         HTOLL   \irqstat, \base
         ands    \irqstat, \irqstat, \tmp
         beq      1001f							     @ if eq to 0 jump to high cause
         clz     \irqnr, \irqstat
         mov     \base, #31
         subs    \irqnr, \base, \irqnr
         teq     \irqnr, #32
         b	1004f		@ update irqnr and return
1001:
         ldr     \base, =MV_ASM_CPU_MASK_HIGH_REG
	update_base_for_cpu \base, \tmp
         ldr     \tmp, [\base]    			                @ get high mask
         HTOLL	  \tmp, \base
         ldr     \base, =MV_ASM_CPU_CAUSE_HIGH_REG				
	update_base_for_cpu \base, \tmp
         ldr     \irqstat, [\base]                			@ get high cause
         HTOLL   \irqstat, \base
         ands    \irqstat, \irqstat, \tmp                        
         beq     1004f             @ if eq to 0 -> finish
         clz     \irqnr, \irqstat
         mov     \base, #31
         subs    \irqnr, \base, \irqnr
         cmp	\irqnr, #24					@ GPP 0-7	
         beq	1002f
         cmp	\irqnr, #25					@ GPP 8-15
         beq	1002f
         cmp	\irqnr, #26					@ GPP 16-23
         beq	1002f
         add     \irqnr,\irqnr, #32
         teq     \irqnr, #64                           
         b 	1004f
1002:
         ldr     \base, =MV_ASM_GPP_IRQ_MASK_REG
	update_base_for_cpu \base, \tmp
         ldr     \tmp, [\base]    			                @ get gpp mask
         HTOLL   \tmp, \base
         ldr     \base, =MV_ASM_GPP_IRQ_CAUSE_REG				
	update_base_for_cpu \base, \tmp
         ldr     \irqstat, [\base]                			@ get gpp data
         HTOLL    \irqstat, \base
         ands     \irqstat, \irqstat, \tmp                        
         beq	1004f							          @ if eq to 0 -> finish
         clz     \irqnr, \irqstat
         mov     \base, #31
         subs    \irqnr, \base, \irqnr
         add     \irqnr,\irqnr, #64
         teq     \irqnr, #96
1004:
         .endm

