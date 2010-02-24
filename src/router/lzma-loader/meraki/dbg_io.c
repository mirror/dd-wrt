/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright MontaVista Software Inc
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Basic support for polled character input/output
 * using the AR531X's serial port.
 */

#define CONFIG_AR5315 1


typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef unsigned char u_char;
typedef unsigned long u_long;
typedef char * va_list;
#include "ar531x.h"

#define         BASE                    AR5315_UART0 

/* distance in bytes between two serial registers */
#define         REG_OFFSET              4

/*
 *  * the default baud rate *if* we do serial init
 *   */
#define         BAUD_DEFAULT            UART16550_BAUD_9600

/* === END OF CONFIG === */

#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)


/* memory-mapped read/write of the port */
#define         UART16550_READ(y)    (*((volatile u8*)(BASE + y)))
#define         UART16550_WRITE(y, z)  ((*((volatile u8*)(BASE + y))) = z)

#define NULL ((void*)0)

#define	__va_size(type) \
	(((sizeof(type) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

#define	va_start(ap, last) \
	((ap) = (va_list)&(last) + __va_size(last))

#define	va_arg(ap, type) \
	(*(type *)((ap) += __va_size(type), (ap) - __va_size(type)))

#define	va_end(ap)	((void)0)

static void
putDebugChar(char byte)
{
	while ((UART16550_READ(OFS_LINE_STATUS) &0x20) == 0);
	UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

void puts(char *str)
{
int i=0;
while (str[i]!=0)
    {
    char ch = str[i++];
	if (ch == '\n')
		putDebugChar('\r');
	putDebugChar((char)ch);
    }
}

