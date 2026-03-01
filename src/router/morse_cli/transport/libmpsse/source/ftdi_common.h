/*!
 * \file ftdi_common.h
 *
 * \author FTDI
 * \date 20110127
 *
 * Copyright  2000-2014 Future Technology Devices International Limited
 *
 *
 * THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Project: libMPSSE
 * Module: common
 *
 * Revision History:
 * 0.1 - initial version
 * 0.2 - 20110708 - Changed MAX_CLOCK_RATE from 3.4 to 30MHz
 * 0.3 - 20111103 - Added MPSSE command definitions for fullduplex transfers
 * 0.4 - 20200428 - removed unnecessary files and directory structure
 */

#ifndef FTDI_COMMON_H
#define FTDI_COMMON_H

/* Default library version number. Overridden by makefile. */

#ifndef FT_VER_MAJOR
#define FT_VER_MAJOR		1
#endif // FT_VER_MAJOR
#ifndef FT_VER_MINOR
#define FT_VER_MINOR		0
#endif // FT_VER_MINOR
#ifndef FT_VER_BUILD
#define FT_VER_BUILD		1
#endif // FT_VER_BUILD

/******************************************************************************/
/*								Macro defines								  */
/******************************************************************************/
/* Macros to be called before starting and after ending communication over a MPSSE channel.
Implement the lock/unlock only if really required, otherwise keep as placeholders */
#define LOCK_CHANNEL(arg)	{;}
#define UNLOCK_CHANNEL(arg)	{;}

#define MIN_CLOCK_RATE 					0
#define MAX_CLOCK_RATE 					30000000

#define MIN_LATENCY_TIMER       		0
#define MAX_LATENCY_TIMER       		255
#define USB_INPUT_BUFFER_SIZE			65536
#define USB_OUTPUT_BUFFER_SIZE			65536
#define DISABLE_EVENT					0
#define DISABLE_CHAR					0
#define DEVICE_READ_TIMEOUT_INFINITE    0
#define DEVICE_READ_TIMEOUT 			5000
#define DEVICE_WRITE_TIMEOUT 			5000
#define INTERFACE_MASK_IN				0x00
#define INTERFACE_MASK_OUT				0x01
#define RESET_INTERFACE					0
#define ENABLE_MPSSE					0X02

/*MPSSE Control Commands*/
#define MPSSE_CMD_SET_DATA_BITS_LOWBYTE		0x80
#define MPSSE_CMD_SET_DATA_BITS_HIGHBYTE	0x82
#define MPSSE_CMD_GET_DATA_BITS_LOWBYTE		0x81
#define MPSSE_CMD_GET_DATA_BITS_HIGHBYTE	0x83

#define MPSSE_CMD_SEND_IMMEDIATE			0x87
#define MPSSE_CMD_ENABLE_3PHASE_CLOCKING	0x8C
#define MPSSE_CMD_DISABLE_3PHASE_CLOCKING	0x8D
#define MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO	0x9E

/*MPSSE Data Command - LSB First */
#define MPSSE_CMD_DATA_LSB_FIRST			0x08

/*MPSSE Data Commands - bit mode - MSB first */
#define MPSSE_CMD_DATA_OUT_BITS_POS_EDGE	0x12
#define MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE	0x13
#define MPSSE_CMD_DATA_IN_BITS_POS_EDGE		0x22
#define MPSSE_CMD_DATA_IN_BITS_NEG_EDGE		0x26
#define MPSSE_CMD_DATA_BITS_IN_POS_OUT_NEG_EDGE	0x33
#define MPSSE_CMD_DATA_BITS_IN_NEG_OUT_POS_EDGE	0x36


/*MPSSE Data Commands - byte mode - MSB first */
#define MPSSE_CMD_DATA_OUT_BYTES_POS_EDGE	0x10
#define MPSSE_CMD_DATA_OUT_BYTES_NEG_EDGE	0x11
#define MPSSE_CMD_DATA_IN_BYTES_POS_EDGE	0x20
#define MPSSE_CMD_DATA_IN_BYTES_NEG_EDGE	0x24
#define MPSSE_CMD_DATA_BYTES_IN_POS_OUT_NEG_EDGE	0x31
#define MPSSE_CMD_DATA_BYTES_IN_NEG_OUT_POS_EDGE	0x34


/*SCL & SDA directions*/
#define DIRECTION_SCLIN_SDAIN				0x10
#define DIRECTION_SCLOUT_SDAIN				0x11
#define DIRECTION_SCLIN_SDAOUT				0x12
#define DIRECTION_SCLOUT_SDAOUT				0x13

/*SCL & SDA values*/
#define VALUE_SCLLOW_SDALOW					0x00
#define VALUE_SCLHIGH_SDALOW				0x01
#define VALUE_SCLLOW_SDAHIGH				0x02
#define VALUE_SCLHIGH_SDAHIGH				0x03

/*Data size in bits*/
#define DATA_SIZE_8BITS						0x07
#define DATA_SIZE_1BIT						0x00

/* The I2C master should actually drive the SDA line only when the output is LOW. It should
tristate the SDA line when the output should be high. This tristating the SDA line during high
output is supported only in FT232H chip. This feature is called DriveOnlyZero feature and is
enabled when the following bit is set in the options parameter in function I2C_Init */
#define I2C_ENABLE_DRIVE_ONLY_ZERO	0x0002



/******************************************************************************/
/*								Type defines								  */
/******************************************************************************/

/*The middle layer is designed to have no knowledge of the legacy protocol used(I2C/JTAG/SPI)
but it will have to perform certain operations differently for supporting the different protocols. For
example, the sequence to initilize the hardware for the different protocols will be different. This is
why we need to have a parameter from the caller(top layer) to specify for which protocol is the
service(say, hardware initialization) required. The following enumeration is hence used uniformly
as the first parameter across all the middle layer APIs */
typedef enum FT_LegacyProtocol_t{SPI, I2C, JTAG}FT_LegacyProtocol;



/******************************************************************************/
/*								External variables							  */
/******************************************************************************/





/******************************************************************************/
/*								Function declarations						  */
/******************************************************************************/




/******************************************************************************/


#endif	/*FTDI_COMMON_H*/

