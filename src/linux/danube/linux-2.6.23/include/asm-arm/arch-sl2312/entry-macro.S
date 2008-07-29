/*
 * include/asm-arm/arch-arm/entry-macro.S
 *
 * Low-level IRQ helper macros for ebsa110 platform.
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <asm/arch/platform.h>
#include <asm/arch/int_ctrl.h>


                .macro  disable_fiq
                .endm

                .macro  get_irqnr_preamble, base, tmp
                .endm

                .macro  arch_ret_to_user, tmp1, tmp2
                .endm

                .macro  get_irqnr_and_base, irqnr, irqstat, base, tmp
                ldr     \irqstat, =IRQ_STATUS(IO_ADDRESS(SL2312_INTERRUPT_BASE))
                ldr     \irqnr,[\irqstat]
                cmp     \irqnr,#0
                beq     2313f
                mov     \tmp,\irqnr
                mov     \irqnr,#0
2312:
                tst     \tmp, #1
                bne     2313f
                add     \irqnr, \irqnr, #1
                mov     \tmp, \tmp, lsr #1
                cmp     \irqnr, #31
                bcc     2312b
2313:
                .endm

                .macro  irq_prio_table
                .endm

