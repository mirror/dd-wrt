#ifndef _ASM_SERIAL_H
#define _ASM_SERIAL_H

#include <linux/config.h>
#include <linux/kernel.h>

/*
 * This assumes you have a 7.3728 MHz clock for your UART.
 *
 * It'd be nice if someone built a serial card with a 24.576 MHz
 * clock, since the 16550A is capable of handling a top speed of 1.5
 * megabits/second; but this requires the faster clock.
 */
#define BASE_BAUD ( 7372800 / 16 )

#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)

#define RS_TABLE_SIZE  1

#define STD_SERIAL_PORT_DEFNS                   \
        /* UART CLK   PORT IRQ     FLAGS        */                      \
        { 0, BASE_BAUD, 0x01C00000, 2-1, STD_COM_FLAGS }  /* ttyS0 */

#define SERIAL_PORT_DFNS STD_SERIAL_PORT_DEFNS

#define irq_cannonicalize(x) (x)

#endif /* _ASM_SERIAL_H */
