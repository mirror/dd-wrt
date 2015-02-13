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
 *   psin.c - load prescalers into int6000 parameter file;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/endian.h"
#include "../tools/getoptv.h"
#include "../tools/number.h"
#include "../tools/chars.h"
#include "../tools/types.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../key/HPAVKey.h"
#include "../pib/pib.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/fdchecksum32.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile1.c"
#include "../pib/piblock.c"
#include "../pib/pibscalers.c"
#endif

#ifndef MAKEFILE
#include "../key/keys.c"
#endif

/*====================================================================*
 *
 *
 *   signed ar7x00_psin (struct _file_ * pib, uint32_t value, uint32_t index);
 *
 *   Places a single 10 bit prescaler into the pib file at index;
 *
 *--------------------------------------------------------------------*/

signed ar7x00_psin (struct _file_ * pib, uint32_t value, uint32_t index)

{
	off_t offset = AMP_PRESCALER_OFFSET + (index * 10 / 8);
	uint8_t bit_offset = (index * 10) % 8;
	uint16_t tmp;
	if (lseek (pib->file, offset, SEEK_SET) != offset)
	{
		return (-1);
	}
	if (read (pib->file, &tmp, sizeof (tmp)) != sizeof (tmp))
	{
		return (-1);
	}
	if (lseek (pib->file, offset, SEEK_SET) != offset)
	{
		return (-1);
	}
	value &= 0x03FF;
	tmp = LE16TOH (tmp);
	tmp &= ~(0x03FF << bit_offset);
	tmp |= value << bit_offset;
	tmp = HTOLE16 (tmp);
	if (write (pib->file, &tmp, sizeof (tmp)) != sizeof (tmp))
	{
		return (-1);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed psin (struct _file_ * pib);
 *
 *
 *--------------------------------------------------------------------*/

static signed psin (struct _file_ * pib)

{
	unsigned index = 0;
	unsigned count = 0;
	unsigned limit = pibscalers (pib);
	uint32_t value = 0;
	signed c;
	if ((limit != INT_CARRIERS) && (limit != AMP_CARRIERS))
	{
		error (1, 0, "Don't understand this PIB's prescaler format");
	}
	if (limit == INT_CARRIERS)
	{
		if (lseek (pib->file, INT_PRESCALER_OFFSET, SEEK_SET) != INT_PRESCALER_OFFSET)
		{
			error (1, errno, FILE_CANTSEEK, pib->name);
		}
	}
	while ((c = getc (stdin)) != EOF)
	{
		if (isspace (c))
		{
			continue;
		}
		if ((c == '#') || (c == ';'))
		{
			do
			{
				c = getc (stdin);
			}
			while (nobreak (c));
			continue;
		}
		index = 0;
		while (isdigit (c))
		{
			index *= 10;
			index += c - '0';
			c = getc (stdin);
		}
		if (index != count)
		{
			error (1, ECANCELED, "Carrier %d out of order", index);
		}
		if (index >= limit)
		{
			error (1, EOVERFLOW, "Too many prescalers");
		}
		while (isblank (c))
		{
			c = getc (stdin);
		}
		value = 0;
		while (isxdigit (c))
		{
			value *= 16;
			value += todigit (c);
			c = getc (stdin);
		}
		if (limit == INT_CARRIERS)
		{
			value = HTOLE32 (value);
			if (write (pib->file, &value, sizeof (value)) != sizeof (value))
			{
				error (1, errno, "Can't save %s", pib->name);
			}
		}
		else if (limit == AMP_CARRIERS)
		{
			if (value & ~0x03FF)
			{
				error (1, errno, "Position %d has invalid prescaler value", index);
			}
			if (ar7x00_psin (pib, value, index))
			{
				error (1, errno, "Can't update %s", pib->name);
			}
		}
		while (nobreak (c))
		{
			c = getc (stdin);
		};
		count++;
	}
	return (0);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv [])
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"",
		"pibfile [< scalers]",
		"load prescalers into int6000 parameter file",
		(char const *) (0)
	};
	struct _file_ pib;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 1)
	{
		error (1, ECANCELED, "Only one target file allowed");
	}
	if ((argc) && (* argv))
	{
		pib.name = * argv;
		if ((pib.file = open (pib.name, O_BINARY|O_RDWR)) == -1)
		{
			error (1, errno, "Can't open %s", pib.name);
		}
		else if (pibfile1 (&pib))
		{
			error (1, errno, "Bad PIB file: %s", pib.name);
		}
		else if (psin (&pib))
		{
			error (1, ECANCELED, "%s", pib.name);
		}
		else if (piblock (&pib))
		{
			error (1, ECANCELED, "%s", pib.name);
		}
		close (pib.file);
	}
	return (0);
}

