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
 *   setpib.c -
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../pib/pib.h"
#include "../nvm/nvm.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/basespec.c"
#include "../tools/dataspec.c"
#include "../tools/bytespec.c"
#include "../tools/todigit.c"
#include "../tools/hexdump.c"
#include "../tools/hexpeek.c"
#include "../tools/fdchecksum32.c"
#include "../tools/checksum32.c"
#include "../tools/memencode.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvmseek2.c"
#endif

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define SETPIB_VERBOSE (1 << 0)
#define SETPIB_SILENCE (1 << 1)
#define SETPIB_HEADERS (1 << 2)
#define SETPIB_CHANGED (1 << 3)
#define SETPIB_WINDOW 32

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

static flag_t flags = (flag_t)(0);

/*====================================================================*
 *
 *   signed modify (void * memory, size_t extent, int argc, char const * argv [], unsigned window)
 *
 *   apply a series of edits to a memory region; edits are specified
 *   in string vector argv [] which must follow rules understood by
 *   function memencode(); this function merely walks the vector and
 *   deals with error and display options;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed modify (void * memory, size_t extent, int argc, char const * argv [], unsigned window)

{
	uint32_t origin;
	uint32_t offset;
	if (!argc)
	{
		error (1, ENOTSUP, "Need an offset");
	}
	origin = offset = (size_t)(basespec (* argv, 16, sizeof (offset)));
	if (offset >= extent)
	{
		error (1, ECANCELED, "Offset %X exceeds file length of " SIZE_T_SPEC, offset, extent);
	}
	argc--;
	argv++;
	if (!argc)
	{
		_setbits (flags, SETPIB_VERBOSE);
	}
	while ((argc > 1) && (* argv))
	{
		_setbits (flags, SETPIB_CHANGED);
		offset += (unsigned)(memencode ((byte *)(memory) + offset, extent - offset, argv [0], argv [1]));
		argc -= 2;
		argv += 2;
	}
	if (argc)
	{
		error (1, ECANCELED, "object %s needs a value", *argv);
	}
	if (_anyset (flags, SETPIB_VERBOSE))
	{
		hexpeek (memory, origin, offset, extent, window, stdout);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed pibimage1 (signed fd, char const * filename, int argc, char const * argv [], unsigned window);
 *
 *   read an entire flat parameter file into memory, edit it, save
 *   it and display it;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage1 (signed fd, char const * filename, int argc, char const * argv [], unsigned window)

{
	off_t extent;
	void * memory;
	if ((extent = lseek (fd, 0, SEEK_END)) == -1)
	{
		error (1, errno, FILE_CANTSIZE, filename);
	}
	if (!(memory = malloc (extent)))
	{
		error (1, errno, FILE_CANTLOAD, filename);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	if (read (fd, memory, extent) != extent)
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	if (modify (memory, extent, argc, argv, window))
	{
		error (1, errno, FILE_CANTEDIT, filename);
	}
	if (_anyset (flags, SETPIB_CHANGED))
	{
		struct pib_header * pib_header = (struct pib_header *)(memory);
		pib_header->CHECKSUM = checksum32 (memory, extent, pib_header->CHECKSUM);
		if (write (fd, memory, extent) != extent)
		{
			error (1, errno, FILE_CANTSAVE, filename);
		}
		if (lseek (fd, (off_t)(0) - extent, SEEK_CUR) == -1)
		{
			error (1, errno, FILE_CANTHOME, filename);
		}
	}
	free (memory);
	close (fd);
	return (0);
}


/*====================================================================*
 *
 *   signed pibimage2 (signed fd, char const * filename, int argc, char const * argv [], unsigned window);
 *
 *   read an entire flat parameter file into memory, edit it, save
 *   it and display it;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage2 (signed fd, char const * filename, struct nvm_header2 * nvm_header, int argc, char const * argv [], unsigned window)

{
	void * memory;
	off_t extent = LE32TOH (nvm_header->ImageLength);
	if (!(memory = malloc (extent)))
	{
		error (1, errno, FILE_CANTLOAD, filename);
	}
	if (read (fd, memory, extent) != extent)
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	if (lseek (fd, (off_t)(0) - extent, SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	if (modify (memory, extent, argc, argv, window))
	{
		error (1, errno, FILE_CANTEDIT, filename);
	}
	if (_anyset (flags, SETPIB_CHANGED))
	{
		nvm_header->ImageChecksum = checksum32 (memory, extent, 0);
		if (write (fd, memory, extent) != extent)
		{
			error (1, errno, FILE_CANTSAVE, filename);
		}
		if (lseek (fd, (off_t)(0) - extent, SEEK_CUR) == -1)
		{
			error (1, errno, FILE_CANTHOME, filename);
		}
		nvm_header->HeaderChecksum = checksum32 (nvm_header, sizeof (* nvm_header), nvm_header->HeaderChecksum);
		if (lseek (fd, (off_t)(0) - sizeof (* nvm_header), SEEK_CUR) == -1)
		{
			error (1, errno, FILE_CANTHOME, filename);
		}
		if (write (fd, nvm_header, sizeof (* nvm_header)) != sizeof (* nvm_header))
		{
			error (1, errno, FILE_CANTSAVE, filename);
		}
		if (lseek (fd, (off_t)(0) - sizeof (* nvm_header), SEEK_CUR) == -1)
		{
			error (1, errno, FILE_CANTHOME, filename);
		}
	}
	free (memory);
	return (0);
}


/*====================================================================*
 *
 *   signed function (int argc, char const * argv [], unsigned window);
 *
 *   call an appropriate parameter edit function based on the file
 *   header;
 *
 *   older parameter files are flat with their own header; newer ones
 *   are image chains where one of image contains the parameter block;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed function (int argc, char const * argv [], unsigned window)

{
	uint32_t version;
	char const * filename = * argv;
	signed status = -1;
	signed fd;
	if ((fd = open (filename, O_BINARY|O_RDWR)) == -1)
	{
		error (1, errno, FILE_CANTOPEN, filename);
	}
	if (read (fd, &version, sizeof (version)) != sizeof (version))
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	argc--;
	argv++;
	if (LE32TOH (version) == 0x00010001)
	{
		struct nvm_header2 nvm_header;
		if (!nvmseek2 (fd, filename, &nvm_header, NVM_IMAGE_PIB))
		{
			status = pibimage2 (fd, filename, &nvm_header, argc, argv, window);
		}
	}
	else
	{
		status = pibimage1 (fd, filename, argc, argv, window);
	}
	close (fd);
	return (status);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"qvw:x",
		"file base [type data] [type data] [...]\n\n\tstandard-length types are 'byte'|'word'|'long'|'huge'|'hfid'|'mac'|'key'\n\tvariable-length types are 'data'|'text'|'fill'|'skip'",
		"Qualcomm Atheros PIB File Editor",
		"q\tquiet mode",
		"v[v]\tverbose mode",
		"w n\twindow size is (n) [" LITERAL (SETPIB_WINDOW) "]",
		"x\trepair checksum",
		(char const *) (0)
	};
	unsigned window = SETPIB_WINDOW;
	signed c;
	optind = 1;
	opterr = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'q':
			_setbits (flags, SETPIB_SILENCE);
			break;
		case 'v':
			if (_anyset (flags, SETPIB_VERBOSE))
			{
				_setbits (flags, SETPIB_HEADERS);
			}
			_setbits (flags, SETPIB_VERBOSE);
			break;
		case 'w':
			window = (unsigned)(uintspec (optarg, 0, UINT_MAX));
			_setbits (flags, SETPIB_VERBOSE);
			break;
		case 'x':
			_setbits (flags, SETPIB_CHANGED);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if ((argc) && (* argv))
	{
		function (argc, argv, window);
	}
	return (0);
}

