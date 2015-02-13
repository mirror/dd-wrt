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
 *   qosinfo.c - print qos information;
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

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define TM_VERBOSE (1 << 0)
#define TM_SILENCE (1 << 1)

#define OFFSET 0x020C
#define LENGTH 0x0800

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
		"print qos information",
		(char const *) (0)
	};

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

	struct __packed QoS
	{
		uint8_t UniCastPriority;
		uint8_t McastPriority;
		uint8_t IGMPPriority;
		uint8_t AVStreamPriority;
		uint32_t PriorityTTL [4];
		uint8_t EnableVLANOver;
		uint8_t EnableTOSOver;
		uint16_t RSVD1;
		uint16_t VLANPrioMatrix;
		uint16_t TOSPrecMatrix;
		uint8_t NumberOfConfigEntries;
		struct
		{
			uint8_t one;
			uint8_t two;
		}
		AggregateConfigEntries [8];
		uint8_t CustomAggregationParameters [384];
		uint8_t RSVD2 [1619];
	}
	QoS;

#ifndef __GNUC__
#pragma pack (pop)
#endif

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
		else if (read (fd, &QoS, sizeof (QoS)) != sizeof (QoS))
		{
			error (0, errno, "Can't read %s", * argv);
			close (fd);
		}
		else
		{
			for (c = 0; c < 8; c++)
			{
				unsigned VLAN = (QoS.VLANPrioMatrix >> (c * 2)) & 0x03;
				unsigned TOS = (QoS.TOSPrecMatrix >> (c * 2)) & 0x03;
				printf ("VLAN %d TOS %d\n", VLAN, TOS);
			}
			for (c = 0; c < 4; c++)
			{
				printf ("TTL [%d]=%d\n", c, QoS.PriorityTTL [c]);
			}
			for (c = 0; c < 8; c++)
			{
				printf ("AggregateConfigEntries [%d] %02x %02X\n", c, QoS.AggregateConfigEntries [c].one, QoS.AggregateConfigEntries [c].two);
			}
			printf ("QoS "SIZE_T_SPEC" %04X\n", sizeof (QoS), (unsigned int) sizeof (QoS));
			close (fd);
		}
		argc--;
		argv++;
	}
	return (0);
}

