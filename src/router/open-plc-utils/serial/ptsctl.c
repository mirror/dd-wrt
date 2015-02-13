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
 *   ptsctl.c - PTS Module Controller;
 *
 *   Contributor(s):
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
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
#include "../tools/timer.h"
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

#define PTSCTL_DEBUG 0
#define PTSCTL_UNITS "CBA"
#define PTSCTL_LEDS 5
#define PTSCTL_BITS 7
#define PTSCTL_WAIT 50
#define PTSCTL_ECHO 0
#define PTSCTL_MODE 1

#define PTSCTL_LINE_ATTN 127
#define PTSCTL_GRND_ATTN 127

#define PTSCTL_BUFFER_SIZE 10
#define PTSCTL_STRING_SIZE 15

#ifdef WIN32
#	define PTSCTL_PORT "com1:"
#else
#	define PTSCTL_PORT "/dev/ttyS0"
#endif

#define PTSCTL_SILENCE (1 << 0)
#define PTSCTL_VERBOSE (1 << 1)
#define PTSCTL_CHANGE  (1 << 2)
#define PTSCTL_DISPLAY (1 << 3)
#define PTSCTL_ITERATE (1 << 4)

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

static const struct _term_ modes [] =

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

static char buffer [PTSCTL_BUFFER_SIZE];
static char string [PTSCTL_STRING_SIZE];
static signed length = 0;
static signed offset = 0;

/*====================================================================*
 *
 *   void cycle (char * string, unsigned offset, unsigned length);
 *
 *   rotate a number of consecutive characters starting at a given
 *   offset within a string; this is used to shift the character,
 *   that represents the power on/off bit, out of the way during
 *   data conversions from binary to ASCII and ASCII to binary;
 *
 *--------------------------------------------------------------------*/

static void cycle (char * string, unsigned offset, unsigned length)

{
	signed c = string [offset];
	memcpy (&string [offset], &string [offset + 1], length);
	string [offset + length] = c;
	return;
}


/*====================================================================*
 *
 *   void function1 (struct _file_ * port, char const * units, unsigned wait, unsigned echo);
 *
 *   send echo command to Weeder Solid State Relay modules in an order
 *   specified by units;
 *
 *--------------------------------------------------------------------*/

static void function1 (struct _file_ * port, char const * units, unsigned wait, unsigned echo)

{
	extern char buffer [PTSCTL_BUFFER_SIZE];
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
 *   void function2 (struct _file_ * port, char const * units, unsigned wait, unsigned data);
 *
 *   send write command to Weeder Solid State Relay modules in an
 *   order specified by units;
 *
 *--------------------------------------------------------------------*/

static void function2 (struct _file_ * port, char const * units, unsigned wait, unsigned data)

{
	extern char buffer [PTSCTL_BUFFER_SIZE];
	extern char string [PTSCTL_STRING_SIZE];
	extern signed length;
	extern signed offset;
	memset (string, 0, sizeof (string));
	memset (buffer, 0, sizeof (buffer));
	for (offset = 0; offset < (signed)(sizeof (string)); offset++)
	{
		string [offset] = '0' + (data & 1);
		data >>= 1;
	}
	cycle (string, 0, 5);
	for (offset = 0; *units; offset += PTSCTL_LEDS)
	{
		length = 0;
		buffer [length++] = *units++;
		buffer [length++] = 'W';
		memcpy (&buffer [length], &string [offset], PTSCTL_LEDS);
		length += PTSCTL_LEDS;
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
 *   void function3 (struct _file_ * port, char const * units, unsigned wait);
 *
 *   read weeder solid state modules and display settings on the
 *   console as attenuation;
 *
 *--------------------------------------------------------------------*/

static void function3 (struct _file_ * port, char const * units, unsigned wait)

{
	extern char buffer [PTSCTL_BUFFER_SIZE];
	extern char string [PTSCTL_STRING_SIZE];
	extern signed length;
	extern signed offset;
	signed value1 = 0;
	signed value2 = 0;
	memset (string, 0, sizeof (string));
	for (offset = 0; *units; offset += PTSCTL_LEDS)
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
		if (read (port->file, buffer, PTSCTL_LEDS + 2) == -1)
		{
			error (1, errno, FILE_CANTREAD, port->name);
		}
		memcpy (&string [offset], &buffer [1], PTSCTL_LEDS);
		SLEEP (wait);
	}
	cycle (string, PTSCTL_LEDS, 2);
	while (--offset > PTSCTL_BITS)
	{
		value1 <<= 1;
		value1 |= string [offset] - '0';
	}
	while (offset-- > 0)
	{
		value2 <<= 1;
		value2 |= string [offset] - '0';
	}
	if ((value1 >= 0) && (value2 >= 0))
	{
		printf ("%d %d\n", value1, value2);
	}
	return;
}


/*====================================================================*
 *
 *   void function4 (struct _file_ * port, char const * units, unsigned wait);
 *
 *   sequence through all attenuator settings at one second intervals;
 *   this function can be used to debug program additions and changes;
 *
 *--------------------------------------------------------------------*/

static void function4 (struct _file_ * port, char const * units, unsigned wait)

{
	signed value;
	for (value = 0; value < 128; value++)
	{
		function2 (port, units, wait, (value << 8) | (value << 1) | 1);
		function3 (port, units, wait);
		SLEEP (wait);
	}
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
		"f:g:n:p:iqrvw:z",
		"",
		"PTS Module Controller",
		"f f\tport is (f) [" PTSCTL_PORT "]",
		"g n\tline ground attenuation is (n) [" LITERAL (PTSCTL_GRND_ATTN) "]",
		"n n\tline neutral attenuation is (n) [" LITERAL (PTSCTL_LINE_ATTN) "]",
		"p n\tpower is (n) [" LITERAL (PTSCTL_MODE) "]",
		"q\tquiet mode",
		"r\tread and display attenuator settings",
		"v\tverbose mode",
		"w n\twait (n) millseconds [" LITERAL (PTSCTL_WAIT) "]",
		(char const *) (0)
	};
	struct _file_ port =
	{
		-1,
		PTSCTL_PORT
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

	char const * units = PTSCTL_UNITS;
	unsigned wait = PTSCTL_WAIT;
	unsigned mode = PTSCTL_MODE;
	unsigned echo = PTSCTL_ECHO;
	unsigned line = PTSCTL_LINE_ATTN;
	unsigned grnd = PTSCTL_GRND_ATTN;
	unsigned data = 0;
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	if (getenv ("PTSCTL"))
	{
		port.name = strdup (getenv ("PTSCTL"));
	}
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'f':
			port.name = optarg;
			break;
		case 'g':
			_setbits (flags, PTSCTL_CHANGE);
			grnd = (unsigned)(uintspec (optarg, 0, 0x7F));
			break;
		case 'n':
			_setbits (flags, PTSCTL_CHANGE);
			line = (unsigned)(uintspec (optarg, 0, 0x7F));
			break;
		case 'p':
			_setbits (flags, PTSCTL_CHANGE);
			mode = (unsigned)(uintspec (synonym (optarg, modes, SIZEOF (modes)), 0, 1));
			break;
		case 'w':
			wait = (unsigned)(uintspec (optarg, 5, 100));
			break;
		case 'q':
			_setbits (flags, PTSCTL_SILENCE);
			break;
		case 'r':
			_setbits (flags, PTSCTL_DISPLAY);
			break;
		case 'v':
			_setbits (flags, PTSCTL_VERBOSE);
			break;
		case 'z':
			_setbits (flags, PTSCTL_ITERATE);
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
		error (1, 0, FILE_CANTSAVE " state", port.name);
	}
	CloseHandle (hSerial);
	if ((port.file = open (port.name, O_BINARY | O_RDWR)) == -1)
	{
		error (1, errno, FILE_CANTOPEN, port.name);
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
	if (_anyset (flags, PTSCTL_CHANGE))
	{
		data = line << 8 | grnd << 1 | mode;
		function2 (&port, units, wait, data);
	}
	if (_anyset (flags, PTSCTL_DISPLAY))
	{
		function3 (&port, units, wait);
	}
	if (_anyset (flags, PTSCTL_ITERATE))
	{
		function4 (&port, units, wait);
	}
	close (port.file);
	return (0);
}

