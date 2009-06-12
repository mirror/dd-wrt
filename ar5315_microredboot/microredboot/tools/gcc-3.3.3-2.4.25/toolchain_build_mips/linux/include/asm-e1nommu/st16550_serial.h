#ifndef __HYPERSTONE_NOMMU_SERIAL_H
#define __HYPERSTONE_NOMMU_SERIAL_H

#include <linux/config.h> 
#include <asm/io.h> 

#ifdef CONFIG_EMBEDDED_SERIAL_CONSOLE
static unsigned long Debug_UART = (((CONFIG_EMBEDDED_SERIAL_CONSOLE_CS + 4) << 22) | ( 0x3 << IOSetupTime ) | (0x7 << IOAccessTime) | (0x3 << IOHoldTime)) ;
#endif

#define XTAL 7372800 /* The oscilator's frequency */

#define FIFO_ENABLE 0x07
#define INT_ENABLE  0x04	/* default interrupt mask */


struct low_level_serial {
	int port;
	unsigned long base_addr;
	unsigned long chipselect;
	unsigned long addr_offset;
	int SetupTime;
	int AccessTime;
	int HoldTime;
};


#endif /* __HYPERSTONE_NOMMU_SERIAL_H */
