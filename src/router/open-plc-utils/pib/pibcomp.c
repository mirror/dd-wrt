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
 *   pibcomp.c - Qualcomm Atheros Parameter Information Block Compare Utility
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/number.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/chars.h"
#include "../tools/sizes.h"
#include "../tools/files.h"
#include "../nvm/nvm.h"
#include "../pib/pib.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/hexview.c"
#include "../tools/hexoffset.c"
#include "../tools/error.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvmseek2.c"
#endif

/*====================================================================*
 *
 *   void function (char const * filename [], flag_t flags);
 *
 *   read object definitions from stdin and use them to compare two
 *   files; dump only those objects that differ on stdout;
 *
 *
 *--------------------------------------------------------------------*/

static void function (char const * filename [], flag_t flags)

{
	unsigned file;
	unsigned object = 0;
	unsigned lineno = 1;
	signed fd [2];
	signed length = 0;
	off_t origin [2];
	off_t offset [2];
	off_t extent [2];
	char memory [_ADDRSIZE+1];
	char symbol [_NAMESIZE];
	char string [_LINESIZE];
	char * sp;
	signed c;
	for (file = 0; file < SIZEOF (fd); file++)
	{
		uint32_t version;
		if ((fd [file] = open (filename [file], O_BINARY|O_RDONLY)) == -1)
		{
			error (1, errno, "%s", filename [file]);
		}
		if (read (fd [file], &version, sizeof (version)) != sizeof (version))
		{
			error (1, errno, FILE_CANTREAD, filename [file]);
		}
		if ((extent [file] = lseek (fd [file], 0, SEEK_END)) == (off_t)(-1))
		{
			error (1, 0, FILE_CANTSIZE, filename [file]);
		}
		if ((origin [file] = lseek (fd [file], 0, SEEK_SET)))
		{
			error (1, errno, FILE_CANTHOME, filename [file]);
		}
		if (LE32TOH (version) == 0x60000000)
		{
			error (1, ENOTSUP, "%s is not a PIB file", filename [file]);
		}
		if (LE32TOH (version) == 0x00010001)
		{
			struct nvm_header2 nvm_header;
			if (nvmseek2 (fd [file], filename [file], &nvm_header, NVM_IMAGE_PIB))
			{
				error (1, ENOTSUP, "%s is not a PIB file", filename [file]);
			}
			origin [file] = lseek (fd [file], 0, SEEK_CUR);
		}
	}
	if (origin [0] != origin [1])
	{
		error (0, EINVAL, "PIBs have different offsets");
	}
	memset (offset, 0, sizeof (offset));
	while ((c = getc (stdin)) != EOF)
	{
		if ((c == '#') || (c == ';'))
		{
			do
			{
				c = getc (stdin);
			}
			while (nobreak (c));
			lineno++;
			continue;
		}
		if (isspace (c))
		{
			if (c == '\n')
			{
				lineno++;
			}
			continue;
		}
		if (c == '+')
		{
			do
			{
				c = getc (stdin);
			}
			while (isblank (c));
		}
		length = 0;
		while (isdigit (c))
		{
			length *= 10;
			length += c - '0';
			c = getc (stdin);
		}
		while (isblank (c))
		{
			c = getc (stdin);
		}
		sp = symbol;
		if (isalpha (c) || (c == '_'))
		{
			do
			{
				*sp++ = (char)(c);
				c = getc (stdin);
			}
			while (isident (c));
		}
		while (isblank (c))
		{
			c = getc (stdin);
		}
		if (c == '[')
		{
			*sp++ = (char)(c);
			c = getc (stdin);
			while (isblank (c))
			{
				c = getc (stdin);
			}
			while (isdigit (c))
			{
				*sp++ = (char)(c);
				c = getc (stdin);
			}
			while (isblank (c))
			{
				c = getc (stdin);
			}
			*sp = (char)(0);
			if (c != ']')
			{
				error (1, EINVAL, "Have '%s' but need ']' on line %d", symbol, lineno);
			}
			*sp++ = (char)(c);
			c = getc (stdin);
		}
		*sp = (char)(0);
		while (isblank (c))
		{
			c = getc (stdin);
		}
		sp = string;
		while (nobreak (c))
		{
			*sp++ = (char)(c);
			c = getc (stdin);
		}
		*sp = (char)(0);
		if (length > 0)
		{

#if defined (WIN32)

			char * buffer [2];
			buffer [0] = (char *)(emalloc (length));
			buffer [1] = (char *)(emalloc (length));

#else

			byte buffer [2][length];

#endif

			if ((read (fd [0], buffer [0], length) == length) && (read (fd [1], buffer [1], length) == length))
			{
				if (memcmp (buffer [0], buffer [1], length))
				{
					if (!object++)
					{
						for (c = 0; c < _ADDRSIZE + 65; c++)
						{
							putc ('-', stdout);
						}
						putc ('\n', stdout);
					}
					printf ("%s %d %s\n", hexoffset (memory, sizeof (memory), offset [0]), length, symbol);
					for (c = 0; c < _ADDRSIZE; c++)
					{
						putc ('-', stdout);
					}
					printf (" %s\n", filename [0]);
					hexview (buffer [0], offset [0], length, stdout);
					for (c = 0; c < _ADDRSIZE; c++)
					{
						putc ('-', stdout);
					}
					printf (" %s\n", filename [1]);
					hexview (buffer [1], offset [1], length, stdout);
					for (c = 0; c < _ADDRSIZE + 65; c++)
					{
						putc ('-', stdout);
					}
					putc ('\n', stdout);
				}
			}

#if defined (WIN32)

			free (buffer [0]);
			free (buffer [1]);

#endif

		}
		offset [0] += length;
		offset [1] += length;
		lineno++;
	}
	if (_allclr (flags, PIB_SILENCE))
	{
		offset [0] += origin [0];
		offset [1] += origin [1];
		for (file = 0; file < SIZEOF (extent); file++)
		{
			if (offset [file] < extent [file])
			{
				error (0, 0, "%s exceeds definition by " OFF_T_SPEC " bytes", filename [file], extent [file] - offset [file]);
			}
			if (offset [file] > extent [file])
			{
				error (0, 0, "definition exceeds %s by " OFF_T_SPEC " bytes", filename [file], offset [file] - extent [file]);
			}
		}
		if (extent [0] > extent [1])
		{
			error (0, 0, "%s exceeds %s by " OFF_T_SPEC " bytes", filename [0], filename [1], extent [0] - extent [1]);
		}
		if (extent [1] > extent [0])
		{
			error (0, 0, "%s exceeds %s by " OFF_T_SPEC " bytes", filename [1], filename [0], extent [1] - extent [0]);
		}
	}
	close (fd [0]);
	close (fd [1]);
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"f:qv",
		"file1 file2",
		"Qualcomm Atheros Parameter Information Block Compare Utility",
		"f f\tobject definition file",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *)(0)
	};
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'f':
			if (!freopen (optarg, "rb", stdin))
			{
				error (1, errno, "%s", optarg);
			}
			break;
		case 'q':
			_setbits (flags, PIB_SILENCE);
			break;
		case 'v':
			_setbits (flags, PIB_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 2)
	{
		error (1, 0, "Need two files to compare.");
	}
	function (argv, flags);
	return (0);
}

