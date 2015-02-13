/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   int6kuart.c - Atheros Serial Line Device Manager;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Mathieu Olivari <mathieu@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#if defined (WIN32)
#	include <net/ethernet.h>
#elif defined (__linux__)
#	include <net/ethernet.h>
#elif defined (__APPLE__)
#	include <net/ethernet.h>
#elif defined (__OpenBSD__)
#	include <sys/socket.h>
#	include <net/if.h>
#	include <net/if_arp.h>
#	include <netinet/in.h>
#	include <netinet/if_ether.h>
#else
#error "Unknown Environment"
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/number.h"
#include "../tools/memory.h"
#include "../tools/endian.h"
#include "../tools/files.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/types.h"
#include "../serial/serial.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/basespec.c"
#include "../tools/synonym.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#include "../tools/checksum32.c"
#include "../tools/hexencode.c"
#include "../tools/hexdump.c"
#include "../tools/hexstring.c"
#include "../tools/hexdecode.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../serial/openport.c"
#include "../serial/closeport.c"
#include "../serial/serial.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define FRAME_MIN_CHAR 120
#define FRAME_MAX_CHAR 1496

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

typedef struct uart

{
	struct _file_ port;
	struct _file_ pib;
	struct _file_ nvm;
	struct _file_ eth;
	char const * string;
	char PIBVersion [3];
	char IMGVersion [128];
	byte MACAddress [ETHER_ADDR_LEN];
	char NMKDigest [16];
	byte NMKNumber;
	byte module;
	uint16_t bfsize;
	uint16_t snooze;
	uint16_t timeout;
	unsigned flags;
}

uart;

/*====================================================================*
 *
 *   void at_writenvm (struct uart * uart);
 *
 *   read firmware image from file and send to device using command
 *   "ATWPF"; the file descriptor is "nvm" member of struct uart;
 *
 *--------------------------------------------------------------------*/

static void at_writenvm (struct uart * uart)

{
	extern struct command command;
	byte memory [UART_BLOCKSIZE];
	signed mblock = sizeof (memory);
	uint16_t mlength = 0;
	uint32_t moffset = 0;
	uint32_t mchksum;
	uint16_t olength = 0;
	uint32_t ooffset = 0;
	uint32_t ochksum;
	while ((mblock = read (uart->nvm.file, memory, mblock)) > 0)
	{
		clearcommand ();
		insert ('A');
		insert ('T');
		insert ('W');
		insert ('P');
		insert ('F');
		insert ('1');
		insert (',');
		mchksum = checksum32 (memory, (size_t)(mblock), 0);
		mlength = (uint16_t)(mblock);
		mlength = HTOBE16 (mlength);
		decode (&mlength, sizeof (mlength));
		mlength = BE16TOH (mlength);
		insert (',');
		moffset = HTOBE32 (moffset);
		decode (&moffset, sizeof (moffset));
		moffset = BE32TOH (moffset);
		insert (',');
		mchksum = HTOBE32 (mchksum);
		decode (&mchksum, sizeof (mchksum));
		mchksum = BE32TOH (mchksum);
		insert (',');
		decode (memory, mlength);
		insert ('\r');
		sendcommand (&uart->port, uart->flags);
		readcommand (&uart->port, uart->flags);
		mustbe ('O');
		mustbe ('K');
		mustbe ('1');
		mustbe (',');
		olength = (uint16_t)(hextoint (sizeof (olength)));
		if (olength != mlength)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected length %X", command.buffer, mlength);
		}
		mustbe (',');
		ooffset = (uint32_t)(hextoint (sizeof (ooffset)));
		if (ooffset != moffset)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected offset %X", command.buffer, moffset);
		}
		mustbe (',');
		ochksum = (uint32_t)(hextoint (sizeof (ochksum)));
		if (ochksum != mchksum)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected checksum %X (%X)", command.buffer, mchksum, ochksum);
		}
		mustbe (',');
		encode (memory, mblock);
		mustbe ('\r');
		moffset += mlength;
		if (_allclr (uart->flags, (UART_VERBOSE | UART_SILENCE)))
		{
			write (STDOUT_FILENO, ".", 1);
		}
	}

#ifndef WIN32

	if (_allclr (uart->flags, (UART_VERBOSE | UART_SILENCE)))
	{
		write (STDOUT_FILENO, "\n", 1);
	}

#endif

	return;
}


/*====================================================================*
 *
 *   void at_writepib (struct uart * uart);
 *
 *   read parameter block file and send to device using command
 *   "ATWPF"; the file descriptor is "pib" member of struct uart;
 *
 *--------------------------------------------------------------------*/

static void at_writepib (struct uart * uart)

{
	extern struct command command;
	byte memory [UART_BLOCKSIZE];
	signed mblock = sizeof (memory);
	uint16_t mlength = 0;
	uint16_t moffset = 0;
	uint32_t mchksum;
	uint16_t olength = 0;
	uint16_t ooffset = 0;
	uint32_t ochksum;
	while ((mblock = read (uart->pib.file, memory, mblock)) > 0)
	{
		clearcommand ();
		insert ('A');
		insert ('T');
		insert ('W');
		insert ('P');
		insert ('F');
		insert ('2');
		insert (',');
		mchksum = checksum32 (memory, (size_t)(mblock), 0);
		mlength = (uint16_t)(mblock);
		mlength = HTOBE16 (mlength);
		decode (&mlength, sizeof (mlength));
		mlength = BE16TOH (mlength);
		insert (',');
		moffset = HTOBE16 (moffset);
		decode (&moffset, sizeof (moffset));
		moffset = BE16TOH (moffset);
		insert (',');
		mchksum = HTOBE32 (mchksum);
		decode (&mchksum, sizeof (mchksum));
		mchksum = BE32TOH (mchksum);
		insert (',');
		decode (memory, mlength);
		insert ('\r');
		sendcommand (&uart->port, uart->flags);
		readcommand (&uart->port, uart->flags);
		mustbe ('O');
		mustbe ('K');
		mustbe ('2');
		mustbe (',');
		olength = (uint16_t)(hextoint (sizeof (olength)));
		if (olength != mlength)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected length %X", command.buffer, mlength);
		}
		mustbe (',');
		ooffset = (uint16_t)(hextoint (sizeof (ooffset)));
		if (ooffset != moffset)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected offset %X", command.buffer, moffset);
		}
		mustbe (',');
		ochksum = (uint32_t)(hextoint (sizeof (ochksum)));
		if (ochksum != mchksum)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected checksum %X (%X)", command.buffer, mchksum, ochksum);
		}
		mustbe (',');
		encode (memory, mblock);
		mustbe ('\r');
		moffset += mlength;
		if (_allclr (uart->flags, (UART_VERBOSE | UART_SILENCE)))
		{
			write (STDOUT_FILENO, ".", 1);
		}
	}

#ifndef WIN32

	if (_allclr (uart->flags, (UART_VERBOSE | UART_SILENCE)))
	{
		write (STDOUT_FILENO, "\n", 1);
	}

#endif

	return;
}


/*====================================================================*
 *
 *   void at_readpib (struct uart * uart);
 *
 *   read parameter block from device and save to file using command
 *   "ATRP"; the file descriptor is "pib" member of struct uart;
 *
 *--------------------------------------------------------------------*/

static void at_readpib (struct uart * uart)

{
	extern struct command command;
	byte memory [UART_BLOCKSIZE];
	signed mblock = sizeof (memory);
	uint16_t mextent = 0;
	uint16_t mlength = 0;
	uint16_t moffset = 0;
	uint16_t olength = 0;
	uint16_t ooffset = 0;
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('R');
	insert ('P');
	insert ('2');
	insert (',');
	insert ('4');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('2');
	mustbe (',');
	mustbe ('4');
	mustbe (',');
	encode (&mextent, sizeof (mextent));
	mextent = LE16TOH (mextent);
	mustbe ('\r');
	while (mextent)
	{
		clearcommand ();
		insert ('A');
		insert ('T');
		insert ('R');
		insert ('P');
		if (mblock > mextent)
		{
			mblock = mextent;
		}
		mlength = (uint16_t)(mblock);
		mlength = HTOBE16 (mlength);
		decode (&mlength, sizeof (mlength));
		mlength = BE16TOH (mlength);
		insert (',');
		moffset = HTOBE16 (moffset);
		decode (&moffset, sizeof (moffset));
		moffset = BE16TOH (moffset);
		insert ('\r');
		sendcommand (&uart->port, uart->flags);
		readcommand (&uart->port, uart->flags);
		mustbe ('O');
		mustbe ('K');
		olength = (uint16_t)(hextoint (sizeof (olength)));
		if (olength != mlength)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: have %d bytes but wanted %d", command.buffer, olength, mlength);
		}
		mustbe (',');
		ooffset = (uint16_t)(hextoint (sizeof (ooffset)));
		if (ooffset != moffset)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, EINVAL, "[%s]: expected offset %X", command.buffer, moffset);
		}
		mustbe (',');
		encode (memory, mblock);
		if (write (uart->pib.file, memory, mblock) < mblock)
		{
			command.buffer [command.offset] = (char)(0);
			error (1, errno, "[%s]: expected length %d", command.buffer, mblock);
		}
		mustbe ('\r');
		moffset += mblock;
		mextent -= mblock;
		if (_allclr (uart->flags, (UART_VERBOSE | UART_SILENCE)))
		{
			write (STDOUT_FILENO, ".", 1);
		}
	}

#ifndef WIN32

	if (_allclr (uart->flags, (UART_VERBOSE | UART_SILENCE)))
	{
		write (STDOUT_FILENO, "\n", 1);
	}

#endif

	return;
}


/*====================================================================*
 *
 *   void at_wake (struct uart * uart);
 *
 *   send wake command "+++" to enter command mode;
 *
 *--------------------------------------------------------------------*/

static void at_wake (struct uart * uart)

{
	clearcommand ();
	insert ('+');
	insert ('+');
	insert ('+');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void at_command (struct uart * uart);
 *
 *   send custom command; use this function to send any serial line
 *   command that may not be supported by this program;
 *
 *--------------------------------------------------------------------*/

static void at_command (struct uart * uart)

{
	clearcommand ();
	while (*uart->string)
	{
		insert (*uart->string++);
	}
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	clearcommand ();
	return;
}


/*====================================================================*
 *
 *   void at_respond (struct uart * uart);
 *
 *   send command "AT" to test command mode; this command does nothing
 *   but echo "OK";
 *
 *--------------------------------------------------------------------*/

static void at_respond (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void atz (struct uart * uart);
 *
 *   send command "ATZ" to reset the device; no response is expected;
 *
 *--------------------------------------------------------------------*/

static void atz (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('Z');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	return;
}


/*====================================================================*
 *
 *   void atrv (struct uart * uart);
 *
 *   read and display the firmware image version using command "ATRV";
 *   return the version string in IMGVersion member of struct uart;
 *
 *--------------------------------------------------------------------*/

static void atrv (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('R');
	insert ('V');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\"');
	string (uart->IMGVersion);
	mustbe ('\"');
	mustbe ('\r');
	printf ("%s\n", uart->IMGVersion);
	return;
}


/*====================================================================*
 *
 *   void atrpm (struct uart * uart);
 *
 *   read and display the PIB version and MAC address using command
 *   "ATRPM"; return version string in PIBVersion member and address
 *   string in MACAddress member of struct
 *
 *
 *--------------------------------------------------------------------*/

static void atrpm (struct uart * uart)

{
	char mac [ETHER_ADDR_LEN * 3];
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('R');
	insert ('P');
	insert ('M');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\"');
	string (uart->PIBVersion);
	mustbe ('\"');
	mustbe (',');
	encode (uart->MACAddress, sizeof (uart->MACAddress));
	mustbe ('\r');
	printf ("%s %s\n", uart->PIBVersion, hexstring (mac, sizeof (mac), uart->MACAddress, sizeof (uart->MACAddress)));
	return;
}


/*====================================================================*
 *
 *   void atsk1 (struct uart * uart);
 *
 *   send Set Key command "ATSK"; ask device for NMK; encode returned
 *   key into uart-NMK;
 *
 *--------------------------------------------------------------------*/

static void atsk1 (struct uart * uart)

{
	char key [48];
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('S');
	insert ('K');
	insert ('?');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	encode (uart->NMKDigest, sizeof (uart->NMKDigest));
	mustbe ('\r');
	printf ("%s\n", hexstring (key, sizeof (key), uart->NMKDigest, sizeof (uart->NMKDigest)));
	return;
}


/*====================================================================*
 *
 *   void atsk2 (struct uart * uart);
 *
 *   send Set Key command "ATSK"; send device the NMK; encode returned
 *
 *--------------------------------------------------------------------*/

static void atsk2 (struct uart * uart)

{
	char key [48];
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('S');
	insert ('K');
	decode (uart->NMKDigest, sizeof (uart->NMKDigest));
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	encode (uart->NMKDigest, sizeof (uart->NMKDigest));
	mustbe ('\r');
	printf ("%s\n", hexstring (key, sizeof (key), uart->NMKDigest, sizeof (uart->NMKDigest)));
	return;
}


/*====================================================================*
 *
 *   void atdst1 (struct uart * uart);
 *
 *   read transparent mode destination MAC address command "ATDST?";
 *
 *--------------------------------------------------------------------*/

static void atdst1 (struct uart * uart)

{
	char mac [ETHER_ADDR_LEN * 3];
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('D');
	insert ('S');
	insert ('T');
	insert ('?');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	encode (uart->MACAddress, sizeof (uart->MACAddress));
	mustbe ('\r');
	printf ("%s\n", hexstring (mac, sizeof (mac), uart->MACAddress, sizeof (uart->MACAddress)));
	return;
}


/*====================================================================*
 *
 *   void atdst2 (struct uart * uart);
 *
 *   read transparent mode destination MAC address command "ATDST?";
 *
 *--------------------------------------------------------------------*/

static void atdst2 (struct uart * uart)

{
	char mac [ETHER_ADDR_LEN * 3];
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('D');
	insert ('S');
	insert ('T');
	decode (uart->MACAddress, sizeof (uart->MACAddress));
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	encode (uart->MACAddress, sizeof (uart->MACAddress));
	mustbe ('\r');
	printf ("%s\n", hexstring (mac, sizeof (mac), uart->MACAddress, sizeof (uart->MACAddress)));
	return;
}


/*====================================================================*
 *
 *   void atni (struct uart * uart);
 *
 *   reset device to factory default pib command "ATNI";
 *
 *--------------------------------------------------------------------*/

static void atni (struct uart * uart)

{
	unsigned count;
	unsigned index;
	uint16_t rxrate;
	uint16_t txrate;
	byte address [ETHER_ADDR_LEN];
	char mac [ETHER_ADDR_LEN * 3];
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('N');
	insert ('I');
	insert ('?');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	count = (unsigned)(hextoint (sizeof (unsigned)));
	while (count--)
	{
		mustbe (',');
		index = (unsigned)(hextoint (sizeof (index)));
		mustbe (',');
		encode (address, sizeof (address));
		mustbe (',');
		txrate = (uint16_t)(hextoint (sizeof (rxrate)));
		mustbe (',');
		rxrate = (uint16_t)(hextoint (sizeof (txrate)));
		printf ("%d %s %3d RX %3d TX\n", index, hexstring (mac, sizeof (mac), address, sizeof (address)), rxrate, txrate);
	}
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void atfd (struct uart * uart);
 *
 *   reset device to factory default pib command "ATFD";
 *
 *--------------------------------------------------------------------*/

static void atfd (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('F');
	insert ('D');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void atps (struct uart * uart);
 *
 *   etner power save mode command "ATPS";
 *
 *--------------------------------------------------------------------*/

static void atps (struct uart * uart)

{
	extern struct command command;
	uint16_t result;
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('P');
	insert ('S');
	uart->snooze = HTOBE16 (uart->snooze);
	decode (&uart->snooze, sizeof (uart->snooze));
	uart->snooze = BE16TOH (uart->snooze);
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	result = (uint16_t)(hextoint (sizeof (result)));
	if (result != uart->snooze)
	{
		error (1, EINVAL, "[%s]: expected timeout %04X", command.buffer, uart->snooze);
	}
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void ato (struct uart * uart);
 *
 *   exit command mode and enter tyransparent mode command "ATO";
 *
 *--------------------------------------------------------------------*/

static void ato (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('O');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void athsc (struct uart * uart);
 *
 *   exit command mode; enter high speed command mode "ATHSC";
 *
 *--------------------------------------------------------------------*/

static void athsc (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('H');
	insert ('S');
	insert ('C');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void atwnv (struct uart * uart);
 *
 *   write PIB and/or IMG to NVM "ATWNVx";
 *
 *--------------------------------------------------------------------*/

static void atwnv (struct uart * uart)

{
	extern struct command command;
	byte result;
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('W');
	insert ('N');
	insert ('V');
	decode (&uart->module, sizeof (uart->module));
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	result = (byte)(hextoint (sizeof (result)));
	if (result != uart->module)
	{
		error (1, EINVAL, "[%s]: expected module %d", command.buffer, uart->module);
	}
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void atbsz1 (struct uart * uart);
 *
 *   get transparent mode buffer size "ATBSZ?";
 *
 *--------------------------------------------------------------------*/

static void atbsz1 (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('B');
	insert ('S');
	insert ('Z');
	insert ('?');
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	uart->bfsize = (uint16_t)(hextoint (sizeof (uart->bfsize)));
	mustbe ('\r');
	printf ("%d\n", uart->bfsize);
	return;
}


/*====================================================================*
 *
 *   void atbsz2 (struct uart * uart);
 *
 *   set transparent mode buffer size "ATBSZn";
 *
 *--------------------------------------------------------------------*/

static void atbsz2 (struct uart * uart)

{
	extern struct command command;
	uint16_t result;
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('B');
	insert ('S');
	insert ('Z');
	uart->bfsize = HTOBE16 (uart->bfsize);
	decode (&uart->bfsize, sizeof (uart->bfsize));
	uart->bfsize = BE16TOH (uart->bfsize);
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	result = (uint16_t)(hextoint (sizeof (result)));
	if (result != uart->bfsize)
	{
		error (1, EINVAL, "[%s]: expected buffer size %04X", command.buffer, uart->bfsize);
	}
	mustbe ('\r');
	printf ("%d\n", uart->bfsize);
	return;
}


/*====================================================================*
 *
 *   void atto (struct uart * uart);
 *
 *   set transparent mode buffer timeout "ATTO";
 *
 *--------------------------------------------------------------------*/

static void atto (struct uart * uart)

{
	extern struct command command;
	uint16_t result;
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('T');
	insert ('O');
	uart->timeout = HTOBE16 (uart->timeout);
	decode (&uart->timeout, sizeof (uart->timeout));
	uart->timeout = BE16TOH (uart->timeout);
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	mustbe ('O');
	mustbe ('K');
	result = (uint16_t)(hextoint (sizeof (result)));
	if (result != uart->timeout)
	{
		error (1, EINVAL, "[%s]: expected timeout %04X", command.buffer, uart->timeout);
	}
	mustbe ('\r');
	return;
}


/*====================================================================*
 *
 *   void atm (struct uart * uart);
 *
 *
 *--------------------------------------------------------------------*/

static void atm (struct uart * uart)

{
	extern struct command command;
	uint8_t buffer [ETHER_MAX_LEN + ETHER_MAX_LEN + 512];
	unsigned length = (unsigned)(readframe (uart->eth.file, buffer, sizeof (buffer)));
	if (length < FRAME_MIN_CHAR)
	{
		error (1, ENOTSUP, "Frame specification of %d bytes less than %d minimum", (length >> 1), (FRAME_MIN_CHAR >> 1));
	}
	if (length > FRAME_MAX_CHAR)
	{
		error (1, ENOTSUP, "Frame specification of %d bytes more than %d maximum", (length >> 1), (FRAME_MAX_CHAR >> 1));
	}
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('M');
	memcpy (command.buffer + command.length, buffer, length);
	command.length += (signed)(length);
	insert ('\r');
	sendcommand (&uart->port, uart->flags);
	readcommand (&uart->port, uart->flags);
	write (STDOUT_FILENO, command.buffer, command.length);
	write (STDOUT_FILENO, "\n", sizeof (char));
	return;
}


/*====================================================================*
 *
 *   void manager (struct uart * uart);
 *
 *   examine flagword in struct uart and perform requested operations
 *   in the order that bits are tested; the order that bits are tested
 *   may be changed as needed;
 *
 *--------------------------------------------------------------------*/

static void manager (struct uart * uart)

{
	if (_anyset (uart->flags, UART_WAKE))
	{
		at_wake (uart);
	}
	if (_anyset (uart->flags, UART_COMMAND))
	{
		at_command (uart);
	}
	if (_anyset (uart->flags, UART_RESPOND))
	{
		at_respond (uart);
	}
	if (_anyset (uart->flags, UART_ATRV))
	{
		atrv (uart);
	}
	if (_anyset (uart->flags, UART_ATRPM))
	{
		atrpm (uart);
	}
	if (_anyset (uart->flags, UART_ATDST1))
	{
		atdst1 (uart);
	}
	if (_anyset (uart->flags, UART_ATDST2))
	{
		atdst2 (uart);
	}
	if (_anyset (uart->flags, UART_ATZ))
	{
		atz (uart);
	}
	if (_anyset (uart->flags, UART_ATFD))
	{
		atfd (uart);
	}
	if (_anyset (uart->flags, UART_ATPS))
	{
		atps (uart);
	}
	if (_anyset (uart->flags, UART_ATO))
	{
		ato (uart);
	}
	if (_anyset (uart->flags, UART_ATNI))
	{
		atni (uart);
	}
	if (_anyset (uart->flags, UART_ATHSC))
	{
		athsc (uart);
	}
	if (_anyset (uart->flags, UART_ATSK1))
	{
		atsk1 (uart);
	}
	if (_anyset (uart->flags, UART_ATSK2))
	{
		atsk2 (uart);
	}
	if (_anyset (uart->flags, UART_ATRP))
	{
		at_readpib (uart);
	}
	if (_anyset (uart->flags, UART_ATWPF1))
	{
		at_writenvm (uart);
	}
	if (_anyset (uart->flags, UART_ATWPF2))
	{
		at_writepib (uart);
	}
	if (_anyset (uart->flags, UART_ATWNV))
	{
		atwnv (uart);
	}
	if (_anyset (uart->flags, UART_ATBSZ1))
	{
		atbsz1 (uart);
	}
	if (_anyset (uart->flags, UART_ATBSZ2))
	{
		atbsz2 (uart);
	}
	if (_anyset (uart->flags, UART_ATM))
	{
		atm (uart);
	}
	if (_anyset (uart->flags, UART_ATTO))
	{
		atto (uart);
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"bc:C:dD:F:HiImM:n:N:Op:P:qrRS:s:tTvwW:zZ:",
		"",
		"Atheros Serial Line Device Manager",
		"b\tset default host baud rate",
		"c s\tsend custom serial line command (s)",
		"C x\tcommit module (x) to NVM [ATWNVx]",
		"d\tget destination mac address [ATDST?]",
		"D x\tset destination mac address [ATDSTx]",
		"F f\tframe file is (s)",
		"H\tplace device in High Speed Command Mode [ATHSC]",
		"i\tget network information [ATNI]",
		"I\tget PIB version and MAC address [ATRPM]",
		"m\tget network membership key [ATSK?]",
		"M x\tset network membership key [ATSKx]",
		"N f\twrite NVM file (f) to SDRAM [ATWFP1]",
		"O\tplace device in Transparent Mode [ATO]",
		"p f\tread PIB from SDRAM to file (f) [ATRP]",
		"P f\twrite PIB file (f) to SDRAM [ATWFP2]",
		"q\tplace program in quiet mode",
		"r\tget parameter/firmware revision [ATRV]",
		"R\treset device [ATZ]",
		"s f\tserial port is (f) [" DEVICE "]",
		"S n\tenter power save mode for (n) seconds [ATPS]",
		"t\ttest device [AT]",
		"T\treset to factory defaults [ATFD]",
		"v\tplace program verbose mode",
		"w\tplace device in Command Mode [+++]",
		"W x\tset Transparent Mode aggregation timeout [ATTO]",
		"z\tget Transparent Mode buffer size [ATBSZ?]",
		"Z n\tset Transparent Mode buffer size [ATBSZn]",
		(char const *) (0)
	};
	struct uart uart =
	{
		{
			0,
			DEVICE
		},
		{
			-1,
			"nvmfile"
		},
		{
			-1,
			"pibfile"
		},
		{
			-1,
			"ethfile"
		},
		(char *)(0),
		{
			0
		},
		{
			0
		},
		{
			0
		},
		{
			0
		},
		(uint8_t)(0),
		(uint8_t)(0),
		(uint16_t)(0),
		(uint16_t)(0),
		(uint16_t)(0),
		(unsigned)(0)
	};
	signed c;
	if (getenv (UART_PORT))
	{
		uart.port.name = strdup (getenv (UART_PORT));
	}
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'b':
			_setbits (uart.flags, UART_DEFAULT);
			break;
		case 'c':
			_setbits (uart.flags, UART_COMMAND);
			uart.string = optarg;
			break;
		case 'C':
			_setbits (uart.flags, UART_ATWNV);
			uart.module = (byte)(basespec (optarg, 16, sizeof (uart.module)));
			break;
		case 'd':
			_setbits (uart.flags, UART_ATDST1);
			break;
		case 'D':
			_setbits (uart.flags, UART_ATDST2);
			if (!hexencode (uart.MACAddress, sizeof (uart.MACAddress), optarg))
			{
				error (1, errno, PLC_BAD_MAC, optarg);
			}
			break;
		case 'F':
			if ((uart.eth.file = open (uart.eth.name = optarg, O_BINARY | O_RDONLY)) == -1)
			{
				error (1, errno, "%s", uart.eth.name);
			}
			_setbits (uart.flags, UART_ATM);
			break;
		case 'H':
			_setbits (uart.flags, UART_ATHSC);
			break;
		case 'i':
			_setbits (uart.flags, UART_ATNI);
			break;
		case 'I':
			_setbits (uart.flags, UART_ATRPM);
			break;
		case 'm':
			_setbits (uart.flags, UART_ATSK1);
			break;
		case 'M':
			_setbits (uart.flags, UART_ATSK2);
			if (!hexencode (uart.NMKDigest, sizeof (uart.NMKDigest), optarg))
			{
				error (1, errno, PLC_BAD_NMK, optarg);
			}
			break;
		case 'N':
			if ((uart.nvm.file = open (uart.nvm.name = optarg, O_BINARY | O_RDONLY)) == -1)
			{
				error (1, errno, "%s", uart.nvm.name);
			}
			_setbits (uart.flags, UART_ATWPF1);
			break;
		case 'O':
			_setbits (uart.flags, UART_ATO);
			break;
		case 'P':
			if ((uart.pib.file = open (uart.pib.name = optarg, O_BINARY | O_RDONLY)) == -1)
			{
				error (1, errno, "%s", uart.pib.name);
			}
			_setbits (uart.flags, UART_ATWPF2);
			break;
		case 'p':
			if ((uart.pib.file = open (uart.pib.name = optarg, O_BINARY|O_CREAT|O_RDWR|O_TRUNC, FILE_FILEMODE)) == -1)
			{
				error (1, errno, "%s", uart.pib.name);
			}

#ifndef WIN32

			chown (optarg, getuid (), getgid ());

#endif

			_setbits (uart.flags, UART_ATRP);
			break;
		case 'q':
			_setbits (uart.flags, UART_SILENCE);
			break;
		case 'r':
			_setbits (uart.flags, UART_ATRV);
			break;
		case 'R':
			_setbits (uart.flags, UART_ATZ);
			break;
		case 'S':
			_setbits (uart.flags, UART_ATPS);
			uart.snooze = (uint16_t)(uintspec (optarg, 1, 900));
			break;
		case 's':
			uart.port.name = optarg;
			break;
		case 'T':
			_setbits (uart.flags, UART_ATFD);
			break;
		case 't':
			_setbits (uart.flags, UART_RESPOND);
			break;
		case 'v':
			_setbits (uart.flags, UART_VERBOSE);
			break;
		case 'w':
			_setbits (uart.flags, UART_WAKE);
			break;
		case 'W':
			_setbits (uart.flags, UART_ATTO);
			uart.timeout = (unsigned)(uintspec (optarg, 1, 2000));
			break;
		case 'z':
			_setbits (uart.flags, UART_ATBSZ1);
			break;
		case 'Z':
			_setbits (uart.flags, UART_ATBSZ2);
			uart.bfsize = (uint16_t)(uintspec (optarg, 1, 1500));
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc)
	{
		error (1, ENOTSUP, ERROR_TOOMANY);
	}
	openport (&uart.port, uart.flags);
	manager (&uart);
	closeport (&uart.port);
	exit (0);
}

