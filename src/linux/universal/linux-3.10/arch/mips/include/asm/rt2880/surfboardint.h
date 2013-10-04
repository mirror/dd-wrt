/*
 * Copyright (C) 2001 Palmchip Corporation.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 * Defines for the Surfboard interrupt controller.
 *
 */
#ifndef _SURFBOARDINT_H
#define _SURFBOARDINT_H

/* Number of IRQ supported on hw interrupt 0. */
#if defined (CONFIG_RALINK_RT2880)
#define RALINK_CPU_TIMER_IRQ 	 6	/* mips timer */
#define SURFBOARDINT_GPIO	 7	/* GPIO */
#define SURFBOARDINT_UART1	 8	/* UART Lite */
#define SURFBOARDINT_UART	 9	/* UART */
#define SURFBOARDINT_TIMER0	 10	/* timer0 */
#elif defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT2883) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined(CONFIG_SOC_MT7620_OPENWRT)
#define RALINK_CPU_TIMER_IRQ 	 5	/* mips timer */
#define SURFBOARDINT_GPIO	 6	/* GPIO */
#define SURFBOARDINT_DMA	 7	/* DMA */
#define SURFBOARDINT_NAND	 8	/* NAND */
#define SURFBOARDINT_PC	 	 9	/* Performance counter */
#define SURFBOARDINT_I2S 	 10	/* I2S */
#define SURFBOARDINT_ESW	 17	/* ESW */
#define SURFBOARDINT_UART1	 12 	/* UART Lite */
#define SURFBOARDINT_CRYPTO      13     /* CryptoEngine */
#define SURFBOARDINT_PLC	 24     /* PLC */
#define SURFBOARDINT_SYSCTL 	 32	/* SYSCTL */
#define SURFBOARDINT_TIMER0	 33	/* timer0 */
#define SURFBOARDINT_WDG	 34	/* watch dog */
#define SURFBOARDINT_ILL_ACC	 35	/* illegal access */
#define SURFBOARDINT_PCM	 36	/* PCM */
#define SURFBOARDINT_UART	 37	/* UART */
#define RALINK_INT_PCIE0         13	/* PCIE0 */
#define RALINK_INT_PCIE1	 14	/* PCIE1 */
#elif defined (CONFIG_RALINK_RT3883)
#define RALINK_CPU_TIMER_IRQ     5      /* mips timer */
#define SURFBOARDINT_GPIO        6      /* GPIO */
#define SURFBOARDINT_DMA         7      /* DMA */
#define SURFBOARDINT_NAND        8      /* NAND */
#define SURFBOARDINT_PC          9      /* Performance counter */
#define SURFBOARDINT_I2S         10     /* I2S */
#define SURFBOARDINT_UART1       12     /* UART Lite */
#define SURFBOARDINT_PCI         18     /* PCI */
#define SURFBOARDINT_UDEV        19     /* USB Device */
#define SURFBOARDINT_UHST        20     /* USB Host */
#define SURFBOARDINT_SYSCTL      32     /* SYSCTL */
#define SURFBOARDINT_TIMER0      33     /* timer0 */
#define SURFBOARDINT_ILL_ACC     35     /* illegal access */
#define SURFBOARDINT_PCM         36     /* PCM */
#define SURFBOARDINT_UART        37     /* UART */
#endif

#define SURFBOARDINT_END 	 64
#define RT2880_INTERINT_START 	 40

/* Global interrupt bit definitions */
#define C_SURFBOARD_GLOBAL_INT	31
#define M_SURFBOARD_GLOBAL_INT	(1 << C_SURFBOARD_GLOBAL_INT)

/* added ??? */
#define RALINK_SDRAM_ILL_ACC_ADDR  *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x310)
#define RALINK_SDRAM_ILL_ACC_TYPE  *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x314)
/* end of added, bobtseng */

/*
 * Surfboard registers are memory mapped on 32-bit aligned boundaries and
 * only word access are allowed.
 */
#if defined (CONFIG_RALINK_MT7621)
#define RALINK_IRQ0STAT		(RALINK_INTCL_BASE + 0x9C) //IRQ_STAT
#define RALINK_IRQ1STAT		(RALINK_INTCL_BASE + 0xA0) //FIQ_STAT
#define RALINK_INTTYPE		(RALINK_INTCL_BASE + 0x6C) //FIQ_SEL
#define RALINK_INTRAW		(RALINK_INTCL_BASE + 0xA4) //INT_PURE
#define RALINK_INTENA		(RALINK_INTCL_BASE + 0x80) //IRQ_MASK_SET
#define RALINK_INTDIS		(RALINK_INTCL_BASE + 0x78) //IRQ_MASK_CLR
#else
#define RALINK_IRQ0STAT		(RALINK_INTCL_BASE + 0x0)
#define RALINK_IRQ1STAT		(RALINK_INTCL_BASE + 0x4)
#define RALINK_INTTYPE		(RALINK_INTCL_BASE + 0x20)
#define RALINK_INTRAW		(RALINK_INTCL_BASE + 0x30)
#define RALINK_INTENA		(RALINK_INTCL_BASE + 0x34)
#define RALINK_INTDIS		(RALINK_INTCL_BASE + 0x38)
#endif

/* bobtseng added ++, 2006.3.6. */
#define read_32bit_cp0_register(source)                         \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
        ".set\tpush\n\t"                                        \
        ".set\treorder\n\t"                                     \
        "mfc0\t%0,"STR(source)"\n\t"                            \
        ".set\tpop"                                             \
        : "=r" (__res));                                        \
        __res;})
        
#define write_32bit_cp0_register(register,value)                \
        __asm__ __volatile__(                                   \
        "mtc0\t%0,"STR(register)"\n\t"                          \
        "nop"                                                   \
        : : "r" (value));
        
/* bobtseng added --, 2006.3.6. */

void surfboardint_init(void);
u32 get_surfboard_sysclk(void);


#endif /* !(_SURFBOARDINT_H) */
