/*
 *  PMC-Sierra Inc. Stretch Board
 *  Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  KGDB support for the PMC-Sierra Stretch board
 *
 */

#include <linux/config.h>

#if defined(CONFIG_REMOTE_DEBUG)

#include <asm/serial.h> /* For the serial port location and base baud */


typedef unsigned char uint8;
typedef unsigned int uint32;

/*
 * Baud rate
 */
#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

/*
 * Parity bit 
 */
#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

/*
 * Data bit
 */
#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

/*
 * Stop bit
 */
#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4

/*
 * Serial port defines
 */
#define         BASE                    PMC_STRETCH_SERIAL1_BASE
#define         MAX_BAUD                PMC_STRETCH_BASE_BAUD


#define         REG_OFFSET              2

/* 
 * register offset 
 */
#define         OFS_RCV_BUFFER          0
#define         OFS_TRANS_HOLD          0
#define         OFS_SEND_BUFFER         0
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


/* 
 * memory-mapped read/write of the port 
 */
#define         UART16550_READ(y)    (*((volatile uint8*)(BASE + y)))
#define         UART16550_WRITE(y, z)  ((*((volatile uint8*)(BASE + y))) = z)

void debugInit(uint32 baud, uint8 data, uint8 parity, uint8 stop)
{
	/* disable interrupts */
	UART16550_WRITE(OFS_INTR_ENABLE, 0);

	/* set up buad rate */
	{
		uint32 divisor;

		/* set DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x80);

		/* set divisor */
		divisor = MAX_BAUD / baud;
		UART16550_WRITE(OFS_DIVISOR_LSB, divisor & 0xff);
		UART16550_WRITE(OFS_DIVISOR_MSB, (divisor & 0xff00) >> 8);

		/* clear DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	}

	/* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, data | parity | stop);
}

static int remoteDebugInitialized = 0;

uint8 getDebugChar(void)
{
	if (!remoteDebugInitialized) {
		remoteDebugInitialized = 1;
		debugInit(UART16550_BAUD_38400,
			  UART16550_DATA_8BIT,
			  UART16550_PARITY_NONE, UART16550_STOP_1BIT);
	}

	while ((UART16550_READ(OFS_LINE_STATUS) & 0x1) == 0);
	return UART16550_READ(OFS_RCV_BUFFER);
}


int putDebugChar(uint8 byte)
{
	if (!remoteDebugInitialized) {
		remoteDebugInitialized = 1;
		debugInit(UART16550_BAUD_38400,
			  UART16550_DATA_8BIT,
			  UART16550_PARITY_NONE, UART16550_STOP_1BIT);
	}

	while ((UART16550_READ(OFS_LINE_STATUS) & 0x20) == 0);
	UART16550_WRITE(OFS_SEND_BUFFER, byte);
	return 1;
}

#endif
