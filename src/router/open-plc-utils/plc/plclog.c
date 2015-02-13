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
 *   plclog.c - Atheros INT6x00 Log Retrieval Utility;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/format.h"
#include "../tools/base64.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../ether/channel.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/Devices.c"
#include "../plc/Failure.c"
#include "../plc/SendMME.c"
#include "../plc/ReadMME.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#endif

#ifndef MAKEFILE
#include "../tools/error.c"
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/strfbits.c"
#include "../tools/synonym.c"
#include "../tools/b64dump.c"
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
#endif

/*====================================================================*
 *   constants
 *--------------------------------------------------------------------*/

#define MSTATUS_STATUS (7 << 0)
#define MSTATUS_MAJORVERSION (1 << 3)
#define MSTATUS_BUFFERISLOCKED (1 << 4)
#define MSTATUS_AUTOLOCKONRESET (1 << 5)
#define MSTATUS_UNSOLICITEDUPDATES (1 << 6)
#define MSTATUS_UNSOLICITED (1 << 7)

#define WD_SESSION_ID 0xFEFE
#define WD_ACTION_READ   (1 << 0)
#define WD_ACTION_CLEAR  (1 << 1)
#define WD_ACTION_CUSTOM (1 << 2)

/*====================================================================*
 *
 *   static signed PrintRawWatchdogReport (struct plc * plc);
 *
 *   Read the watchdog report using VS_WD_RPT and write to file in
 *   binary or XML format; this file can be sent to Atheros Support
 *   for analysis;
 *
 *   The XML version rquires additional information and so VS_SW_VER
 *   is used to collect it;
 *
 *   This VW_WD_RPT message protocol returns an indication message,
 *   not a confirm message;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed PrintRawWatchdogReport (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_wd_rpt_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint16_t SESSIONID;
		uint8_t CLR;
	}
	* request = (struct vs_wd_rpt_request *) (message);
	struct __packed vs_wd_rpt_ind
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint16_t SESSIONID;
		uint8_t NUMPARTS;
		uint8_t CURPART;
		uint16_t RDATALENGTH;
		uint8_t RDATAOFFSET;
		uint8_t RDATA [1];
	}
	* indicate = (struct vs_wd_rpt_ind *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_WD_RPT | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	request->SESSIONID = HTOLE16 (WD_SESSION_ID);
	request->CLR = plc->readaction;
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	do
	{
		if (ReadMME (plc, 0, (VS_WD_RPT | MMTYPE_IND)) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (indicate->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		write (STDOUT_FILENO, indicate->RDATA + indicate->RDATAOFFSET, LE16TOH (indicate->RDATALENGTH) - indicate->RDATAOFFSET);
	}
	while (indicate->CURPART < indicate->NUMPARTS);
	return (0);
}


/*====================================================================*
 *
 *   static signed PrintWatchdogReport (struct plc * plc, char const * version);
 *
 *   Read the watchdog report using VS_WD_RPT and write to file in
 *   XML format; this file may be sent to QCA for analysis;
 *
 *   The XML version rquires additional information and so VS_SW_VER
 *   is used to collect it;
 *
 *   This VW_WD_RPT message protocol returns an indication message,
 *   not a confirm message;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed PrintWatchdogReport (struct plc * plc, char const * version)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_wd_rpt_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint16_t SESSIONID;
		uint8_t CLR;
	}
	* request = (struct vs_wd_rpt_request *) (message);
	struct __packed vs_wd_rpt_ind
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint16_t SESSIONID;
		uint8_t NUMPARTS;
		uint8_t CURPART;
		uint16_t RDATALENGTH;
		uint8_t RDATAOFFSET;
		uint8_t RDATA [1];
	}
	* indicate = (struct vs_wd_rpt_ind *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_WD_RPT | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	request->SESSIONID = HTOLE16 (WD_SESSION_ID);
	request->CLR = plc->readaction;
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	printf ("<WatchdogReport>");
	do
	{
		if (ReadMME (plc, 0, (VS_WD_RPT | MMTYPE_IND)) <= 0)
		{
			printf ("</WatchdogReport>");
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (indicate->MSTATUS)
		{
			printf ("</WatchdogReport>");
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		printf ("<Packet>");
		printf ("<Version>%s</Version>", version);
		printf ("<OUI>%s</OUI>", "00B052");
		printf ("<Status>0</Status>");
		printf ("<SessionId>0</SessionId>");
		printf ("<NumParts>%d</NumParts>", indicate->NUMPARTS);
		printf ("<CurPart>%d</CurPart>", indicate->CURPART);
		printf ("<DataLength>%d</DataLength>", LE16TOH (indicate->RDATALENGTH));
		printf ("<DataOffset>%d</DataOffset>", indicate->RDATAOFFSET);
		printf ("<Data>");
		b64dump (indicate->RDATA, LE16TOH (indicate->RDATALENGTH), 0, stdout);
		printf ("</Data>");
		printf ("</Packet>");
	}
	while (indicate->CURPART < indicate->NUMPARTS);
	printf ("</WatchdogReport>");
	return (0);
}


/*====================================================================*
 *
 *   static signed PrintCheckpointReport (struct plc * plc, char const * version);
 *
 *   Read the watchdog reqport using VS_CP_RPT and write to file in
 *   binary or XML format; this file can be sent to Atheros Support
 *   for analysis;
 *
 *   The XML version rquires additional information and so VS_SW_VER
 *   is used to collect it;
 *
 *   This VW_WD_RPT message protocol returns an indication message,
 *   not a confirm message;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed PrintCheckpointReport (struct plc * plc, char const * version)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_cp_rpt_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint16_t SESSIONID;
		uint8_t CLR;
	}
	* request = (struct vs_cp_rpt_request *) (message);
	struct __packed vs_cp_rpt_ind
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MAJORVERSION;
		uint8_t MINORVERSION;
		uint8_t RESERVED [14];
		uint16_t SESSIONID;
		uint32_t TOTALBUFFERSIZE;
		uint32_t BLOCKOFFSET;
		uint32_t BYTEINDEXOFNEXTPOSITION;
		uint8_t NUMPARTS;
		uint8_t CURPART;
		uint16_t RDATALENGTH;
		uint8_t RDATAOFFSET;
		uint8_t RDATA [1];
	}
	* indicate = (struct vs_cp_rpt_ind *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_CP_RPT | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	request->SESSIONID = HTOLE16 (WD_SESSION_ID);
	request->CLR = plc->readaction;
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	printf ("<CheckpointReport>");
	do
	{
		if (ReadMME (plc, 0, (VS_CP_RPT | MMTYPE_IND)) <= 0)
		{
			printf ("</CheckpointReport>");
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (_anyset (indicate->MSTATUS, MSTATUS_STATUS))
		{
			printf ("</CheckpointReport>");
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		printf ("<Packet>");
		printf ("<Version>%s</Version>", version);
		printf ("<Status>0</Status>");
		printf ("<MajorVersionBit>%d</MajorVersionBit>", indicate->MSTATUS & MSTATUS_MAJORVERSION? 1:0);
		printf ("<BufferIsLocked>%d</BufferIsLocked>", indicate->MSTATUS & MSTATUS_BUFFERISLOCKED? 1:0);
		printf ("<AutoLockOnResetIsOn>%d</AutoLockOnResetIsOn>", indicate->MSTATUS & MSTATUS_AUTOLOCKONRESET? 1:0);
		printf ("<UnsolicitedUpdatesIsOn>%d</UnsolicitedUpdatesIsOn>", indicate->MSTATUS & MSTATUS_UNSOLICITEDUPDATES? 1:0);
		printf ("<Unsolicited>%d</Unsolicited>", indicate->MSTATUS & MSTATUS_UNSOLICITED? 1:0);
		printf ("<MajorVersion>%d</MajorVersion>", indicate->MAJORVERSION);
		printf ("<MinorVersion>%d</MinorVersion>", indicate->MINORVERSION);
		printf ("<Reserved1>0</Reserved1>");
		printf ("<Reserved2>0</Reserved2>");
		printf ("<Reserved3>0</Reserved3>");
		printf ("<Reserved4>0</Reserved4>");
		printf ("<SessionId>1</SessionId>");
		printf ("<TotalBufferSize>%d</TotalBufferSize>", LE32TOH (indicate->TOTALBUFFERSIZE));
		printf ("<BlockOffset>%d</BlockOffset>", LE32TOH (indicate->BLOCKOFFSET));
		printf ("<ByteIndexOfNextPos>%d</ByteIndexOfNextPos>", LE32TOH (indicate->BYTEINDEXOFNEXTPOSITION));
		printf ("<NumParts>%d</NumParts>", indicate->NUMPARTS);
		printf ("<CurPart>%d</CurPart>", indicate->CURPART);
		printf ("<RDataLength>%d</RDataLength>", LE16TOH (indicate->RDATALENGTH));
		printf ("<RDataOffset>%d</RDataOffset>", indicate->RDATAOFFSET);
		printf ("<RData>");
		b64dump (indicate->RDATA, LE16TOH (indicate->RDATALENGTH), 0, stdout);
		printf ("</RData>");
		printf ("</Packet>");
	}
	while (indicate->CURPART < indicate->NUMPARTS);
	printf ("</CheckpointReport>");
	return (0);
}


/*====================================================================*
 *
 *   static signed Diagnostics (struct plc * plc)
 *
 *   read the firmware version string from a device before reading
 *   and writing the watchdog and checkpoint reports in XML format;
 *
 *
 *--------------------------------------------------------------------*/

static signed Diagnostics (struct plc * plc)

{
	char version [PLC_VERSION_STRING];
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_sw_ver_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MDEVICEID;
		uint8_t MVERLENGTH;
		char MVERSION [PLC_VERSION_STRING];
	}
	* request = (struct vs_sw_ver_request *) (message);
	struct __packed vs_sw_ver_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MDEVICEID;
		uint8_t MVERLENGTH;
		char MVERSION [PLC_VERSION_STRING];
	}
	* confirm = (struct vs_sw_ver_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_SW_VER | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	if (ReadMME (plc, 0, (VS_SW_VER | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	memcpy (version, confirm->MVERSION, sizeof (version));
	if (_anyset (plc->flags, PLC_XML_FORMAT))
	{
		printf ("<?xml version='1.0' encoding='utf-8' standalone='yes'?>");
		printf ("<Diagnostics>");
		PrintWatchdogReport (plc, version);
		PrintCheckpointReport (plc, version);
		printf ("</Diagnostics>\n");
		return (0);
	}
	PrintRawWatchdogReport (plc);
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
		"cCei:qvx",
		"device [device] [...] [> stdout]",
		"Qualcomm Atheros INT6x00 Log Retrieval Utility",
		"c\tcustom report",
		"C\tclear report",
		"e\tredirect stderr to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"q\tquiet mode",
		"v\tverbose mode",
		"x\tprint watchdog report in XML format",
		(char const *) (0)
	};

#include "../plc/plc.c"

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
	plc.readaction = WD_ACTION_READ;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'C':
			plc.readaction |= WD_ACTION_CLEAR;
			break;
		case 'c':
			plc.readaction |= WD_ACTION_CUSTOM;
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'x':
			_setbits (plc.flags, PLC_XML_FORMAT);
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
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
	{
		if (plc.rpt.file != -1)
		{
			error (1, ECANCELED, PLC_NODEVICE);
		}
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}

#if defined (WIN32)

	if (format == INT6KLOG_FMT_RAW)
	{
		setmode (STDOUT_FILENO, O_BINARY);
	}

#endif

	if (!argc)
	{
		Diagnostics (&plc);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		Diagnostics (&plc);
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

