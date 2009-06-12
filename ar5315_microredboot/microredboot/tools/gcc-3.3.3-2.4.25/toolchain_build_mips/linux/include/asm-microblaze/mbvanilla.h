/*  /include/asm-microblaze/mbvanilla.h
 *
 *  (C) 2003 John Williams <jwilliams@itee.uq.edu.au>
 *
 *  Defines board specific stuff for a "vanilla" microblaze system
 *
 */

#ifndef _ASM_MICROBLAZE_MBVANILLA_H
#define _ASM_MICROBLAZE_MBVANILLA_H

/*
 * All Microblaze uClinux systems require a timer, interrupt controller
 * and UART.  Here, we define base addresses for each, then include
 * specific header files to get the rest.
 *
 * Note other, non-system peripherals may also exist (including UARTs and
 * timers), however these are handled differently (haven't worked out how yet!)
 */

#ifndef HZ
#define HZ 100
#endif

/* interrupt controller */
#ifndef MICROBLAZE_INTC_BASE_ADDR
#define MICROBLAZE_INTC_BASE_ADDR 0xFFFF3000
#endif

/* timer */
#ifndef MICROBLAZE_TIMER_BASE_ADDR
#define MICROBLAZE_TIMER_BASE_ADDR 0xFFFF1000
#endif
#define MICROBLAZE_TIMER_IRQ_NO 0

/* UART */
#ifndef MICROBLAZE_UART_BASE_ADDR
#define MICROBLAZE_UART_BASE_ADDR 0xFFFF2000
#endif
#define MICROBLAZE_UART_IRQ_NO 1

/* Memory controller */
#ifndef MICROBLAZE_MEMCON_BASE_ADDR
#define MICROBLAZE_MEMCON_BASE_ADDR 0xFFFF0000
#endif

/* GPIO */
#ifndef MICROBLAZE_GPIO_BASE_ADDR
#define MICROBLAZE_GPIO_BASE_ADDR 0xFFFF5000
#define MICROBLAZE_GPIO_DIR 0x0000FF00
#endif

/* Start and size of external RAM */
/* #define ERAM_ADDR 0xFFE00000
#define ERAM_SIZE 0x00100000 */
#define ERAM_ADDR 0x80000000
#define ERAM_SIZE 0x01000000

/* for <asm/page.h> */
#define PAGE_OFFSET ERAM_ADDR

/* Something to do with interrupts - not quite sure yet */
#define IRQ_RPU(n) 3+(n)
#define IRQ_RPU_NUM 4

#define NUM_CPU_IRQS 32

#endif

