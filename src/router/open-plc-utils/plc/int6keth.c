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
 *   int6keth.c - Atheros Ethernet PHY Settings;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

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
#include "../ether/channel.h"
#include "../mme/mme.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../mme/UnwantedMessage.c"
#include "../plc/Devices.c"
#endif

#ifndef MAKEFILE
#include "../tools/assist.c"
#include "../tools/error.c"
#include "../tools/codelist.c"
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/checkfilename.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/strfbits.c"
#include "../tools/typename.c"
#include "../tools/lookup.c"
#include "../tools/synonym.c"
#include "../tools/uintspec.c"
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

#define MCONTROL_READ 	0x00
#define MCONTROL_WRITE 	0x01
#define ETH_PORT 0

#define NEGOTIATE 	SIZEOF (negotiate)
#define SPEEDS  	SIZEOF (speeds)
#define DUPLEX 		SIZEOF (duplex)
#define CONTROL 	SIZEOF (control)
#define ADVCAP 		SIZEOF (advcap)

static struct _code_ const negotiate [] =

{
	{
		0,
		"Off"
	},
	{
		1,
		"On"
	}
};

static struct _code_ const speeds [] =

{
	{
		0,
		"10"
	},
	{
		1,
		"100"
	},
	{
		2,
		"1000"
	}
};

static struct _code_ const duplex [] =

{
	{
		0,
		"Half"
	},
	{
		1,
		"Full"
	}
};

static struct _code_ const control [] =

{
	{
		0,
		"Off"
	},
	{
		1,
		"Tx"
	},
	{
		2,
		"Rx"
	},
	{
		3,
		"On"
	}
};

static struct _code_ const advcap [] =

{
	{
		1,
		"100Full"
	},
	{
		2,
		"100Half"
	},
	{
		4,
		"10Full"
	},
	{
		8,
		"10Half"
	},
	{
		16,
		"1000Full"
	}
};


#define RATES SIZEOF (rates)
#define MODES SIZEOF (modes)
#define LINKS SIZEOF (links)
#define FLOWS SIZEOF (flows)

static char const * rates [] =

{
	"10",
	"100",
	"1000"
};

static char const * modes [] =

{
	"Half",
	"Full"
};

static char const * links [] =

{
	"Unknown",
	"Off",
	"On"
};

static char const * flows [] =

{
	"Off",
	"Tx",
	"Rx",
	"On"
};


/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push,1)
#endif

typedef struct __packed phy_settings

{
	uint8_t MCONTROL;
	uint8_t AUTONEGOTIATE;
	uint8_t ADVCAPS;
	uint8_t ESPEED;
	uint8_t EDUPLEX;
	uint8_t EFLOWCONTROL;
}

phy_settings;
typedef struct __packed phy_readings

{
	uint8_t MSTATUS;
	uint8_t ESPEED;
	uint8_t EDUPLEX;
	uint8_t ELINKSTATUS;
	uint8_t EFLOWCONTROL;
}

phy_readings;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *
 *   signed PHYSettings (struct channel * channel, struct phy_settings * settings, flag_t flags);
 *
 *   plc.h
 *
 *   read and display Ethernet PHY settings or write then read and
 *   display settings;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed PHYSettings (struct channel * channel, struct phy_settings * settings, flag_t flags)

{
	struct message message;
	signed packetsize;

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_enet_settings_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MCONTROL;
		uint8_t AUTONEGOTIATE;
		uint8_t ADVCAPS;
		uint8_t ESPEED;
		uint8_t EDUPLEX;
		uint8_t EFLOWCONTROL;
	}
	* request = (struct vs_enet_settings_request *) (&message);
	struct __packed vs_enet_settings_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t ESPEED;
		uint8_t EDUPLEX;
		uint8_t ELINKSTATUS;
		uint8_t EFLOWCONTROL;
	}
	* confirm = (struct vs_enet_settings_confirm *) (&message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	char address [ETHER_ADDR_LEN * 3];
	memset (&message, 0, sizeof (message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_ENET_SETTINGS | MMTYPE_REQ));
	request->MCONTROL = settings->MCONTROL;
	request->AUTONEGOTIATE = settings->AUTONEGOTIATE;
	request->ADVCAPS = settings->ADVCAPS;
	request->ESPEED = settings->ESPEED;
	request->EDUPLEX = settings->EDUPLEX;
	request->EFLOWCONTROL = settings->EFLOWCONTROL;
	if (sendpacket (channel, &message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) < 0)
	{
		error (1, errno, CHANNEL_CANTSEND);
	}
	while ((packetsize = readpacket (channel, &message, sizeof (message))) > 0)
	{
		if (UnwantedMessage (&message, packetsize, 0, (VS_ENET_SETTINGS | MMTYPE_CNF)))
		{
			continue;
		}
		if ((confirm->MSTATUS == 1) || (confirm->MSTATUS == 3))
		{
			error (0, 0, "%s: %s (0x%0X): ", PLC_WONTDOIT, MMECode (confirm->qualcomm.MMTYPE, confirm->MSTATUS), confirm->MSTATUS);
			continue;
		}
		if (_anyset (flags, PLC_ANALYSE))
		{
			printf ("Bits Mode Link Flow\n");
			printf ("%4d ", confirm->ESPEED);
			printf ("%4d ", confirm->EDUPLEX);
			printf ("%4d ", confirm->ELINKSTATUS);
			printf ("%4d\n", confirm->EFLOWCONTROL);
		}
		else
		{
			printf ("%s %s ", channel->ifname, hexstring (address, sizeof (address), channel->host, sizeof (channel->host)));
			printf ("Speed=%s ", rates [confirm->ESPEED]);
			printf ("Duplex=%s ", modes [confirm->EDUPLEX]);
			printf ("LinkStatus=%s ", links [confirm->ELINKSTATUS]);
			printf ("FlowControl=%s\n", flows [confirm->EFLOWCONTROL]);
		}
	}
	if (packetsize < 0)
	{
		error (1, errno, CHANNEL_CANTREAD);
	}
	return (0);
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
		"a:d:ef:i:n:p:qrs:tvw",
		"device [device] [...] [> stdout]",
		"Qualcomm Atheros Ethernet PHY Settings",
		"a s\tadvertise capabilities as (s) ['1000Full'|'100Full'|'100Half'|10Full'|'10Half']",
		"d s\tduplex setting is (s) ['half'|'full']",
		"e\tredirect stderr to stdout",
		"f s\tflow control is (s) ['on'|'tx'|'rx'|'off']",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"n s\tauto-negotiate mode is (s) ['on'|'off']",
		"p n\tport number is (n) [" LITERAL (ETH_PORT) "]",
		"q\tquiet mode",
		"r\tread settings instead of write settings",
		"s s\ttransmission speed in mbps is (s) ['10'|'100'|'1000']",
		"v\tverbose mode",
		"w\twrite settings instead of read settings",
		(char const *) (0)
	};
	struct phy_settings settings =
	{
		0,
		1,
		0,
		0,
		0,
		0
	};
	flag_t flags = (flag_t)(0);
	signed c;
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
		case 'a':
			if ((c = lookup (optarg, advcap, ADVCAP)) == -1)
			{
				assist (optarg, "capability", advcap, ADVCAP);
			}
			settings.ADVCAPS |= (uint8_t)(c);
			break;
		case 'd':
			if ((c = lookup (optarg, duplex, DUPLEX)) == -1)
			{
				assist (optarg, "duplex", duplex, DUPLEX);
			}
			settings.EDUPLEX = (uint8_t)(c);
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'f':
			if ((c = lookup (optarg, control, CONTROL)) == -1)
			{
				assist (optarg, "control", control, CONTROL);
			}
			settings.EFLOWCONTROL = (uint8_t)(c);
			break;
		case 'n':
			if ((c = lookup (optarg, negotiate, NEGOTIATE)) == -1)
			{
				assist (optarg, "auto-negotiate", negotiate, NEGOTIATE);
			}
			settings.AUTONEGOTIATE = (uint8_t)(c);
			break;
		case 's':
			if ((c = lookup (optarg, speeds, SPEEDS)) == -1)
			{
				assist (optarg, "speed", speeds, SPEEDS);
			}
			settings.ESPEED = (uint8_t)(c);
			break;
		case 't':
			_setbits (flags, PLC_ANALYSE);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'p':
			settings.MCONTROL &= 0x0F;
			settings.MCONTROL |= (unsigned)(uintspec (optarg, 0, 7)) << 4;
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			break;
		case 'r':
			settings.MCONTROL &= 0xF0;
			settings.MCONTROL |= 0x00;
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			break;
		case 'w':
			settings.MCONTROL &= 0xF0;
			settings.MCONTROL |= 0x01;
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
		PHYSettings (&channel, &settings, flags);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		PHYSettings (&channel, &settings, flags);
		argc--;
		argv++;
	}
	closechannel (&channel);
	exit (0);
}

