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
 *   edsu.c - Qualcomm Atheros Ethernet II Data Send Utility
 *
 *   send one or more files over Ethernet using IEEE 802.2 Ethernet
 *   Frames;
 *
 *   this program can be used as a data source when testing AR6405
 *   UART applications; use this program to send files and program
 *   edru to read and display or save them at the other end;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <signal.h>
#include <unistd.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/flags.h"
#include "../ether/ether.h"
#include "../ether/channel.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/efreopen.c"
#include "../tools/basespec.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define EDSU_INTERFACE "PLC"
#define EDSU_ETHERTYPE ETH_P_802_2
#define EDSU_PAUSE 0

/*====================================================================*
 *
 *   signed function (struct channel * channel, unsigned pause, signed fd);
 *
 *   read a file and transmit it over network as a stream of Ethernet
 *   frames; pause between frames to prevent over-loading the remote
 *   host;
 *
 *
 *--------------------------------------------------------------------*/

signed function (struct channel * channel, unsigned pause, signed fd)

{
	struct ethernet_frame frame;
	signed length = sizeof (frame.frame_data);
	memcpy (frame.frame_dhost, channel->peer, sizeof (frame.frame_dhost));
	memcpy (frame.frame_shost, channel->host, sizeof (frame.frame_shost));
	while ((length = read (fd, frame.frame_data, sizeof (frame.frame_data))) > 0)
	{
		frame.frame_type = htons (length);
		if (length < ETHERMIN)
		{
			length = ETHERMIN;
		}
		length += ETHER_HDR_LEN;
		if (sendpacket (channel, &frame, length) < 0)
		{
			error (1, errno, CHANNEL_CANTSEND);
		}
		sleep (pause);
	}
	return (0);
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
		"e:d:i:p:qv",
		"file [file] [...]",
		"Qualcomm Atheros Ethernet II Data Send Utility",
		"e x\tethertype is (x) [" LITERAL (EDSU_ETHERTYPE) "]",
		"d x\tdestination address is (x) [00:B0:52:00:00:01]",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"p n\tpause (n) seconds between frames [" LITERAL (EDSU_PAUSE) "]",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *)(0)
	};
	unsigned pause = EDSU_PAUSE;
	signed c;
	if (getenv (EDSU_INTERFACE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (EDSU_INTERFACE));

#else

		channel.ifname = strdup (getenv (EDSU_INTERFACE));

#endif

	}
	channel.type = EDSU_ETHERTYPE;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'e':
			channel.type = (uint16_t)(basespec (optarg, 16, sizeof (channel.type)));
			break;
		case 'd':
			if (!hexencode (channel.peer, sizeof (channel.peer), optarg))
			{
				error (1, errno, "%s", optarg);
			}
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'p':
			pause = (unsigned)(uintspec (optarg, 0, UCHAR_MAX));
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
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
	openchannel (&channel);
	if (!argc)
	{
		function (&channel, pause, STDIN_FILENO);
	}
	while ((argc) && (* argv))
	{
		if (efreopen (* argv, "rb", stdin))
		{
			function (&channel, pause, fileno (stdin));
		}
		argc--;
		argv++;
	}
	closechannel (&channel);
	exit (0);
}

