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
 *   ttycat.c - serial port test program;
 *
 *   write one or more files to a serial device;
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../serial/serial.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/efreopen.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#endif

/*====================================================================*
 *   program warnings;
 *--------------------------------------------------------------------*/

#if defined (WIN32)
#error "This program does not support Windows platforms"
#endif

/*====================================================================*
 *
 *   signed copy (signed ifd, signed ofd, void * memory, signed extent)
 *
 *   copy ifd to ofd using a buffer of specified size;
 *
 *--------------------------------------------------------------------*/

static signed copy (signed ifd, signed ofd, void * memory, signed extent)

{
	while ((extent = read (ifd, memory, extent)) > 0)
	{
		if (write (ofd, memory, extent) < extent)
		{
			return (-1);
		}
	}
	return (0);
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
		"s:",
		PUTOPTV_S_FUNNEL,
		"copy one or more files to a  serial device",
		"s n\tline speed is (n) [115200]",
		(char const *)(0)
	};
	struct termios restore;
	struct termios current;
	speed_t speed = B115200;
	byte buffer [512];
	signed c;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 's':
			if (baudrate (uintspec (optarg, 0, UINT_MAX), &speed))
			{
				error (1, 0, "could not set baud rate");
			}
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (!isatty (STDOUT_FILENO))
	{
		error (1, ENOTSUP, "stdout must be a serial line device");
	}
	tcflush (STDOUT_FILENO, TCIFLUSH);
	tcgetattr (STDOUT_FILENO, &restore);
	memset (&current, 0, sizeof (current));
	current.c_cflag = speed | CS8 | CLOCAL | CREAD;
	current.c_iflag = IGNPAR;
	current.c_oflag = 0;
	current.c_lflag = 0;
	current.c_cc [VTIME] = 0;
	current.c_cc [VMIN] = 5;
	tcsetattr (STDOUT_FILENO, TCSANOW, &current);
	if (!argc)
	{
		copy (STDIN_FILENO, STDOUT_FILENO, buffer, sizeof (buffer));
	}
	while ((argc) && (* argv))
	{
		if (efreopen (* argv, "rb", stdin))
		{
			copy (STDIN_FILENO, STDOUT_FILENO, buffer, sizeof (buffer));
		}
		argc--;
		argv++;
	}
	tcsetattr (STDOUT_FILENO, TCSANOW, &restore);
	exit (0);
}

