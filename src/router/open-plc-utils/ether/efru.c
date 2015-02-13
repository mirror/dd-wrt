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
 *   efru.c - Ethernet Frame Read Utility
 *
 *   print message header and/or full message content on stdout for
 *   each HomePlugAV or Atheros Vendor Specific message received by
 *   the host;
 *
 *   this program receives raw ethernet frames and so requires root
 *   privileges; if you install it using "chmod 555" and "chown
 *   root:root" then you must login as root to run it; otherwise, you
 *   can install it using "chmod 4555" and "chown root:root" so that
 *   anyone can run it; the program will refuse to run until you get
 *   thing right;
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __linux__
#include <net/if.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <signal.h>
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../ether/channel.h"
#include "../plc/plc.h"
#include "../mme/mme.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/hexdump.c"
#include "../tools/basespec.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/sendpacket.c"
#include "../ether/readpacket.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define EFRU_VERBOSE (1 << 0)
#define EFRU_SILENCE (1 << 1)

#define EFRU_INTERFACE "PLC"
#define EFRU_ETHERTYPE ETH_P_802_2

/*====================================================================*
 *
 *   int main (int argc, char * argv[]);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	struct message message;
	static char const * optv [] =
	{
		"e:i:qt:v",
		PUTOPTV_S_DIVINE,
		"Qualcomm Atheros Ethernet Frame Read Utility",
		"e x\tethertype is (x) [" LITERAL (EFRU_ETHERTYPE) "]",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (s) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (n) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"q\tsuppress normal output",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_FOREVER) "]",
		"v\tverbose messages on stdout",
		(char const *) (0)
	};
	flag_t flags = (flag_t)(0);
	signed length;
	signed c;
	if (getenv (EFRU_INTERFACE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (EFRU_INTERFACE));

#else

		channel.ifname = strdup (getenv (EFRU_INTERFACE));

#endif

	}
	optind = 1;
	channel.type = EFRU_ETHERTYPE;
	channel.timeout = CHANNEL_FOREVER;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'e':
			channel.type = (uint16_t)(basespec (optarg, 16, sizeof (channel.type)));
			break;
		case 'i':

#if defined (WIN32)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'q':
			_setbits (flags, EFRU_SILENCE);
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (flags, EFRU_VERBOSE);
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
	openchannel (&channel);
	while ((length = readpacket (&channel, &message, sizeof (message))) >= 0)
	{
		hexdump (&message, 0, length, stdout);
	}
	closechannel (&channel);
	return (0);
}

