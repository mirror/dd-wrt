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
 *   plcdevs.c -
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <limits.h>
#include <memory.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../ether/channel.h"
#include "../ether/ether.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#include "../tools/hexdecode.c"
#include "../tools/decdecode.c"
#include "../tools/hexstring.c"
#include "../tools/decstring.c"
#include "../tools/typename.c"
#include "../tools/hexdump.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/hostnics.c"
#include "../ether/openchannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/closechannel.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#endif

#ifndef MAKEFILE
#include "../plc/Platform.c"
#include "../plc/chipset.c"
#include "../plc/Devices.c"
#endif

/*====================================================================*
 *
 *   void enumerate ();
 *
 *   enumerate host interfaces on stdout;
 *
 *--------------------------------------------------------------------*/

static void enumerate (struct channel * channel, struct nic nic [], unsigned size)

{
	extern const byte localcast [ETHER_ADDR_LEN];
	for (; size--; nic++)
	{
		byte memory [ETHER_ADDR_LEN];
		char string [ETHER_ADDR_LEN * 3];
		memset (memory, 0x00, sizeof (memory));
		if (!memcmp (memory, nic->ethernet, sizeof (memory)))
		{
			continue;
		}
		memset (memory, 0xFF, sizeof (memory));
		if (!memcmp (memory, nic->ethernet, sizeof (memory)))
		{
			continue;
		}

#if defined (WINPCAP) || defined (LIBPCAP)

		printf (" %d", nic->ifindex);

#elif defined (__linux__) || defined (__OpenBSD__) || defined (__APPLE__)

		printf (" %s", nic->ifname);

#else
#error "Unknown environment"
#endif

		printf (" %s", hexstring (string, sizeof (string), nic->ethernet, sizeof (nic->ethernet)));
		printf (" %s", decstring (string, sizeof (string), nic->internet, sizeof (nic->internet)));

#if 0

		printf (" %s", nic->ifname);
		printf (" %s", nic->ifdesc);

#endif

		channel->ifname = nic->ifname;
		openchannel (channel);
		Platform (channel, localcast);
		closechannel (channel);
		printf ("\n");
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"qt:v",
		PUTOPTV_S_DIVINE,
		"Atheros Ethernet Interface Enumerator",
		(char const *) (0)
	};
	struct nic nics [16];
	unsigned size = hostnics (nics, sizeof (nics) / sizeof (struct nic));
	signed c;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc)
	{
		error (1, ENOTSUP, ERROR_TOOMANY);
	}
	enumerate (&channel, nics, size);
	return (0);
}

