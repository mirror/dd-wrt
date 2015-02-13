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
 *   efsu.c - Ethernet Frame Send Utility;
 *
 *   convert hexadecimal text files to ethernet frames and transmit
 *   them over the network; basically, it is a 'send your own frame'
 *   utility;
 *
 *   the program works like cat, sending file after file to a given
 *   interface; as each file is read, all hexadecimal octets in the
 *   file are converted to bytes and buffered; a semicolon causes a
 *   buffer transmit as does the end of file; script-style comments
 *   starting with hash (#) and c-language-style comments starting
 *   with slash-slash or slash-asterisk are consumed and discard as
 *   the file is read; the errors that can occur are non-hex digits
 *   and odd number of hex digits;
 *
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <limits.h>

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
#include "../tools/hexencode.c"
#include "../tools/hexload.c"
#include "../tools/hexdump.c"
#include "../tools/todigit.c"
#include "../tools/uintspec.c"
#include "../tools/basespec.c"
#include "../tools/synonym.c"
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

#define EFSU_INTERFACE "PLC"
#define EFSU_ETHERTYPE 0x88E1
#define EFSU_PAUSE 0
#define EFSU_DELAY 0
#define EFSU_LOOP 1

#ifndef ETHER_CRC_LEN
#define ETHER_CRC_LEN 4
#endif

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

static const struct _term_ protocols [] =

{
	{
		"hp10",
		"887B"
	},
	{
		"hpav",
		"88E1"
	}
};


/*====================================================================*
 *
 *   void function (struct channel * channel, void * memory, ssize_t extent);
 *
 *   read Ethernet frame descriptions from stdin and transmit them
 *   as raw ethernet frames; wait for a response if CHANNEL_LISTEN
 *   flagword bit is set;
 *
 *
 *--------------------------------------------------------------------*/

static void function (struct channel * channel, void * memory, ssize_t extent)

{
	struct ether_header * frame = (struct ether_header *)(memory);
	unsigned length;
	while ((length = (unsigned)(hexload (memory, extent, stdin))) > 0)
	{
		if (length < (ETHER_MIN_LEN - ETHER_CRC_LEN))
		{
			error (1, ENOTSUP, "Frame size of %d is less than %d bytes", length, (ETHER_MIN_LEN - ETHER_CRC_LEN));
		}
		if (length > (ETHER_MAX_LEN - ETHER_CRC_LEN))
		{
			error (1, ENOTSUP, "Frame size of %d is more than %d bytes", length, (ETHER_MAX_LEN - ETHER_CRC_LEN));
		}
		if (_anyset (channel->flags, CHANNEL_UPDATE_TARGET))
		{
			memcpy (frame->ether_dhost, channel->peer, sizeof (frame->ether_dhost));
		}
		if (_anyset (channel->flags, CHANNEL_UPDATE_SOURCE))
		{
			memcpy (frame->ether_shost, channel->host, sizeof (frame->ether_shost));
		}
		sendpacket (channel, memory, length);
		if (_anyset (channel->flags, CHANNEL_LISTEN))
		{
			while (readpacket (channel, memory, extent) > 0);
		}
	}
	return;
}


/*====================================================================*
 *
 *   void iterate (int argc, char const * argv [], void * memory, ssize_t extent, unsigned pause);
 *
 *
 *
 *--------------------------------------------------------------------*/

static void iterate (int argc, char const * argv [], struct channel * channel, unsigned pause)

{
	byte buffer [ETHER_MAX_LEN];
	if (!argc)
	{
		function (channel, buffer, sizeof (buffer));
	}
	while ((argc) && (* argv))
	{
		if (!freopen (* argv, "rb", stdin))
		{
			error (1, errno, "Can't open %s", * argv);
		}
		function (channel, buffer, sizeof (buffer));
		argc--;
		argv++;
		if ((argc) && (* argv))
		{
			sleep (pause);
		}
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
		"d:e:hi:l:p:t:vw:",
		PUTOPTV_S_FUNNEL,
		"Ethernet Frame Send Utility",
		"d x\treplace destination address with (x)",
		"e x\techo return frames having ethertype (x) [" LITERAL (EFSU_ETHERTYPE) "]",
		"h\treplace source address with host address",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"l n\trepeat file sequence (n) times [" LITERAL (EFSU_LOOP) "]",
		"p n\twait (n) seconds between files [" LITERAL (EFSU_PAUSE) "]",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMEOUT) "]",
		"v\tverbose messages",
		"w n\twait (n) seconds between loops [" LITERAL (EFSU_DELAY) "]",
		(char const *) (0)
	};
	unsigned pause = EFSU_PAUSE;
	unsigned delay = EFSU_DELAY;
	unsigned loop = EFSU_LOOP;
	signed c;
	channel.type = EFSU_ETHERTYPE;
	if (getenv (EFSU_INTERFACE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (EFSU_INTERFACE));

#else

		channel.ifname = strdup (getenv (EFSU_INTERFACE));

#endif

	}
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'd':
			_setbits (channel.flags, CHANNEL_UPDATE_TARGET);
			if (!hexencode (channel.peer, sizeof (channel.peer), optarg))
			{
				error (1, errno, "%s", optarg);
			}
			break;
		case 'e':
			_setbits (channel.flags, CHANNEL_LISTEN);
			channel.type = (uint16_t)(basespec (synonym (optarg, protocols, SIZEOF (protocols)), 16, sizeof (channel.type)));
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
		case 'l':
			loop = (unsigned)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'p':
			pause = (unsigned)(uintspec (optarg, 0, 1200));
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			break;
		case 'w':
			delay = (unsigned)(uintspec (optarg, 0, 1200));
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	openchannel (&channel);
	while (loop--)
	{
		iterate (argc, argv, &channel, pause);
		if (loop)
		{
			sleep (delay);
		}
	}
	closechannel (&channel);
	return (0);
}

