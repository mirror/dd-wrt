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
 *   config2cfg.c - convert a .config file to a .cfg file;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#define _GETOPT_H

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/number.h"
#include "../tools/flags.h"
#include "../tools/memory.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../ram/sdram.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#include "../tools/checksum32.c"
#include "../tools/todigit.c"
#include "../tools/hexencode.c"
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"qv",
		"file [file] [...]",
		"convert ASCII SDRAM configuration files (DM) to binary (toolkit) format",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};
	struct config_ram config_ram;
	char string [(sizeof (config_ram) << 1) + 1];
	uint32_t checksum;
	flag_t flags = (flag_t)(0);
	signed state = 1;
	signed fd;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'q':
			_setbits (flags, SDRAM_SILENCE);
			break;
		case 'v':
			_setbits (flags, SDRAM_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	while ((argc-- > 0) && (* argv != (char const *)(0)))
	{

#if 0

		char const * pathname;
		char const * filename;
		char const * extender;
		for (pathname = filename = * argv; *pathname; pathname++)
		{
			if ((*pathname == '/') || (*pathname == '\\'))
			{
				filename = pathname + 1;
			}
		}
		for (pathname = extender = filename; *pathname; pathname++)
		{
			if (*pathname == '.')
			{
				extender = pathname;
			}
		}
		if (extender == filename)
		{
			extender = pathname;
		}

#endif

		if ((fd = open (* argv, O_BINARY|O_RDONLY)) == -1)
		{
			error (0, errno, "can't open %s for input", * argv);
			state = 1;
		}
		else if (read (fd, &string, sizeof (string)) < (ssize_t) (sizeof (string) - 1))
		{
			error (0, errno, "can't read %s", * argv);
			state = 1;
		}
		else
		{
			close (fd);
			if (hexencode ((uint8_t *) (&config_ram), sizeof (config_ram), string) == sizeof (config_ram))
			{
				error (1, errno, "%s is suspect", * argv);
			}
			checksum = checksum32 (&config_ram, sizeof (config_ram), 0);
			if ((fd = open (* argv, O_BINARY|O_CREAT|O_RDWR|O_TRUNC, FILE_FILEMODE)) == -1)
			{
				error (1, errno, "can't open %s for output", * argv);
			}
			write (fd, &config_ram, sizeof (config_ram));
			write (fd, &checksum, sizeof (checksum));
		}
		close (fd);
		argv++;
	}
	return (state);
}

