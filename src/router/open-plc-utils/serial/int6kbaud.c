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
 *   int6kbaud.c - Atheros Serial Line Device Manager;
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if defined (WIN32)
#elif defined (__linux__)
#	include <termios.h>
#elif defined (__APPLE__)
#	include <termios.h>
#elif defined (__OpenBSD__)
#	include <termios.h>
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
#include "../tools/symbol.h"
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
#include "../tools/synonym.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../serial/openport.c"
#include "../serial/closeport.c"
#include "../serial/serial.c"
#endif

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

typedef struct uart

{
	struct _file_ port;
	char const * string;
	byte mode;
	uint64_t baudrate;
	byte databits;
	byte parity;
	byte stopbits;
	byte flowctrl;
	unsigned flags;
}

uart;

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

static const struct _term_ modes [] =

{
	{
		"command",
		"2"
	},
	{
		"transparent",
		"1"
	}
};

static const struct _term_ paritybits [] =

{
	{
		"even",
		"2"
	},
	{
		"none",
		"0"
	},
	{
		"odd",
		"1"
	}
};

static const struct _term_ flowctrls [] =

{
	{
		"none",
		"0"
	},
	{
		"off",
		"0"
	},
	{
		"on",
		"1"
	}
};


/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define MODES 		(sizeof (modes) / sizeof (struct _term_))
#define PARITYBITS 	(sizeof (paritybits) / sizeof (struct _term_))
#define FLOWCTRLS 	(sizeof (flowctrls) / sizeof (struct _term_))

/*====================================================================*
 *
 *   void at_command (struct uart * uart);
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
 *   void atbr (struct uart * uart);
 *
 *   set serial line parameters;
 *
 *--------------------------------------------------------------------*/

static void atbr (struct uart * uart)

{
	clearcommand ();
	insert ('A');
	insert ('T');
	insert ('B');
	insert ('R');
	decode (&uart->mode, sizeof (uart->mode));
	insert (',');
	uart->baudrate = HTOBE64 (uart->baudrate);
	decode (&uart->baudrate, sizeof (uart->baudrate));
	uart->baudrate = BE64TOH (uart->baudrate);
	insert (',');
	decode (&uart->databits, sizeof (uart->databits));
	insert (',');
	decode (&uart->parity, sizeof (uart->parity));
	insert (',');
	decode (&uart->stopbits, sizeof (uart->stopbits));
	insert (',');
	decode (&uart->flowctrl, sizeof (uart->flowctrl));
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
	if (_anyset (uart->flags, UART_ATBR))
	{
		atbr (uart);
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
		"B:c:D:F:m:p:P:q:S:uvw",
		"",
		"Atheros Serial Line Device Settings",
		"B n\tbaud rate is (n) [" LITERAL (UART_BAUDRATE) "]",
		"c s\tsend custom serial line command (s)",
		"D n\tuse (n) data bits [" LITERAL (UART_DATABITS) "]",
		"F n\tflow control is (n) ["LITERAL (UART_FLOWCTRL) "]",
		"m n\tcommand mode is (n)",
		"p f\tserial port is (f) [" DEVICE "]",
		"P n\tuse (n) parity bits [" LITERAL (UART_PARITY) "]",
		"q\tquiet mode",
		"S n\tuse (n) stop bits [" LITERAL (UART_STOPBITS) "]",
		"u\tforce default host port settings [115200 8N1]",
		"v\tverbose mode",
		"w\twake device [+++]",
		(char const *) (0)
	};
	struct uart uart =
	{
		{
			0,
			DEVICE
		},
		(char *)(0),
		UART_MODE,
		UART_BAUDRATE,
		UART_DATABITS,
		UART_PARITY,
		UART_STOPBITS,
		UART_FLOWCTRL,
		0
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
		case 'B':
			_setbits (uart.flags, UART_ATBR);
			uart.baudrate = (uint64_t)(uintspec (optarg, 1, ULONG_MAX));
			break;
		case 'c':
			_setbits (uart.flags, UART_COMMAND);
			uart.string = optarg;
			break;
		case 'D':
			_setbits (uart.flags, UART_ATBR);
			uart.databits = (byte)(uintspec (optarg, 7, 8));
			break;
		case 'F':
			_setbits (uart.flags, UART_ATBR);
			uart.flowctrl = (byte)(uintspec (synonym (optarg, flowctrls, FLOWCTRLS), 0, UCHAR_MAX));
			break;
		case 'm':
			_setbits (uart.flags, UART_ATBR);
			uart.mode = (byte)(uintspec (synonym (optarg, modes, MODES), 0, UCHAR_MAX));
		case 'P':
			_setbits (uart.flags, UART_ATBR);
			uart.parity = (byte)(uintspec (synonym (optarg, paritybits, PARITYBITS), 0, UCHAR_MAX));
			break;
		case 'q':
			_setbits (uart.flags, UART_SILENCE);
			break;
		case 'p':
			uart.port.name = optarg;
			break;
		case 'S':
			_setbits (uart.flags, UART_ATBR);
			uart.stopbits = (unsigned)(uintspec (optarg, 1, 2));
			break;
		case 'u':
			_setbits (uart.flags, UART_DEFAULT);
			break;
		case 'v':
			_setbits (uart.flags, UART_VERBOSE);
			break;
		case 'w':
			_setbits (uart.flags, UART_WAKE);
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
	return (0);
}

