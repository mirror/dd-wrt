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
 *   psout.c - Export PIB Prescalers;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/number.h"
#include "../tools/chars.h"
#include "../tools/types.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/endian.h"
#include "../pib/pib.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile.c"
#include "../pib/pibfile1.c"
#include "../pib/pibfile2.c"
#include "../pib/pibscalers.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PSOUT_VERBOSE (1 << 0)
#define PSOUT_SILENCE (1 << 1)

/*====================================================================*
 *
 *   void ar7x00Prescalers (struct _file_ * pib);
 *
 *--------------------------------------------------------------------*/

static void ar7x00Prescalers (struct _file_ * pib)

{

#if 0

	uint32_t value;
	uint32_t temp;
	uint32_t buffer = 0;
	unsigned bits = 0;
	unsigned index = 0;
	if (lseek (pib->file, AMP_PRESCALER_OFFSET, SEEK_SET) != AMP_PRESCALER_OFFSET)
	{
		error (1, errno, FILE_CANTSEEK, pib->name);
	}

/* no dependency on math lib */

	while (index < AMP_CARRIERS)
	{
		if (read (pib->file, &temp, sizeof (temp)) != sizeof (temp))
		{
			error (1, errno, FILE_CANTREAD, pib->name);
		}
		temp = LE32TOH (temp);
		buffer |= (temp << bits);
		if (bits == 0)
		{
			bits = 8 * sizeof (buffer);
		}
		else
		{
			if (lseek (pib->file, -1, SEEK_CUR) == -1)
			{
				error (1, errno, "could not seek backwards");
			}
			bits += 8 * (sizeof (buffer) - 1);
		}
		while (bits >= 10)
		{
			value = (buffer & 0x000003FF);
			printf ("%08d %08X\n", index, value);
			buffer = buffer >> 10;
			++index;
			bits -= 10;
		}
	}

#elif 0

	uint32_t value;
	uint32_t temp;
	uint32_t buffer = 0;
	unsigned bits = 0;
	unsigned index = 0;
	if (lseek (pib->file, AMP_PRESCALER_OFFSET, SEEK_SET) != AMP_PRESCALER_OFFSET)
	{
		error (1, errno, FILE_CANTSEEK, pib->name);
	}

/* more generic code, but requires math lib, add -lm to LFLAGS */

	while (index < AMP_CARRIERS)
	{
		if (read (pib->file, &temp, sizeof (temp)) != sizeof (temp))
		{
			error (1, errno, FILE_CANTREAD, pib->name);
		}
		temp = LE32TOH (temp);
		buffer |= (temp << bits);
		if (lseek (pib->file, (off_t)(-ceil (bits / 8.0)), SEEK_CUR) == -1)
		{
			error (1, errno, "could not seek backwards");
		}
		bits += 8 * (sizeof (buffer) - (unsigned)(ceil (bits / 8.0)));
		while (bits >= 10)
		{
			value = (buffer & 0x000003FF);
			printf ("%08d %08X\n", index, value);
			buffer = buffer >> 10;
			++index;
			bits -= 10;
		}
	}

#else

	uint16_t upper;
	uint16_t lower;
	unsigned index = 0;
	byte buffer [AMP_PRESCALER_LENGTH];
	byte * p = buffer;
	if (lseek (pib->file, AMP_PRESCALER_OFFSET, SEEK_SET) != AMP_PRESCALER_OFFSET)
	{
		error (1, errno, FILE_CANTSEEK, pib->name);
	}
	if (read (pib->file, buffer, sizeof (buffer)) != sizeof (buffer))
	{
		error (1, errno, FILE_CANTREAD, pib->name);
	}

/*
 * |00000000|00111111|11112222|22222233|33333333|
 * |01234567|89012345|67890123|45678901|23456789|
 */

	while (index < AMP_CARRIERS)
	{
		lower = (*p++ & 0xFF) >> 0;
		upper = (*p & 0x03) << 8;
		printf ("%08d %08X\n", index++, upper | lower);
		lower = (*p++ & 0xFC) >> 2;
		upper = (*p & 0x0F) << 6;
		printf ("%08d %08X\n", index++, upper | lower);
		lower = (*p++ & 0xF0) >> 4;
		upper = (*p & 0x3F) << 4;
		printf ("%08d %08X\n", index++, upper | lower);
		lower = (*p++ & 0xC0) >> 6;
		upper = (*p++ & 0xFF) << 2;
		printf ("%08d %08X\n", index++, upper | lower);
	}

#endif

	return;
}


/*====================================================================*
 *
 *   void int6x00Prescalers (struct _file_ * pib);
 *
 *--------------------------------------------------------------------*/

static void int6x00Prescalers (struct _file_ * pib)

{
	unsigned index = 0;
	if (lseek (pib->file, INT_PRESCALER_OFFSET, SEEK_SET) != INT_PRESCALER_OFFSET)
	{
		error (1, errno, FILE_CANTSEEK, pib->name);
	}
	while (index < INT_CARRIERS)
	{
		uint32_t value;
		if (read (pib->file, &value, sizeof (value)) != sizeof (value))
		{
			error (1, errno, FILE_CANTREAD, pib->name);
		}
		printf ("%08d %08X\n", index++, LE32TOH (value));
	}
	return;
}

/*====================================================================*
 *
 *   void qca7x00Prescalers (struct _file_ * pib);
 *
 *--------------------------------------------------------------------*/

static void qca7x00Prescalers (struct _file_ * pib)
{
	unsigned index = 0;
	byte buffer [QCA_PRESCALER_LENGTH];
	byte * p = buffer;
	if (lseek (pib->file, QCA_PRESCALER_OFFSET, SEEK_SET) != QCA_PRESCALER_OFFSET)
	{
		error (1, errno, FILE_CANTSEEK, pib->name);
	}
	if (read (pib->file, buffer, sizeof (buffer)) != sizeof (buffer))
	{
		error (1, errno, FILE_CANTREAD, pib->name);
	}

	while (index < PLC_CARRIERS)
	{
		printf ("%08d %08x\n", index++, *p++);
	}
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
		"pibfile [> scalers]",
		"Export PIB Prescalers",
		(char const *) (0)
	};
	struct _file_ pib;
	unsigned scalers;
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
		error (1, ENOTSUP, "Only one file is permitted");
	}
	if ((argc) && (* argv))
	{
		pib.name = * argv;
		if ((pib.file = open (pib.name, O_BINARY|O_RDONLY)) == -1)
		{
			error (1, errno, "Can't open %s", pib.name);
		}
		if (pibfile (&pib))
		{
			error (1, errno, "Bad PIB file: %s", pib.name);
		}
		scalers = pibscalers (&pib);
		if (scalers == PLC_CARRIERS)
		{
			qca7x00Prescalers (&pib);
		}
		else if (scalers == AMP_CARRIERS)
		{
			ar7x00Prescalers (&pib);
		}
		else if (scalers == INT_CARRIERS)
		{
			int6x00Prescalers (&pib);
		}
		else
		{
			error (1, ENOTSUP, "Unexpected number of carriers");
		}
		close (pib.file);
	}
	return (0);
}

