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
 *   psgraph.c - print PIB prescaler graph;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <limits.h>
#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../plc/plc.h"
#include "../pib/pib.h"

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
#include "../pib/pibscalers.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PSGRAPH_VERBOSE (1 << 0)
#define PSGRAPH_SILENCE (1 << 1)

/*====================================================================*
 *
 *   void int6x00Prescalers (struct _file_ * pib, unsigned scale);
 *
 *
 *--------------------------------------------------------------------*/

static void int6x00Prescalers (struct _file_ * pib, unsigned scale)

{
	unsigned index = 0;
	if (lseek (pib->file, INT_PRESCALER_OFFSET, SEEK_SET) != INT_PRESCALER_OFFSET)
	{
		error (1, errno, "Can't seek %s", pib->name);
	}
	for (index = 0; index < INT_PRESCALER_LENGTH; index++)
	{
		uint32_t value;
		if (read (pib->file, &value, sizeof (value)) != sizeof (value))
		{
			error (1, errno, "can't read %s", pib->name);
		}
		printf (" %6.3f %04d %6d ", INDEX_TO_FREQ (index), index, value);
		while (value > scale)
		{
			printf ("#");
			value -= scale;
		}
		printf ("\n");
	}
	return;
}

/*====================================================================*
 *
 *   void qca7x00Prescalers (struct _file_ * pib, unsigned scale);
 *
 *--------------------------------------------------------------------*/

static void qca7x00Prescalers (struct _file_ * pib, unsigned scale)
{
	unsigned index = 0;
	if (lseek (pib->file, QCA_PRESCALER_OFFSET, SEEK_SET) != QCA_PRESCALER_OFFSET)
	{
		error (1, errno, FILE_CANTSEEK, pib->name);
	}
	for (index = 0; index < QCA_PRESCALER_LENGTH; index++)
	{
		byte value;
		if (read (pib->file, &value, sizeof (value)) != sizeof (value))
		{
			error (1, errno, "can't read %s", pib->name);
		}
		printf (" %6.3f %04d %6d ", INDEX_TO_FREQ (index), index, value);
		while (value > scale)
		{
			printf ("#");
			value -= scale;
		}
		printf ("\n");
	}
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
		"s:",
		"file [> stdout]",
		"print PIB prescaler graph",
		"s n\tscale",
		(char const *) (0)
	};
	struct _file_ pib;
	unsigned count = 0;
	unsigned scale = 10;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 's':
			scale = (unsigned)(uintspec (optarg, 1, UCHAR_MAX));
			break;
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
		count = pibscalers (&pib);
		if (count == PLC_CARRIERS)
		{
			qca7x00Prescalers (&pib, scale);
		}
		else if (count == AMP_CARRIERS)
		{
			error (1, ENOTSUP, "AR7x00 PIB Format");
		}
		else if (count == INT_CARRIERS)
		{
			int6x00Prescalers (&pib, scale);
		}
		else
		{
			error (1, ENOTSUP, "Unknown PIB Format");
		}
		close (pib.file);
	}
	return (0);
}

