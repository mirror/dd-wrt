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

        ldr r8, __reg_flash_ctrl1   @ deactivate flash bank 1
        mov r9, #0
        str r9, [r8]

	ldr r8, __reg_flash_ctrl0   @ we set it here, to get not overlapped with the SDRAM
        ldr r9, __reg_flash_value0  @ it's "real" mapped again in our arch kernel directory (vsopenrisc.c)
        str r9, [r8]

        ldr r8, __reg_sdram_ctrl1   @ deactivate sdram bank 1
        mov r9, #0
        str r9, [r8]

        ldr r8, __reg_sdram_ctrl0
        ldr r9, __reg_sdram_value0
        str r9, [r8]

        b   __reg_ks8695_end

__reg_flash_ctrl0:
        .long   KS8695_MEM_PA + KS8695_ROMCON0
__reg_flash_value0:
        .long   ((VSOPENRISC_FLASH_BASE + VSOPENRISC_FLASH_SIZE - 1) << 6) & 0xffc00000 \
                | (VSOPENRISC_FLASH_BASE >> 4) \
                | 0x7c                                                  @ 22-31: RB0NPTR, 12-21: RB0BPTR, 0-6: timg, mode
__reg_flash_ctrl1:
        .long   KS8695_MEM_PA + KS8695_ROMCON1
__reg_sdram_ctrl0:
        .long   KS8695_MEM_PA + KS8695_SDCON0
__reg_sdram_value0:
        .long   ((VSOPENRISC_SDRAM_BASE + VSOPENRISC_SDRAM_SIZE - 1) << 6) & 0xffc00000 \
                | (VSOPENRISC_SDRAM_BASE >> 4) \
                | 0x10e                                                  @ 22-31: DB0NPTR, 12-21: DB0BPTR, 0-9: col(9bit), banks, width
__reg_sdram_ctrl1:
        .long   KS8695_MEM_PA + KS8695_SDCON1

__reg_ks8695_end:

#endif

