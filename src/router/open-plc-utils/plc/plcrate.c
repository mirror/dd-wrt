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

/*====================================================================*"
 *
 *    plcrate.c -
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/chipset.c"
#include "../plc/Devices.c"
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/Request.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../plc/LocalDevices.c"
#include "../plc/PLCSelect.c"
#include "../plc/ResetDevice.c"
#include "../plc/PhyRates2.c"
#include "../plc/Traffic2.c"
#include "../plc/NetworkTraffic2.c"
#include "../plc/Antiphon.c"
#include "../plc/Transmit.c"
#include "../plc/VersionInfo2.c"
#include "../plc/WaitForStart.c"
#endif

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/todigit.c"
#include "../tools/checkfilename.c"
#include "../tools/checksum32.c"
#include "../tools/error.c"
#include "../tools/fdchecksum32.c"
#include "../tools/synonym.c"
#include "../tools/typename.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/QualcommHeader1.c"
#include "../mme/UnwantedMessage.c"
#include "../mme/MMECode.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PLCRATE_WAIT 0
#define PLCRATE_LOOP 1

/*====================================================================*
 *
 *   void manager (struct plc * plc, signed count, signed pause);
 *
 *   perform operations in logical order despite any order specfied
 *   on the command line;
 *
 *   operation order is controlled by the order of "if" statements
 *   shown here; the entire sequence can be repeated with optional
 *   pause between each iteration;
 *
 *
 *--------------------------------------------------------------------*/

void manager (struct plc * plc, signed count, signed pause)

{
	while (count--)
	{
		if (_anyset (plc->flags, PLC_VERSION))
		{
			VersionInfo2 (plc);
		}
		if (_anyset (plc->flags, PLC_LOCAL_TRAFFIC))
		{
			Traffic2 (plc);
		}
		if (_anyset (plc->flags, PLC_NETWORK_TRAFFIC))
		{
			NetworkTraffic2 (plc);
		}
		if (_anyset (plc->flags, PLC_NETWORK))
		{
			PhyRates2 (plc);
		}
		if (_anyset (plc->flags, PLC_RESET_DEVICE))
		{
			ResetDevice (plc);
		}
		sleep (pause);
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *   parse command line, populate plc structure and perform selected
 *   operations; show help summary if asked; see getoptv and putoptv
 *   to understand command line parsing and help summary display; see
 *   plc.h for the definition of struct plc;
 *
 *   the command line accepts multiple MAC addresses and the program
 *   performs the specified operations on each address, in turn; the
 *   address order is significant but the option order is not; the
 *   default address is a local broadcast that causes all devices on
 *   the local H1 interface to respond but not those at the remote
 *   end of the powerline;
 *
 *   the default address is 00:B0:52:00:00:01; omitting the address
 *   will automatically address the local device; some options will
 *   cancel themselves if this makes no sense;
 *
 *   the default interface is eth1 because most people use eth0 as
 *   their principle network connection; you can specify another
 *   interface with -i or define environment string PLC to make
 *   that the default interface and save typing;
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"cd:ei:l:o:nqrRtTuvw:x",
		"device [device] [...]",
		"Qualcomm Atheros PLC PHY Rate Monitor",
		"c\tdisplay coded PHY rates",
		"d n\ttraffic duration is (n) seconds per leg [" LITERAL (PLC_ECHOTIME) "]",
		"e\tredirect stderr to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"l n\tloop (n) times [" LITERAL (PLCRATE_LOOP) "]",
		"n\tnetwork TX/RX information",
		"o n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMEOUT) "]",
		"q\tquiet mode",
		"r\trequest device information",
		"R\treset device with VS_RS_DEV",
		"t\tgenerate network traffic (one-to-many)",
		"T\tgenerate network traffic (many-to-many)",
		"u\tdisplay uncoded PHY rates",
		"v\tverbose mode",
		"w n\twait (n) seconds [" LITERAL (PLCRATE_WAIT) "]",
		"x\texit on error",
		(char const *) (0)
	};

#include "../plc/plc.c"

	signed loop = PLCRATE_LOOP;
	signed wait = PLCRATE_WAIT;
	signed c;
	optind = 1;
	if (getenv (PLCDEVICE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (PLCDEVICE));

#else

		channel.ifname = strdup (getenv (PLCDEVICE));

#endif

	}
	plc.timer = PLC_ECHOTIME;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'c':
			_clrbits (plc.flags, PLC_UNCODED_RATES);
			break;
		case 'd':
			plc.timer = (unsigned)(uintspec (optarg, 1, 60));
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
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
		case 'n':
			_setbits (plc.flags, PLC_NETWORK);
			break;
		case 'o':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'q':
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'r':
			_setbits (plc.flags, PLC_VERSION);
			break;
		case 'R':
			_setbits (plc.flags, PLC_RESET_DEVICE);
			break;
		case 't':
			_setbits (plc.flags, PLC_LOCAL_TRAFFIC);
			break;
		case 'T':
			_setbits (plc.flags, PLC_NETWORK_TRAFFIC);
			break;
		case 'u':
			_setbits (plc.flags, PLC_UNCODED_RATES);
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		case 'w':
			wait = (unsigned)(uintspec (optarg, 0, 3600));
			break;
		case 'x':
			_setbits (plc.flags, PLC_BAILOUT);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (_allclr (plc.flags, (PLC_VERSION | PLC_LOCAL_TRAFFIC | PLC_NETWORK_TRAFFIC | PLC_RESET_DEVICE)))
	{
		_setbits (plc.flags, PLC_NETWORK);
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		manager (&plc, loop, wait);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		manager (&plc, loop, wait);
		argv++;
		argc--;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

