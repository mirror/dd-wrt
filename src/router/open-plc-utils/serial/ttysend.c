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
 *   ttysend.c
 *
 *   Contributor(s):
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/putoptv.h"
#include "../tools/getoptv.h"
#include "../tools/number.h"
#include "../serial/serial.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../serial/baudrate.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define SERIAL_PORT "/dev/ttyS0"

void ttysend (int ifd, int ofd, size_t time, size_t chunk_size)

{
	char *buf;
	char *p;
	ssize_t r;
	ssize_t w;
	struct timeval tv_start,
	tv_now,
	tv_result;
	buf = malloc (chunk_size);
	if (buf == NULL)
	{
		error (1, errno, "could not allocate memory");
	}
	if (ifd == - 1)
	{
		unsigned i;
		for (i = 0; i < chunk_size; ++ i)
		{
			buf [i] = i % 256;
		}
	}
	if (gettimeofday (& tv_start, NULL) == - 1)
	{
		error (1, errno, "could not get time");
	}
	do
	{
		if (ifd == -1)
		{
			r = chunk_size;
		}
		else
		{
			r = read (ifd, buf, chunk_size);
			if (r == - 1)
			{
				error (1, errno, "could not read");
			}
			if (r == 0)
			{
				free (buf);
				return;
			}
		}
		p = buf;
		while (r)
		{
			w = write (ofd, p, r);
			if (w == - 1)
			{
				error (1, errno, "could not write");
			}
			p += w;
			r -= w;
		}
		if (gettimeofday (& tv_now, NULL) == - 1)
		{
			error (1, errno, "could not get time");
		}
		timersub (& tv_now, & tv_start, & tv_result);
	}
	while (tv_result.tv_sec < (signed)(time));
	free (buf);
}

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"f:l:s:t:qv",
		"",
		"Serial Line Rate Tester",
		"f f\tsend file (f)",
		"l f\tserial port is (f) [" SERIAL_PORT "]",
		"s n\tport speed [ 115200 ]",
		"t n\ttransmit for (n) seconds [ 15 ]",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};
	int fd;
	signed c;
	optind = 1;
	char const *line = SERIAL_PORT;
	struct _file_ file =
	{
		-1,
		NULL
	};
	speed_t speed = B115200;
	size_t time = 15;
	size_t chunk_size = 256;
	struct termios termios;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'f':
			file.name = optarg;
			if (!strcmp (file.name, "-")) file.file = STDIN_FILENO;
			else
			{
				file.file = open (file.name, O_BINARY | O_RDONLY);
				if (file.file == - 1)
				{
					error (1, errno, "could not open %s", file.name);
				}
			}
			break;
		case 'l':
			line = optarg;
			break;
		case 's':
			if (baudrate (uintspec (optarg, 0, UINT_MAX), & speed))
			{
				error (1, 0, "could not set baud rate");
			}
			break;
		case 't':
			time = uintspec (optarg, 0, SIZE_MAX);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	fd = open (line, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (fd == - 1)
	{
		error (1, errno, "could not open %s", line);
	}
	if (fcntl(fd, F_SETFL, 0) == -1)
	{
		error (1, errno, "failed to set tty flags");
	}
	if (tcgetattr (fd, & termios) == - 1)
	{
		error (1, errno, "could not get tty attributes");
	}
	cfmakeraw (& termios);
	termios.c_cflag = CS8 | CREAD | CLOCAL;
	if (cfsetspeed (& termios, speed) == - 1)
	{
		error (1, errno, "could not set tty speed");
	}
	if (tcsetattr (fd, TCSANOW, & termios) == - 1)
	{
		error (1, errno, "could not set tty attributes");
	}
	ttysend (file.file, fd, time, chunk_size);
	close (fd);
	return (0);
}

