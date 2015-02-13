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
 *   pcapdevs.c - pcap device enumerator;
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
#include <errno.h>
#include <string.h>
#include <pcap.h>

#if defined (__linux__)
#elif defined (__APPLE__)
#elif defined (__OpenBSD__)
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <net/if.h>
#	include <netinet/in.h>
#	include <netinet/if_ether.h>
#elif defined (WINPCAP) || defined (LIBPCAP)
#else
#error "Unknown environment"
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/memory.h"
#include "../tools/flags.h"
#include "../tools/types.h"
#include "../tools/error.h"
#include "../ether/ether.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#include "../tools/hexdecode.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PCAP_VERBOSE (1 << 0)
#define PCAP_SILENCE (1 << 1)
#define PCAP_DEVICES (1 << 2)
#define PCAP_NICS (1 << 3)
#define PCAP_MACS (1 << 4)

/*====================================================================*
 *
 *   void pcap_enum (flag_t flags);
 *
 *   pcap_enum available pcap devices on stdout;
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

void pcap_enum (flag_t flags)

{
	char report [PCAP_ERRBUF_SIZE];
	char string [ETHER_ADDR_LEN * 3];
	byte number [ETHER_ADDR_LEN];
	pcap_if_t * device;
	pcap_if_t * devices = (pcap_if_t *)(0);
	unsigned index;
	if (pcap_findalldevs (&devices, report) == -1)
	{
		error (1, 0, "Can't enumerate interfaces");
	}
	if (!devices)
	{
		error (1, 0, "No interfaces available");
	}
	if (_anyset (flags, PCAP_DEVICES))
	{
		for (device = devices, index = 1; device; device = device->next, index++)
		{
			gethwaddr (number, device->name);
			hexdecode (number, sizeof (number), string, sizeof (string));
			printf ("%2d %s %s", index, string, device->name);
			if (device->description)
			{
				printf ("\t(%s)", device->description);
			}
			printf ("\n");
		}
	}
	if (_anyset (flags, PCAP_NICS))
	{
		for (device = devices, index = 1; device; device = device->next, index++)
		{

#if defined (WIN32)

			printf ("ETH%d=%d", index, index);

#else

			printf ("ETH%d=%s", index, device->name);

#endif

			if (device->description)
			{
				printf ("\t# %s", device->description);
			}
			printf ("\n");
		}
		printf ("\n");
	}
	if (_anyset (flags, PCAP_MACS))
	{
		for (device = devices, index = 1; device; device = device->next, index++)
		{
			gethwaddr (number, device->name);
			hexdecode (number, sizeof (number), string, sizeof (string));
			printf ("NIC%d=%s", index, string);
			if (device->description)
			{
				printf ("\t# %s", device->description);
			}
			printf ("\n");
		}
		printf ("\n");
	}
	pcap_freealldevs (devices);
	return;
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
		"hqv",
		"",
		"enumerate available pcap devices on stdout",
		"h\tprint host definitions for scripting",
		"q\tquiet",
		"v\tverbose messages",
		(char const *) (0)
	};
	flag_t flags = PCAP_DEVICES;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'h':
			_clrbits (flags, (PCAP_DEVICES));
			_setbits (flags, (PCAP_NICS | PCAP_MACS));
			break;
		case 'q':
			_setbits (flags, PCAP_SILENCE);
			break;
		case 'v':
			_setbits (flags, PCAP_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc)
	{
		error (1, ECANCELED, ERROR_TOOMANY);
	}
	pcap_enum (flags);
	return (0);
}

