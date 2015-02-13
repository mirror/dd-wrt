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
 *   int6kdetect.c
 *
 *
 *   Contributor(s):
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/flags.h"
#include "../tools/putoptv.h"
#include "../tools/getoptv.h"
#include "../serial/serial.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../serial/baudrate.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define SERIAL_PORT "/dev/ttyS0"

#define INT6KDETECT_QUIET    (1 << 0)
#define INT6KDETECT_CMD_MODE (1 << 1)

struct serial

{

#ifdef WIN32

	HANDLE h;

#else

	int fd;

#endif

};


#define UART_NOPARITY   0
#define UART_EVENPARITY 1
#define UART_ODDPARITY  2

struct serial_mode

{
	int baud_rate;
	int parity;
	int data_bits;
	int stop_bits;
};

ssize_t read_serial (struct serial *s, void *buf, size_t nbytes)

{

#ifdef WIN32

	DWORD read;
	BOOL r;
	r = ReadFile (s->h, buf, (DWORD) nbytes, &read, NULL);
	if (r) return read;
	else return -1;

#else

	return (read (s->fd, buf, nbytes));

#endif

}

ssize_t write_serial (struct serial *s, void *buf, size_t nbytes)

{

#ifdef WIN32

	DWORD written;
	BOOL r;
	r = WriteFile (s->h, buf, (DWORD) nbytes, &written, NULL);
	if (r) return written;
	else return -1;

#else

	return (write (s->fd, buf, nbytes));

#endif

}

int open_serial (char const *file, struct serial *s)

{

#ifdef WIN32

	s->h = CreateFile (file, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (s->h == INVALID_HANDLE_VALUE)
	{
		return (-1);
	}

#else

	if ((s->fd = open (file, O_RDWR)) == -1)
	{
		return (-1);
	}

#endif

	return (0);
}

int close_serial (struct serial *s)

{

#ifdef WIN32

	if (CloseHandle (s->h)) return 0;
	else return -1;

#else

	return (close (s->fd));

#endif

}

int set_serial (struct serial *s, struct serial_mode *serial_mode)

{

#ifdef WIN32

	COMMTIMEOUTS timeouts;
	DCB dcbSerial;
	memset (&dcbSerial, 0, sizeof (dcbSerial));
	dcbSerial.DCBlength = sizeof (dcbSerial);
	if (!GetCommState (s->h, &dcbSerial))
	{
		return (-1);
	}
	dcbSerial.BaudRate = serial_mode->baud_rate;
	dcbSerial.ByteSize = serial_mode->data_bits;
	switch (serial_mode->stop_bits)
	{
	case 1:
		dcbSerial.StopBits = ONESTOPBIT;
		break;
	case 2:
		dcbSerial.StopBits = TWOSTOPBITS;
		break;
	default:
		error (1, 0, "invalid stop bit setting");
	}
	switch (serial_mode->parity)
	{
	case UART_ODDPARITY:
		dcbSerial.Parity = ODDPARITY;
		dcbSerial.fParity = TRUE;
		break;
	case UART_EVENPARITY:
		dcbSerial.Parity = EVENPARITY;
		dcbSerial.fParity = TRUE;
		break;
	case UART_NOPARITY:
		dcbSerial.Parity = NOPARITY;
		dcbSerial.fParity = FALSE;
		break;
	default:
		error (1, 0, "invalid parity serial_mode");
	}
	if (!SetCommState (s->h, &dcbSerial))
	{
		error (0, 0, "could not set serial port settings");
		return (-1);
	}
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutConstant = 10;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 10;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts (s->h, &timeouts))
	{
		return (-1);
	}

#else

	struct termios termios;
	speed_t speed;
	tcgetattr (s->fd, &termios);
	cfmakeraw (&termios);
	termios.c_cflag &= ~CSIZE;
	switch (serial_mode->data_bits)
	{
	case 8:
		termios.c_cflag |= CS8;
		break;
	case 7:
		termios.c_cflag |= CS7;
		break;
	case 6:
		termios.c_cflag |= CS6;
		break;
	case 5:
		termios.c_cflag |= CS5;
		break;
	default:
		error (1, 0, "invalid serial byte size");
	}
	switch (serial_mode->stop_bits)
	{
	case 2:
		termios.c_cflag |= CSTOPB;
		break;
	case 1:
		termios.c_cflag &= ~CSTOPB;
		break;
	default:
		error (1, 0, "invalid number of stop bits");
	}
	switch (serial_mode->parity)
	{
	case UART_ODDPARITY:
		termios.c_cflag |= PARENB;
		termios.c_cflag |= PARODD;
		break;
	case UART_EVENPARITY:
		termios.c_cflag |= PARENB;
		termios.c_cflag &= ~PARODD;
		break;
	case UART_NOPARITY:
		termios.c_cflag &= ~PARENB;
		break;
	default:
		error (1, 0, "invalid parity serial_mode");
	}
	if (baudrate (serial_mode->baud_rate, &speed) == -1)
	{
		error (0, 0, "warning: unsupported baud rate: %d", serial_mode->baud_rate);
		return (-1);
	}
	if (cfsetspeed (&termios, speed) == -1) error (1, 0, "could not set serial baud rate");
	termios.c_cc [VTIME] = 1;
	termios.c_cc [VMIN] = 0;
	if (tcsetattr (s->fd, TCSANOW, &termios) == -1) error (1, 0, "could not set serial attributes");

#endif

	return (0);
}

int at_cmd (struct serial *s)

{
	char buf [32];
	ssize_t r;
	if (write_serial (s, "AT\r", 3) == -1) error (1, 0, "could not write");
	memset (buf, 0, sizeof (buf));
	r = read_serial (s, buf, sizeof (buf) - 1);
	if (r < 0) return -1;
	else if (r == 0) return -1;
	if (!strcmp (buf, "OK\r")) return 0;
	return (-1);
}

void wakeup (struct serial *s)

{
	sleep (1);
	if (write_serial (s, "+++", 3) == -1) error (1, 0, "could not write");
	sleep (1);
}

void dump_serial_mode (struct serial_mode *serial_mode)

{
	printf ("baud_rate = %d\n", serial_mode->baud_rate);
	printf ("stop_bits = %d\n", serial_mode->stop_bits);
	printf ("data_bits = %d\n", serial_mode->data_bits);
	printf ("parity    = %d\n", serial_mode->parity);
}

int try_serial_mode (struct serial *s, struct serial_mode *serial_mode, flag_t flags)

{
	if (set_serial (s, serial_mode) == -1)
	{
		error (0, 0, "could not set serial_mode");
		return (-1);
	}
	if (!_anyset (flags, INT6KDETECT_CMD_MODE)) wakeup (s);
	at_cmd (s);
	return (at_cmd (s));
}

int detect (struct serial *s, struct serial_mode *serial_mode, flag_t flags)

{
	static int rate [] =
	{
		115200,
		9600,
		460800,
		230400,
		57600,
		38400,
		19200,
		4800,
		2400,
		600,
		300,
		50
	};
	static int parity [] =
	{
		UART_NOPARITY,
		UART_EVENPARITY,
		UART_ODDPARITY
	};
	size_t i;
	size_t j;
	unsigned current;
	unsigned total;
	total = 2 * 2 * 3 * (sizeof (rate) / sizeof (int));
	current = 0;
	for (serial_mode->stop_bits = 1; serial_mode->stop_bits <= 2; ++serial_mode->stop_bits)
	{
		for (serial_mode->data_bits = 8; serial_mode->data_bits >= 7; --serial_mode->data_bits)
		{
			for (i = 0; i < sizeof (parity) / sizeof (int); ++i)
			{
				serial_mode->parity = parity [i];
				for (j = 0; j < sizeof (rate) / sizeof (int); ++j)
				{
					serial_mode->baud_rate = rate [j];
					++current;
					if (!_anyset (flags, INT6KDETECT_QUIET))
					{
						printf ("\rTesting mode: %03d/%03d (%.01f%%)...", current, total, current * 100.0 / total);
						fflush (stdout);
					}
					if (!try_serial_mode (s, serial_mode, flags))
					{
						if (!_anyset (flags, INT6KDETECT_QUIET)) printf ("\n");
						return (0);
					}
				}
			}
		}
	}
	if (!_anyset (flags, INT6KDETECT_QUIET)) printf ("\n");
	return (-1);
}

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"cl:qv",
		"",
		"Atheros UART Device Detector",
		"c\tassume device is in command mode",
		"l f\tserial port is (f) [" SERIAL_PORT "]",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};
	signed c;
	char const *line = SERIAL_PORT;
	struct serial serial;
	struct serial_mode serial_mode;
	flag_t flags = 0;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'c':
			_setbits (flags, INT6KDETECT_CMD_MODE);
			break;
		case 'l':
			line = optarg;
			break;
		case 'q':
			_setbits (flags, INT6KDETECT_QUIET);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (open_serial (line, &serial) == -1) error (1, errno, "could not open %s", line);
	if (detect (&serial, &serial_mode, flags) == -1) error (1, 0, "could not detect device");
	printf ("Detected the following serial mode:\n");
	dump_serial_mode (&serial_mode);
	close_serial (&serial);
	return (0);
}

