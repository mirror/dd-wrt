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
 *   efbu.c - Ethernet Frame Blast Utility;
 *
 *   transmit an IP broadcast frame of maximum length at maximum rate
 *   for a given period of time in seconds;
 *
 *
 *--------------------------------------------------------------------*/


/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/error.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/flags.h"
#include "../ether/channel.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#include "../tools/basespec.c"
#include "../tools/uintspec.c"
#include "../tools/hexencode.c"
#include "../tools/hexdump.c"
#include "../tools/todigit.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

/*====================================================================*
 *   program contants;
 *--------------------------------------------------------------------*/

#define EFBU_VLAN_TAG 0
#define EFBU_INTERFACE "PLC"
#define EFBU_ETHERTYPE ETH_P_802_2
#define EFBU_BINARY 0xAA
#define EFBU_TIMER 1000
#define EFBU_PAUSE 50

#ifndef ETHER_CRC_LEN
#define ETHER_CRC_LEN 4
#endif

#if defined (WIN32)
#define SLEEP(milliseconds) Sleep(milliseconds)
#else
#define SLEEP(milliseconds) usleep(1000*(milliseconds))
#endif

/*====================================================================*
 *
 *   void function (struct channel * channel, void * memory, unsigned extent, unsigned timer, unsigned pause);
 *
 *   transmit an IP broadcast frame of given length at maximum rate
 *   for a given period of time in seconds;
 *
 *
 *--------------------------------------------------------------------*/

static void function (struct channel * channel, void * memory, ssize_t extent, byte binary, unsigned timer, unsigned pause)

{

#if EFBU_VLAN_TAG

	struct ether_header
	{
		uint8_t ether_dhost [ETH_ALEN];
		uint8_t ether_shost [ETH_ALEN];
		uint32_t ether_vlan;
		uint16_t ether_type;
	}
	__attribute__ ((__packed__)) * frame = (struct ether_header *)(memory);

#else

	struct ether_header * frame = (struct ether_header *)(memory);

#endif

	struct timeval ts;
	struct timeval tc;
	unsigned since;
	memset (memory, binary, extent);
	if (extent > (ETHER_MAX_LEN - ETHER_CRC_LEN))
	{
		extent = ETHER_MAX_LEN - ETHER_CRC_LEN;
	}
	memcpy (frame->ether_dhost, channel->peer, sizeof (frame->ether_dhost));
	memcpy (frame->ether_shost, channel->host, sizeof (frame->ether_shost));

#if EFBU_VLAN_TAG

	frame->ether_vlan = htonl (0x8100A000);

#endif

	frame->ether_type = htons (channel->type);
	if (gettimeofday (&ts, NULL) == -1)
	{
		error (1, errno, CANT_START_TIMER);
	}
	for (since = 0; since < timer; since = (tc.tv_sec - ts.tv_sec) * 1000 + ((tc.tv_usec - ts.tv_usec) / 1000))
	{
		sendpacket (channel, memory, extent);
		if (gettimeofday (&tc, NULL) == -1)
		{
			error (1, errno, CANT_RESET_TIMER);
		}
		SLEEP (pause);
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
	extern struct channel channel;
	static char const * optv [] =
	{
		"b:d:e:hi:p:t:v",
		"",
		"Ethernet Frame Blast Utility",
		"b n\tbinary byte value is (n) [" LITERAL (EFBU_BINARY) "]",
		"d x\treplace destination address with (x)",
		"e x\tethertype is (x) [" LITERAL (EFBU_ETHERTYPE) "]",
		"h\treplace source address with host address",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"t n\ttransmit for (n) milliseconds [" LITERAL (EFBU_TIMER) "]",
		"p n\tpause (n) milliseconds between frames [" LITERAL (EFBU_PAUSE) "]",
		"v\tverbose messages",
		(char const *) (0)
	};
	byte buffer [ETHER_MAX_LEN];
	byte binary = EFBU_BINARY;
	unsigned timer = EFBU_TIMER;
	unsigned pause = EFBU_PAUSE;
	signed c;
	if (getenv (EFBU_INTERFACE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (EFBU_INTERFACE));

#else

		channel.ifname = strdup (getenv (EFBU_INTERFACE));

#endif

	}
	optind = 1;
	memset (channel.peer, 0xFF, sizeof (channel.peer));
	memset (channel.host, 0xFF, sizeof (channel.host));
	channel.type = EFBU_ETHERTYPE;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'b':
			binary = (uint8_t)(uintspec (optarg, 0, 255));
			break;
		case 'd':
			_setbits (channel.flags, CHANNEL_UPDATE_TARGET);
			if (!hexencode (channel.peer, sizeof (channel.peer), optarg))
			{
				error (1, errno, "%s", optarg);
			}
			break;
		case 'e':
			channel.type = (uint16_t)(basespec (optarg, 16, sizeof (channel.type)));
			break;
		case 'h':
			_setbits (channel.flags, CHANNEL_UPDATE_SOURCE);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'p':
			pause = (unsigned)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			break;
		case 't':
			timer = (unsigned)(uintspec (optarg, 0, UINT_MAX));
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
		error (1, ECANCELED, ERROR_TOOMANY);
	}
	openchannel (&channel);
	function (&channel, buffer, sizeof (buffer), binary, timer, pause);
	closechannel (&channel);
	return (0);
}

