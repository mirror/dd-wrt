/*
 * linux/include/asm-armnommu/arch-ta7s/serial.h
 * 2003	CSH	Created
 */
#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H

#include <asm/arch/hardware.h>
#include <asm/irq.h>

#define RS_TABLE_SIZE	2
#define BASE_BAUD	115200
#define STD_COM_FLAGS	(ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)

#define STD_SERIAL_PORT_DEFNS		\
	/* UART CLK PORT IRQ FLAGS */	\
	{ 0, BASE_BAUD, UART0_RX_TX_REG, IRQ_SERIAL_0, STD_COM_FLAGS },	/* ttyS0 */	\
	{ 0, BASE_BAUD, UART1_RX_TX_REG, IRQ_SERIAL_1, STD_COM_FLAGS },	/* ttyS1 */
#define EXTRA_SERIAL_PORT_DEFNS

extern unsigned long a7hal_clock_getFreq( int );

unsigned int baudrate_div(unsigned int baudrate)
{
	return(((a7hal_clock_getFreq(0)/16)/baudrate)-1);
}

#endif /* __ASM_ARCH_SERIAL_H */
