/*
 * timers.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the 32-bit timers definitions 
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */
 
#ifndef __ASM_ARCH_TIMERS_H
#define __ASM_ARCH_TIMERS_H

#define	KERNEL_TIMER	0

#define	TMOD		0xF0040000	/* Timer mode R		*/
#define	IC		0xF0040004	/* Interrupt clear R 	*/
#define	WDT		0xF0040008	/* Wathdog timer R 	*/
#define TDATA0		0xF0040010	/* Timer 0 data R 	*/
#define TCNT0		0xF0040014	/* Timer 0 counter R 	*/
#define TDATA1		0xF0040018	/* Timer 1 data R 	*/
#define TCNT1		0xF004001C	/* Timer 1 counter R 	*/
#define TDATA2		0xF0040020	/* Timer 2 data R 	*/
#define TCNT2		0xF0040024	/* Timer 2 counter R 	*/
#define TDATA3		0xF0040028	/* Timer 3 data R 	*/
#define TCNT3		0xF004002C	/* Timer 3 counter R 	*/
#define TDATA4		0xF0040030	/* Timer 4 data R 	*/
#define TCNT4		0xF0040034	/* Timer 4 counter R 	*/
#define TDATA5		0xF0040038	/* Timer 5 data R 	*/
#define TCNT5		0xF004003C	/* Timer 5 counter R 	*/

#define TIMER_BASE 	TMOD

#ifndef __ASSEMBLER__
struct s3c2500_timer
{
  unsigned int tmr;
  unsigned int ic;
  unsigned int wdt;
  unsigned int reserved;
  unsigned int td[12];
};
#endif
	
#endif	/* __ASM_ARCH_TIMERS_H */
