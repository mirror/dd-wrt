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
 *   gpioinfo.c - print gpio Iinformation
 *
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
#include "../tools/types.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/hexstring.c"
#include "../tools/hexdecode.c"
#include "../tools/error.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define TM_VERBOSE (1 << 0)
#define TM_SILENCE (1 << 1)

#define OFFSET 0x24BF
#define LENGTH 50

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
		"",
		"file [file] [...] [> stdout]",
		"print GPIO information",
		(char const *) (0)
	};

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

	typedef struct __packed EventBlock
	{
		uint8_t EvtPriorityId;
		uint8_t EvtId;
		uint8_t BehId [3];
		uint16_t ParticipatingGPIOs;
		uint8_t EventAttributes;
		uint8_t RSVD [3];
	}
	EventBlock;

#ifndef __GNUC__
#pragma pack (pop)
#endif

	struct EventBlock EventBlockArray [50];
	file_t fd;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	while ((argc) && (* argv))
	{
		if ((fd = open (* argv, O_BINARY|O_RDONLY)) == -1)
		{
			error (0, errno, "Can't open %s", * argv);
		}
		else if (lseek (fd, OFFSET, SEEK_SET) != OFFSET)
		{
			error (0, errno, "Can't seek %s", * argv);
			close (fd);
		}
		else if (read (fd, &EventBlockArray, sizeof (EventBlockArray)) != sizeof (EventBlockArray))
		{
			error (0, errno, "Can't read %s", * argv);
			close (fd);
		}
		else
		{
			for (c = 0; c < LENGTH; c++)
			{
				struct EventBlock * EventBlock = (&EventBlockArray [c]);
				char string [10];
				printf ("EvtPriorityId %3d ", EventBlock->EvtPriorityId);
				printf ("EvtId %3d ", EventBlock->EvtId);
				printf ("BehId %s ", hexstring (string, sizeof (string), EventBlock->BehId, sizeof (EventBlock->BehId)));
				printf ("ParticipatingGPIOs %3d ", EventBlock->ParticipatingGPIOs);
				printf ("EventAttributes %3d ", EventBlock->EventAttributes);
				printf ("\n");
			}
			close (fd);
		}
		argc--;
		argv++;
	}
	return (0);
}

