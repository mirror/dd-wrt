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
 *   int6krule.c - Qualcomm Atheros Message MakeRule Utility;
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
#include "../plc/rules.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/basespec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/bytespec.c"
#include "../tools/ipv4spec.c"
#include "../tools/ipv6spec.c"
#include "../tools/uintspec.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#include "../tools/synonym.c"
#include "../tools/assist.c"
#include "../tools/lookup.c"
#include "../tools/codelist.c"
#include "../tools/memout.c"
#include "../tools/reword.c"
#endif

#ifndef MAKEFILE
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/ReadMME.c"
#include "../plc/Request.c"
#include "../plc/SendMME.c"
#include "../plc/Devices.c"
#include "../plc/rules.c"
#include "../plc/ParseRule.c"
#include "../plc/PrintRule.c"
#include "../plc/MakeRule.c"
#include "../plc/ReadRules.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#endif

#ifndef MAKEFILE
#include "../mme/MMECode.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define INT6KRULE_CSPEC_VERSION 0
#define INT6KRULE_VLAN_TAG 0x00000000
#define COMMA ","
#define QUOTE "''"

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
		"ei:qrst:T:vV:",
		"action operand condition [...] control volatility [device] [...]\n\n          where a condition is: field operator value",
		"Qualcomm Atheros Stream MakeRule Utility",
		"e\tredirect stderr to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"q\tquiet mode",
		"r\tread rules from device",
		"s\tdisplay symbol tables",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMEOUT) "]",
		"T x\tinserted vlan tag is x [" LITERAL (INT6KRULE_VLAN_TAG) "]",
		"v\tverbose mode",
		"V n\tcspec version is n [" LITERAL (INT6KRULE_CSPEC_VERSION) "]",
		(char const *) (0)
	};

#include "../plc/plc.c"

	struct cspec cspec;
	struct MMERule rule;
	signed c;
	memset (& rule, 0, sizeof (rule));
	memset (& cspec, 0, sizeof (cspec));
	cspec.VLAN_TAG = INT6KRULE_VLAN_TAG;
	cspec.CSPEC_VERSION = INT6KRULE_CSPEC_VERSION;
	if (getenv (PLCDEVICE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (PLCDEVICE));

#else

		channel.ifname = strdup (getenv (PLCDEVICE));

#endif

	}
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
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
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'r':
			_setbits (plc.flags, PLC_ANALYSE);
			break;
		case 's':
			printf ("\n");
			printf (" Controls are ");
			codelist (controls, SIZEOF (controls), COMMA, QUOTE, stdout);
			printf (".\n");
			printf (" Volatilities are ");
			codelist (volatilities, SIZEOF (volatilities), COMMA, QUOTE, stdout);
			printf (".\n");
			printf (" Actions are ");
			codelist (actions, SIZEOF (actions), COMMA, QUOTE, stdout);
			printf (".\n");
			printf (" Operands are ");
			codelist (operands, SIZEOF (operands), COMMA, QUOTE, stdout);
			printf (".\n");
			printf (" Fields are ");
			codelist (fields, SIZEOF (fields), COMMA, QUOTE, stdout);
			printf (".\n");
			printf (" Operators are ");
			codelist (operators, SIZEOF (operators), COMMA, QUOTE, stdout);
			printf (".\n");
			printf (" States are ");
			codelist (states, SIZEOF (states), COMMA, QUOTE, stdout);
			printf (".\n");
			printf ("\n");
			return (0);
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'T':
			cspec.VLAN_TAG = (uint32_t)(basespec (optarg, 16, sizeof (cspec.VLAN_TAG)));
			cspec.VLAN_TAG = htonl (cspec.VLAN_TAG);
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		case 'V':
			cspec.CSPEC_VERSION = (uint16_t)(basespec (optarg, 10, sizeof (cspec.CSPEC_VERSION)));
			cspec.CSPEC_VERSION = HTOLE16 (cspec.CSPEC_VERSION);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (_allclr (plc.flags, PLC_ANALYSE))
	{
		if (ParseRule (& argc, & argv, & rule, & cspec) == -1)
		{
			error (1, 0, "invalid rule");
		}
	}
	openchannel (& channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		if (_anyset (plc.flags, PLC_ANALYSE))
		{
			ReadRules (& plc);
		}
		else
		{
			MakeRule (& plc, & rule);
		}
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		if (_anyset (plc.flags, PLC_ANALYSE))
		{
			ReadRules (& plc);
		}
		else
		{
			MakeRule (& plc, & rule);
		}
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (& channel);
	exit (0);
}

