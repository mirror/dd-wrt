/* 
 * linux/arch/arm/boot/compressed/head-ks8695.S
 * 
 * Some code borrowed from Nicolas Pitre's 'head-sa1100.S' file. This
 * is merged with head.S by the linker.
 */

#include <asm/mach-types.h>
#include <mach/regs-mem.h>
#include <mach/hardware.h>

#ifndef CONFIG_ARCH_KS8695
#error What am I doing here...
#endif

		.section        ".start", "ax"

__KS8695_start:

#ifdef CONFIG_MACH_KS8695_VSOPENRISC

        mov r7, #MACH_TYPE_KS8695 @ normaly r1, but it was saved in r7 from head.S
                                  @ we override this, because our bootloader set it to a wrong value
        b   __reg_ks8695_end
__reg_ks8695_end:

#endif

