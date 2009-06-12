/*
 * linux/include/asm-armnommu/arch-swarm/hardware.h
 *
 * Copyright (C) 1996 Russell King.
 * 09 Sep 2001 - C Hanish Menon [www.hanishkvc.com]
 *   - Copied for armnommu/swarm and updated has required
 *   
 * This file contains the hardware definitions of the SWARM Emulator
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

/* ARM clock rate (MHz) */
#define SWARM_CLOCK_RATE     00010000

/* uarts (note that the register fields aren't needed by the standard linux
   serial driver -- we only use them for our initial debugging console) */
#define SWARM_UART0_BASE     0x90081000
#define SWARM_UART0_TXBASE   0x90081000
#define SWARM_UART0_RXBASE   0x90081004
#define SWARM_UART0_CR       0x90081008
#define SWARM_UART0_SR       0x9008100c

/* timers */
#define SWARM_TIMER_BASE     0x90000000
#define SWARM_TIMER_MATCH0   0x90000000
#define SWARM_TIMER_MATCH1   0x90000004
#define SWARM_TIMER_MATCH2   0x90000008
#define SWARM_TIMER_MATCH3   0x9000000c
#define SWARM_TIMER_CNT      0x90000010
#define SWARM_TIMER_STATUS   0x90000014
#define SWARM_TIMER_WDOG     0x90000018
#define SWARM_TIMER_INTEN    0x9000001C

#define SWARM_TIMER_SYS_COUNT 400000

/* interrupt */
#define SWARM_INT_BASE       0x90050000
#define SWARM_INT_IRQ_STATUS 0x90050000
#define SWARM_INT_MASK       0x90050004
#define SWARM_INT_LEVEL      0x90050008
#define SWARM_INT_FIQ_STATUS 0x9005000c

/*
 * HARD_RESET_NOW -- used in blkmem.c. Should call arch_hard_reset(), but I 
 * don't appear to have one ;).
 * --gmcnutt
 */
#define HARD_RESET_NOW()


#endif  /* _ASM_ARCH_HARDWARE_H */

