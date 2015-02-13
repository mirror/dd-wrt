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
 *   weeder.c - Weeder Solid State Relay Module Controller;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Mathieu Olivari <mathieu@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined (__linux__)
#	include <termios.h>
#elif defined (__APPLE__)
#	include <termios.h>
#elif defined (__OpenBSD__)
#	include <termios.h>
#elif defined (WIN32)
#	include <windows.h>
#else
#error "Unknown Environment"
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/timer.h"
#include "../tools/files.h"
#include "../tools/flags.h"
#include "../tools/error.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/synonym.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define WEEDER_UNITS "BA"
#define WEEDER_LEDS 5
#define WEEDER_BITS 7
#define WEEDER_WAIT 25
#define WEEDER_ECHO 0
#define WEEDER_MODE 1

#define WEEDER_BUFFER_LENGTH 10
#define WEEDER_STRING_LENGTH 15

#ifdef WIN32
#	define WEEDER_PORT "com1:"
#else
#	define WEEDER_PORT "/dev/ttyS0"
#endif

#define WEEDER_SILENCE (1 << 0)
#define WEEDER_VERBOSE (1 << 1)
#define WEEDER_DISPLAY (1 << 2)
#define WEEDER_NEWLINE (1 << 2)

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

static struct _term_ const modes [] =

{
	{
		"off",
		"0"
	},
	{
		"on",
		"1"
	}
};

static char buffer [WEEDER_BUFFER_LENGTH];
static char string [WEEDER_STRING_LENGTH];
static signed length = 0;
static signed offset = 0;

/*====================================================================*
 *
 *   void function1 (struct _file_ * port, char const * units, unsigned wait, unsigned echo);
 *
 *   send echo command to Weeder Solid State Relay modules B then A;
 *   Standard Atheros relay modules were wired in reverse order for
 *   some reason;
 *
 *--------------------------------------------------------------------*/

static void function1 (struct _file_ * port, char const * units, unsigned wait, unsigned echo)

{
	extern char buffer [];
	extern signed length;
	while (*units)
	{
		length = 0;
		buffer [length++] = *units++;
		buffer [length++] = 'X';
		buffer [length++] = '0' + (echo & 1);
		buffer [length++] = '\r';
		if (write (port->file, buffer, length) != length)
		{
			error (1, errno, FILE_CANTSAVE, port->name);
		}
		SLEEP (wait);
	}
	return;
}


/*====================================================================*
 *
 *   void function2 (struct _file_ * port, char const * units, unsigned wait, unsigned mode, unsigned data);
 *
 *   send write command to Weeder Solid State Relay modules B then A
 *   because Qualcomm Atheros relay modules are wired in reverse order
 *   for some reason;
 *
 *--------------------------------------------------------------------*/

static void function2 (struct _file_ * port, char const * units, unsigned wait, unsigned mode, unsigned data)

{
	extern char buffer [WEEDER_BUFFER_LENGTH];
	extern signed length;
	length = 0;
	buffer [length++] = *units++;
	buffer [length++] = 'W';
	buffer [length++] = '0' + (mode & 1);
	buffer [length++] = '0';
	buffer [length++] = '0';
	while (length < WEEDER_BITS)
	{
		buffer [length++] = '0' + (data & 1);
		data >>= 1;
	}
	buffer [length++] = '\r';
	if (write (port->file, buffer, length) != length)
	{
		error (1, errno, FILE_CANTSAVE, port->name);
	}
	SLEEP (wait);
	length = 0;
	buffer [length++] = *units++;
	buffer [length++] = 'W';
	while (length < WEEDER_BITS)
	{
		buffer [length++] = '0' + (data & 1);
		data >>= 1;
	}
	buffer [length++] = '\r';
	if (write (port->file, buffer, length) != length)
	{
		error (1, errno, FILE_CANTSAVE, port->name);
	}
	SLEEP (wait);
	return;
}


/*====================================================================*
 *
 *   void function3 (struct _file_ * port, char const * units, unsigned wait);
 *
 *   read weeder solid state controller and display settings on the
 *   console as attenuation;
 *
 *--------------------------------------------------------------------*/

static void function3 (struct _file_ * port, char const * units, unsigned wait)

{
	extern char buffer [WEEDER_BUFFER_LENGTH];
	extern char string [WEEDER_STRING_LENGTH];
	extern signed length;
	extern signed offset;
	unsigned number = 0;
	memset (string, 0, sizeof (string));
	for (offset = 0; *units; offset += WEEDER_LEDS)
	{
		length = 0;
		buffer [length++] = *units++;
		buffer [length++] = 'R';
		buffer [length++] = '\r';
		if (write (port->file, buffer, length) != length)
		{
			error (1, errno, FILE_CANTSAVE, port->name);
		}
		SLEEP (wait);
		memset (buffer, 0, sizeof (buffer));
		if (read (port->file, buffer, WEEDER_LEDS + 2) == -1)
		{
			error (1, errno, FILE_CANTREAD, port->name);
		}
		SLEEP (wait);
		memcpy (&string [offset], &buffer [1], WEEDER_LEDS);
	}
	while (offset-- > 3)
	{
		number <<= 1;
		number |= string [offset] - '0';
	}
	printf ("%d\n", number);
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"e:m:o:p:iqrvw:",
		"",
		"Weeder Solid State Relay Module Controller",
		"e n\techo is (n) [" LITERAL (WEEDER_ECHO) "]",
		"m n\tmode is (n) [" LITERAL (WEEDER_MODE) "]",
		"o s\tunit order is (s) [" WEEDER_UNITS "]",
		"p f\tport is (f) [" WEEDER_PORT "]",
		"q\tquiet mode",
		"r\tread attenuator value",
		"v\tverbose mode",
		"w n\twait (n) millseconds [" LITERAL (WEEDER_WAIT) "]",
		(char const *) (0)
	};
	struct _file_ port =
	{
		-1,
		WEEDER_PORT
	};

#if defined (WIN32)

	HANDLE hSerial;
	DCB dcbSerial =
	{
		0
	};

#else

	struct termios termios;

#endif

	char const * units = WEEDER_UNITS;
	unsigned wait = WEEDER_WAIT;
	unsigned echo = WEEDER_ECHO;
	unsigned mode = WEEDER_MODE;
	unsigned data = 0;
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	if (getenv ("WEEDER"))
	{
		port.name = strdup (getenv ("WEEDER"));
	}
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'e':
			echo = (unsigned)(uintspec (synonym (optarg, modes, SIZEOF (modes)), 0, 1));
			break;
		case 'm':
			mode = (unsigned)(uintspec (synonym (optarg, modes, SIZEOF (modes)), 0, 1));
			break;
		case 'n':
			_setbits (flags, WEEDER_NEWLINE);
			break;
		case 'o':
			units = optarg;
			break;
		case 'p':
			port.name = optarg;
			break;
		case 'w':
			wait = (unsigned)(uintspec (optarg, 5, 100));
			break;
		case 'q':
			_setbits (flags, WEEDER_SILENCE);
			break;
		case 'r':
			_setbits (flags, WEEDER_DISPLAY);
			break;
		case 'v':
			_setbits (flags, WEEDER_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if ((argc) && (* argv))
	{
		data = (unsigned)(uintspec (* argv, 0, 0x7F));
	}

#if defined (WIN32)

	hSerial = CreateFile (port.name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		error (1, errno, FILE_CANTOPEN, port.name);
	}
	dcbSerial.DCBlength = sizeof (dcbSerial);
	if (!GetCommState (hSerial, &dcbSerial))
	{
		error (1, 0, FILE_CANTREAD " state", port.name);
	}
	dcbSerial.BaudRate = CBR_9600;
	dcbSerial.ByteSize = 8;
	dcbSerial.StopBits = ONESTOPBIT;
	dcbSerial.Parity = NOPARITY;
	if (!SetCommState (hSerial, &dcbSerial))
	{
		error (1, 0, FILE_CANTSAVE, port.name);
	}
	CloseHandle (hSerial);
	if ((port.file = open (port.name, O_BINARY | O_RDWR)) == -1)
	{
		error (1, errno, FILE_CANTOPEN " state", port.name);
	}

#else

	if ((port.file = open (port.name, O_RDWR|O_NOCTTY|O_NDELAY)) == -1)
	{
		error (1, 0, FILE_CANTOPEN, port.name);
	}
	tcgetattr (port.file, &termios);
	termios.c_cflag = CS8;
	cfsetospeed (&termios, B9600);
	tcsetattr (port.file, TCSANOW, &termios);

#endif

	function1 (&port, units, wait, echo);
	if ((argc) && (* argv))
	{
		function2 (&port, units, wait, mode, data);
	}
	if (_anyset (flags, WEEDER_DISPLAY))
	{
		function3 (&port, units, wait);
	}
	close (port.file);
	exit (0);
}

