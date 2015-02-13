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

/*====================================================================*"
 *
 *   chknvm.c
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/memory.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../ram/sdram.h"
#include "../nvm/nvm.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/strfbits.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ram/sdrampeek.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvm.c"
#include "../nvm/nvmpeek.c"
#include "../nvm/nvmpeek1.c"
#include "../nvm/nvmpeek2.c"
#include "../nvm/manifest.c"
#include "../nvm/fdmanifest.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define HARDWARE 0
#define SOFTWARE 1
#define VER 2
#define REV 3
#define BUILD 6
#define DATE 7

/*====================================================================*
 *
 *   unsigned string2vector (char ** vector, length, char * string, char c);
 *
 *   convert string to a vector and return vector count; split string
 *   on characer (c);
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static unsigned string2vector (char ** vector, char * string, char c)

{
	char ** origin = vector;
	for (*vector++ = string; *string; string++)
	{
		if (*string == c)
		{
			*string++ = (char)(0);
			*vector++ = string--;
		}
	}
	*vector = (char *)(0);
	return ((unsigned)(vector - origin));
}


/*====================================================================*
 *
 *   void firmware (signed fd, char const * filename, unsigned module, unsigned offset, flag_t flags);
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static void firmware (signed fd, char const * filename, unsigned module, unsigned offset, flag_t flags)

{
	char memory [512];
	read (fd, memory, sizeof (memory));
	lseek (fd, (off_t)(0) - sizeof (memory), SEEK_CUR);
	if (_anyset (flags, NVM_FIRMWARE))
	{
		if ((*memory > 0x20) && (*memory < 0x7F))
		{
			printf ("%s (%d) %s\n", filename, module, memory);
		}
		else
		{
			printf ("%s (%d) %s\n", filename, module, memory + offset);
		}
	}
	if (_anyset (flags, NVM_IDENTITY))
	{
		char * vector [16];
		char buffer [256];
		if ((* memory > 0x20) && (* memory < 0x7f))
		{
			strncpy (buffer, memory, sizeof (buffer));
		}
		else
		{
			strncpy (buffer, memory + offset, sizeof (buffer));
		}
		string2vector (vector, buffer, '-');
		printf ("%s ", vector [HARDWARE]);
		printf ("%04d ", atoi (vector [BUILD]));
		printf ("%s ", vector [DATE]);
		printf ("%s.", vector [VER]);
		printf ("%s ", vector [REV]);
		printf ("%s (%d)\n", filename, module);
		return;
	}
	return;
}

/*====================================================================*
 *
 *   signed nvmchain1 (signed fd, char const * filename, flag_t flags);
 *
 *   validate a legcy PLC firmware file; return 0 on success or -1 on
 *   error;
 *
 *   keep as little information as possible in memory; this slows
 *   validation but minimizes the resources needed at runtime so
 *   that resource requirements do not grow with file size;
 *
 *   the checksum of the entire header, including header checksum, is
 *   always 0 for valid headers; similarly, the checksum of the module
 *   and module checksum is always 0 for valid modules;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed nvmchain1 (signed fd, char const * filename, flag_t flags)

{
	struct nvm_header1 nvm_header;
	struct config_ram config_ram;
	unsigned module = 0;
	do
	{
		if (read (fd, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_CANTREAD, filename, module);
			}
			return (-1);
		}
		if (LE32TOH (nvm_header.HEADERVERSION) != 0x60000000)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_CHECKSUM, filename, module);
			}
			return (-1);
		}
		if (_anyset (flags, NVM_VERBOSE))
		{
			printf ("------- %s (%d) -------\n", filename, module);
			nvmpeek1 (&nvm_header);
		}
		if (nvm_header.HEADERMINORVERSION)
		{
			if (LE32TOH (nvm_header.IMAGETYPE) == NVM_IMAGE_CONFIG_SYNOPSIS)
			{
				if (_anyset (flags, NVM_SDRAM))
				{
					printf ("------- %s (%d) -------\n", filename, module);
					read (fd, &config_ram, sizeof (config_ram));
					lseek (fd, (off_t)(0) - sizeof (config_ram), SEEK_CUR);
					sdrampeek (&config_ram);
				}
			}
			else if (LE32TOH (nvm_header.IMAGETYPE) == NVM_IMAGE_FIRMWARE)
			{
				firmware (fd, filename, module, 0x70, flags);
			}
		}
		else if (!module)
		{
			if (_anyset (flags, NVM_SDRAM))
			{
				printf ("------- %s (%d) -------\n", filename, module);
				read (fd, &config_ram, sizeof (config_ram));
				lseek (fd, (off_t)(0) - sizeof (config_ram), SEEK_CUR);
				sdrampeek (&config_ram);
			}
		}
		else if (!nvm_header.NEXTHEADER)
		{
			firmware (fd, filename, module, 0x70, flags);
		}
		if (fdchecksum32 (fd, LE32TOH (nvm_header.IMAGELENGTH), nvm_header.IMAGECHECKSUM))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_IMG_CHECKSUM, filename, module);
			}
			return (-1);
		}
		module++;
	}
	while (nvm_header.NEXTHEADER);
	if (lseek (fd, 0, SEEK_CUR) != lseek (fd, 0, SEEK_END))
	{
		if (_allclr (flags, NVM_SILENCE))
		{
			error (0, errno, NVM_HDR_LINK, filename, module);
		}
		return (-1);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed nvmchain2 (signed fd, char const * filename, flag_t flags);
 *
 *   validate a PLC image file; return 0 on success or -1 on error;
 *
 *   keep as little information as possible in memory; this slows
 *   validation but minimizes the resources needed at runtime so
 *   that resource requirements do not grow with file size;
 *
 *   the checksum of the entire header, including header checksum, is
 *   always 0 for valid headers; similarly, the checksum of the module
 *   and module checksum is always 0 for valid modules;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed nvmchain2 (signed fd, char const * filename, flag_t flags)

{
	struct nvm_header2 nvm_header;
	unsigned module = 0;
	uint32_t origin = ~0;
	uint32_t offset = 0;
	do
	{
		if (read (fd, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_CANTREAD, filename, module);
			}
			return (-1);
		}
		if (LE16TOH (nvm_header.MajorVersion) != 1)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (LE16TOH (nvm_header.MinorVersion) != 1)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_CHECKSUM, filename, module);
			}
			return (-1);
		}
		if (LE32TOH (nvm_header.PrevHeader) != origin)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_LINK, filename, module);
			}
			return (-1);
		}
		if (_anyset (flags, NVM_VERBOSE))
		{
			printf ("------- %s (%d) -------\n", filename, module);
			nvmpeek2 (&nvm_header);
		}
		if (LE32TOH (nvm_header.ImageType) == NVM_IMAGE_MANIFEST)
		{
			if (_anyset (flags, NVM_MANIFEST))
			{
				fdmanifest (fd, filename, &nvm_header, module);
				return (0);
			}
		}
		if (fdchecksum32 (fd, LE32TOH (nvm_header.ImageLength), nvm_header.ImageChecksum))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_IMG_CHECKSUM, filename, module);
			}
			return (-1);
		}
		origin = offset;
		offset = LE32TOH (nvm_header.NextHeader);
		module++;
	}
	while (~nvm_header.NextHeader);
	if (lseek (fd, 0, SEEK_CUR) != lseek (fd, 0, SEEK_END))
	{
		if (_allclr (flags, NVM_SILENCE))
		{
			error (0, errno, NVM_HDR_LINK, filename, module);
		}
		return (-1);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed chknvm (char const * filename, flag_t flags);
 *
 *   validate a PLC firmware module file; keep as little information
 *   as possible in memory; this slows validation but minimizes the
 *   resources needed at runtime; resource requirements do not grow
 *   with file size;
 *
 *   determine file format based on module header version then rewind
 *   the file and call the appropriate chknvm;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed chknvm (char const * filename, flag_t flags)

{
	uint32_t version;
	signed status;
	signed fd;
	if ((fd = open (filename, O_BINARY|O_RDONLY)) == -1)
	{
		if (_allclr (flags, NVM_SILENCE))
		{
			error (0, errno, FILE_CANTOPEN, filename);
		}
		return (-1);
	}
	if (read (fd, &version, sizeof (version)) != sizeof (version))
	{
		if (_allclr (flags, NVM_SILENCE))
		{
			error (0, errno, FILE_CANTREAD, filename);
		}
		return (-1);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		if (_allclr (flags, NVM_SILENCE))
		{
			error (0, errno, FILE_CANTHOME, filename);
		}
		return (-1);
	}
	if (LE32TOH (version) == 0x60000000)
	{
		status = nvmchain1 (fd, filename, flags);
	}
	else
	{
		status = nvmchain2 (fd, filename, flags);
	}
	close (fd);
	return (status);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *   validate a PLC firmware image file; keep as little information
 *   as possible in memory; this slows validation but minimizes the
 *   resources needed at runtime; resource requirements do not grow
 *   with file size;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"imqrsv",
		"file [file] [...]",
		"Qualcomm Atheros PLC Image File Validator",
		"i\tprint firmware identity string",
		"m\tdisplay manifest",
		"q\tsuppress messages",
		"r\tprint firmware revision string",
		"s\tprint SDRAM configuration blocks",
		"v\tverbose messages",
		(char const *) (0)
	};
	signed state = 0;
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'i':
			_setbits (flags, NVM_IDENTITY);
			break;
		case 'r':
			_setbits (flags, NVM_FIRMWARE);
			break;
		case 'm':
			_setbits (flags, NVM_MANIFEST);
			break;
		case 's':
			_setbits (flags, NVM_SDRAM);
			break;
		case 'q':
			_setbits (flags, NVM_SILENCE);
			break;
		case 'v':
			_setbits (flags, NVM_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	while ((argc) && (* argv))
	{
		if (chknvm (* argv, flags))
		{
			state = 1;
		}
		else if (_allclr (flags, (NVM_VERBOSE|NVM_SILENCE|NVM_MANIFEST|NVM_FIRMWARE|NVM_IDENTITY|NVM_SDRAM)))
		{
			printf ("file %s looks good\n", * argv);
		}
		argc--;
		argv++;
	}
	return (state);
}

