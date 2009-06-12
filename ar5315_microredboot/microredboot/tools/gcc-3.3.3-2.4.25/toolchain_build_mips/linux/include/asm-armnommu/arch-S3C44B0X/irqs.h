/*
 * uclinux/include/asm-armnommu/arch-S3C44B0X/irqs.h
 *
 * Copyright (C) 2003 Thomas Eschenbacher <eschenbacher@sympat.de>
 *
 * All IRQ numbers of the S3C44B0X CPU.
 *
 */

#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

#define NR_IRQS		26

#define S3C44B0X_INTERRUPT_EINT0      25      /* External int. 0 */
#define S3C44B0X_INTERRUPT_EINT1      24      /* External int. 1 */
#define S3C44B0X_INTERRUPT_EINT2      23      /* External int. 2 */
#define S3C44B0X_INTERRUPT_EINT3      22      /* External int. 3 */
#define S3C44B0X_INTERRUPT_EINT4567   21      /* External int. 4,5,6 and 7 */
#define S3C44B0X_INTERRUPT_TICK       20      /* Clock Tick int. */
#define S3C44B0X_INTERRUPT_ZDMA0      19      /* ZDMA 0 */
#define S3C44B0X_INTERRUPT_ZDMA1      18      /* ZDMA 1 */
#define S3C44B0X_INTERRUPT_BDMA0      17      /* BDMA 0 */
#define S3C44B0X_INTERRUPT_BDMA1      16      /* BDMA 1 */
#define S3C44B0X_INTERRUPT_WDT        15      /* Watchdog timer */
#define S3C44B0X_INTERRUPT_UERR       14      /* UART error */
#define S3C44B0X_INTERRUPT_TIMER0     13      /* Timer 0 zero-crossing */
#define S3C44B0X_INTERRUPT_TIMER1     12      /* Timer 1 zero-crossing */
#define S3C44B0X_INTERRUPT_TIMER2     11      /* Timer 2 zero-crossing */
#define S3C44B0X_INTERRUPT_TIMER3     10      /* Timer 3 zero-crossing */
#define S3C44B0X_INTERRUPT_TIMER4      9      /* Timer 4 zero-crossing */
#define S3C44B0X_INTERRUPT_TIMER5      8      /* Timer 5 zero-crossing */
#define S3C44B0X_INTERRUPT_URX0        7      /* UART0 receive */
#define S3C44B0X_INTERRUPT_URX1        6      /* UART1 receive */
#define S3C44B0X_INTERRUPT_IIC         5      /* IIC */
#define S3C44B0X_INTERRUPT_SIO         4      /* SIO */
#define S3C44B0X_INTERRUPT_UTX0        3      /* UART0 transmit */
#define S3C44B0X_INTERRUPT_UTX1        2      /* UART1 transmit */
#define S3C44B0X_INTERRUPT_RTCT        1      /* RTC time */
#define S3C44B0X_INTERRUPT_ADC         0      /* ADC interrupt */

#define S3C44B0X_INTERRUPT_RTC       S3C44B0X_INTERRUPT_TIMER5      /* system timer(5) */

#endif /* __ASM_ARCH_IRQS_H__ */
