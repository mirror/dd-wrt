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
 *   plcotst.c - Atheros PLC One-Time Self-Test Manager;
 *
 *   Use VS_SELFTEST_ONETIME_CONFIG and VS_SELFTEST_RESULTS message
 *   type to configure the Atheros PLC One-Time Self-Test then read
 *   and display stored selftest results;
 *
 *   Contributor(s):
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
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/Request.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../mme/UnwantedMessage.c"
#include "../plc/Devices.c"
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
#include "../tools/synonym.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../mme/MMECode.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PLCOTST_MVERSION 0
#define PLCOTST_RUNAFTERRESET 1
#define PLCOTST_DELAYTORUN 0
#define PLCOTST_MEMRUNS 0
#define PLCOTST_FLASHRUNS 0
#define PLCOTST_RESETONDONE 0

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push,1)
#endif

typedef struct __packed selftest

{
	uint8_t MVERSION;
	uint8_t RUNAFTERRESET;
	uint32_t DELAYTORUN;
	uint32_t MEMRUNS;
	uint32_t FLASHRUNS;
	uint8_t RESETONDONE;
}

selftest;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *
 *   signed configure (struct plc * plc, struct selftest * selftest);
 *
 *
 *--------------------------------------------------------------------*/

static signed configure (struct plc * plc, struct selftest * selftest)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_selftest_onetime_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MVERSION;
		uint8_t RUNAFTERRESET;
		uint32_t DELAYTORUN;
		uint32_t MEMRUNS;
		uint32_t FLASHRUNS;
		uint8_t RESETONDONE;
	}
	* request = (struct vs_selftest_onetime_config_request *) (message);
	struct __packed vs_selftest_onetime_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
	}
	* confirm = (struct vs_selftest_onetime_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Configure One-Time Self-Test");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_SELFTEST_ONETIME_CONFIG | MMTYPE_REQ));
	request->MVERSION = selftest->MVERSION;
	request->RUNAFTERRESET = selftest->RUNAFTERRESET;
	request->DELAYTORUN = HTOLE32 (selftest->DELAYTORUN);
	request->MEMRUNS = HTOLE32 (selftest->MEMRUNS);
	request->FLASHRUNS = HTOLE32 (selftest->FLASHRUNS);
	request->RESETONDONE = selftest->RESETONDONE;
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_SELFTEST_ONETIME_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
		Display (plc, "%s", "Configured");
	}
	return (0);
}


/*====================================================================*
 *
 *   signed retrieve (struct plc * plc, struct selftest * selftest);
 *
 *
 *--------------------------------------------------------------------*/

static signed retrieve (struct plc * plc, struct selftest * selftest)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_selftest_results_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MVERSION;
		uint8_t MACTION;
	}
	* request = (struct vs_selftest_results_request *) (message);
	struct __packed vs_selftest_results_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MVERSION;
		uint8_t MSTATUS;
		uint32_t NUMBER [6];
	}
	* confirm = (struct vs_selftest_results_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Retrieve Self-Test Results");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_SELFTEST_RESULTS | MMTYPE_REQ));
	request->MVERSION = selftest->MVERSION;
	request->MACTION = plc->action;
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_SELFTEST_RESULTS | MMTYPE_CNF)) > 0)
	{
		if ((confirm->MSTATUS == 2) || (confirm->MSTATUS == 4) || (confirm->MSTATUS == 6))
		{
			Display (plc, "Memory test %d Passed %d Failed %d  Flash test %d Passed %d Failed %d", LE32TOH (confirm->NUMBER [0]), LE32TOH (confirm->NUMBER [1]), LE32TOH (confirm->NUMBER [2]), LE32TOH (confirm->NUMBER [3]), LE32TOH (confirm->NUMBER [4]), LE32TOH (confirm->NUMBER [5]));
			continue;
		}
		if (confirm->MSTATUS == 3)
		{
			Failure (plc, "%d seconds remaining", LE32TOH (confirm->NUMBER [0]));
			continue;
		}
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
		Confirm (plc, "Cleared Results");
	}
	return (0);
}


/*====================================================================*
 *
 *   signed manager (struct plc * plc, struct selftest * selftest);
 *
 *
 *--------------------------------------------------------------------*/

static signed manager (struct plc * plc, struct selftest * selftest)

{
	if (_anyset (plc->flags, PLC_CONFIGURE))
	{
		configure (plc, selftest);
	}
	if (_anyset (plc->flags, PLC_RESULTS))
	{
		retrieve (plc, selftest);
	}
	return (0);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"c:Cd:ef:i:m:qr:R:t:vx",
		"device [device] [...] [> stdout]",
		"Qualcomm Atheros Powerline One-Time Self-Test Manager",
		"c n\tconfiguration version",
		"C\tclear selftest results",
		"d n\tdelay selftest start by (n) seconds [" LITERAL (PLCOTST_DELAYTORUN) "]",
		"e\tredirect stderr to stdout",
		"f n\trun flash selftest (n) times [" LITERAL (PLCOTST_FLASHRUNS) "]",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"m n\trun memory selftest (n) times [" LITERAL (PLCOTST_MEMRUNS) "]",
		"q\tquiet mode",
		"r n\trun selftest after reset [" LITERAL (PLCOTST_RUNAFTERRESET) "]",
		"R n\treset after selftest completes [" LITERAL (PLCOTST_RESETONDONE) "]",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMEOUT) "]",
		"v\tverbose mode",
		"x\texit on error",
		(char const *) (0)
	};

#include "../plc/plc.c"

	struct selftest selftest =
	{
		PLCOTST_MVERSION,
		PLCOTST_RUNAFTERRESET,
		PLCOTST_DELAYTORUN,
		PLCOTST_MEMRUNS,
		PLCOTST_FLASHRUNS,
		PLCOTST_RESETONDONE
	};
	signed c;
	if (getenv (PLCDEVICE))
	{
		channel.ifname = strdup (getenv (PLCDEVICE));
	}
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'c':
			selftest.MVERSION = (uint8_t)(uintspec (optarg, 0, UCHAR_MAX));
			break;
		case 'C':
			_setbits (plc.flags, PLC_RESULTS);
			plc.action = 1;
			break;
		case 'd':
			_setbits (plc.flags, PLC_CONFIGURE);
			selftest.DELAYTORUN = (uint32_t)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'f':
			_setbits (plc.flags, PLC_CONFIGURE);
			selftest.FLASHRUNS = (uint32_t)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'm':
			_setbits (plc.flags, PLC_CONFIGURE);
			selftest.MEMRUNS = (uint32_t)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'r':
			_setbits (plc.flags, PLC_CONFIGURE);
			selftest.RUNAFTERRESET = (uint8_t)(uintspec (optarg, false, true));
			break;
		case 'R':
			_setbits (plc.flags, PLC_CONFIGURE);
			selftest.RESETONDONE = (uint8_t)(uintspec (optarg, false, true));
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
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
	if (_allclr (plc.flags, (PLC_CONFIGURE | PLC_RESULTS)))
	{
		_setbits (plc.flags, PLC_RESULTS);
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		manager (&plc, &selftest);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		manager (&plc, &selftest);
		argv++;
		argc--;
	}
	free (plc.message);
	closechannel (&channel);
	return (0);
}

