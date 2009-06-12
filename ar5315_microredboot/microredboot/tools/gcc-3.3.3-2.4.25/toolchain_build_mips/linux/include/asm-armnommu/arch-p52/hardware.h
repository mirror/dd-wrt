/*
 * linux/include/asm-arm/arch-p52/hardware.h
 * to complete.... 
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H


#define HARD_RESET_NOW()

/*
 * GPIO
 */

#define P52GPIO_ISM1  0x3500a0
#define P52GPIO_ISM2  0x3500a4
#define P52GPIO_ISM3  0x3500a8

#define P52GPIO_OE1   0x3500b4
#define P52GPIO_OE2   0x3500b8
#define P52GPIO_OE3   0x3500bc

#define P52GPIO_DIN1  0x3500c0
#define P52GPIO_DIN2  0x3500c4
#define P52GPIO_DIN3  0x3500c8

#define P52GPIO_DOUT1 0x3500cc
#define P52GPIO_DOUT2 0x3500d0
#define P52GPIO_DOUT3 0x3500d4

#define P52GPIO_ISR1  0x3500d8
#define P52GPIO_ISR2  0x3500dc
#define P52GPIO_ISR3  0x3500e0

#define P52GPIO_IER1  0x3500e4
#define P52GPIO_IER2  0x3500e8
#define P52GPIO_IER3  0x3500ec

#define P52GPIO_IPC1  0x3500f0
#define P52GPIO_IPC2  0x3500f4
#define P52GPIO_IPC3  0x3500f8

/* 
 * interrupts 
 */

#define INTBase_Addr  0x350040

#define LA            0x00
#define Stat          0x04
#define SetStat       0x08
#define Msk           0x0c
#define MStat         0x50

#define	P52INT_MASK     (INTBase_Addr+Msk) 
#define	P52INT_STATUS   (INTBase_Addr+Stat)
#define P52INT_STATUS_M (INTBase_Addr+MStat)

/*
 * timers
 */

#define P52TIMBase_Addr     0x350020

#define TM_Cnt1          0x00
#define TM_Cnt2          0x04
#define TM_Cnt3          0x08
#define TM_Cnt4          0x0C

#define TM_Lmt1          0x10
#define TM_Lmt2          0x14
#define TM_Lmt3          0x18
#define TM_Lmt4          0x1C

#define TIM_SET_RATE(tmr, value) *((volatile unsigned long *)(P52TIMBase_Addr+tmr)) = value


#define ASB_RAM_ADRS      0x00180000
#define P52_CLK_SPEED     1000000         /* Timer speed is 1 microsecond*/

#endif  /* _ASM_ARCH_HARDWARE_H */


