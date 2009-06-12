/*************************************************************/
/*   Copyright (c) 2001 Xilinx, Inc.  All rights reserved.   */
/*                                                           */
/*************************************************************/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <asm/types.h>

/*
 * OPB Timer/Counter definitions
 */

/*
 * Masks for the control/status register
 */

#define TIMER_ENABLE_ALL    0x400
#define TIMER_PWM           0x200
#define TIMER_INTERRUPT     0x100
#define TIMER_ENABLE        0x080
#define TIMER_ENABLE_INTR   0x040
#define TIMER_RESET         0x020
#define TIMER_RELOAD        0x010
#define TIMER_EXT_CAPTURE   0x008
#define TIMER_EXT_COMPARE   0x004
#define TIMER_DOWN_COUNT    0x002
#define TIMER_CAPTURE_MODE  0x001

/*
 * Timer/counter register positions: Note that these are offsets from the timer base address
 */

#define TIMER_CONTROL_STATUS_0  0x0
#define TIMER_COMPARE_CAPTURE_0 0x4
#define TIMER_COUNTER_0         0x8
#define TIMER_CONTROL_STATUS_1  0x10
#define TIMER_COMPARE_CAPTURE_1 0x14
#define TIMER_COUNTER_1         0x18

/*
 * Macros to access the timer registers
 */

#define TIMER_TCSR0(base_addr) ((volatile __u32 *)(base_addr + TIMER_CONTROL_STATUS_0))
#define TIMER_TCCR0(base_addr) ((volatile __u32 *)(base_addr + TIMER_COMPARE_CAPTURE_0))
#define TIMER_TCR0(base_addr)  ((volatile __u32 *)(base_addr + TIMER_COUNTER_0))
#define TIMER_TCSR1(base_addr) ((volatile __u32 *)(base_addr + TIMER_CONTROL_STATUS_1))
#define TIMER_TCCR1(base_addr) ((volatile __u32 *)(base_addr + TIMER_COMPARE_CAPTURE_1))
#define TIMER_TCR1(base_addr)  ((volatile __u32 *)(base_addr + TIMER_COUNTER_1))

/*
* Note that for all the following macros, timer_number must be 0 or 1 
*/

#define TIMER_TCSR(base_addr, timer_number) ((volatile __u32 *)(base_addr + (timer_number ? 0x10 : 0) + TIMER_CONTROL_STATUS_0))
#define TIMER_TCCR(base_addr, timer_number) ((volatile __u32 *)(base_addr + (timer_number ? 0x10 : 0) + TIMER_COMPARE_CAPTURE_0))
#define TIMER_TCR(base_addr, timer_number)  ((volatile __u32 *)(base_addr + (timer_number ? 0x10 : 0) + TIMER_COUNTER_0))

/* 
 * Set the control/status register
 */

#define timer_set_csr(base_addr, timer_number, status_value) (*TIMER_TCSR(base_addr, timer_number) = status_value)

/*
 * Get the control/status register
 */

#define timer_get_csr(base_addr, timer_number) (*TIMER_TCSR(base_addr, timer_number))

/*
 * Read the time value from the timer/counter register
 */

#define timer_get_time(base_addr, timer_number) (*TIMER_TCR(base_addr, timer_number))

/*
 * Get the capture register
 */

#define timer_get_capture(base_addr, timer_number) (*TIMER_TCCR(base_addr, timer_number))

/*
 * Set the compare register
 */

#define timer_set_compare(base_addr, timer_number, compare_value) (*TIMER_TCCR(base_addr, timer_number) = compare_value)

/*
 * Start timing a portion of code
 */
extern void start_timer(__u32 base_addr, __u32 timer_number);

/*
 * Get the elapsed time since the start of timing using start_timer
 */

extern __u32 get_elapsed_time(__u32 base_addr, __u32 timer_number);

/*
 * Setup the specified timer to generate an interrupt at the specified rate
 * (expressed in Hertz) 
 */
void microblaze_timer_configure(unsigned timer, unsigned rate);

#endif
