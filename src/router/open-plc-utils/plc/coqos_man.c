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
 *   coqos_man.c - Manage MCSMA streams
 *
 *
 *   Contributor(s):
 *	Bill Wike <bill.wike@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
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
#include "../plc/coqos.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/todigit.c"
#include "../tools/typename.c"
#include "../tools/error.c"
#include "../tools/synonym.c"
#endif

#ifndef MAKEFILE
#include "../plc/Request.c"
#include "../plc/Confirm.c"
#include "../plc/Failure.c"
#include "../plc/Display.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../plc/Devices.c"
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
#include "../mme/UnwantedMessage.c"
#endif

/*====================================================================*
 *
 *   signed con_man (struct plc * plc, uint16_t TOT_BW_USED);
 *
 *
 *
 *   Contributor(s):
 *	Bill Wike <bill.wike@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed con_man (struct plc * plc, uint16_t TOT_BW_USED)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_con_info_req
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t REQ_TYPE;
		uint32_t RSVD;
		uint16_t CSPEC_VER;
		uint16_t CID;
		uint16_t TOT_BW_USED;
	}
	* request = (struct vs_con_info_req *)(message);
	struct __packed vs_con_info_cnf
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t REQ_TYPE;
		uint8_t MSTATUS;
		uint16_t ERR_REC_CODE;
		uint32_t RSVD;
		uint8_t NUM_CONN;
		struct conn_info CONN_INFO [64];
	}
	* confirm = (struct vs_con_info_cnf *)(message);
	struct conn_info * conn_info =(struct conn_info*)(confirm->CONN_INFO);
	struct vs_con_info_cnf *indicate = (struct vs_con_info_cnf *)(message);
	struct __packed vs_mod_conn_req
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t REQ_ID;
		uint32_t RSVD;
		uint16_t CID;
		uint8_t MOD_CTRL;
	}
	* mc_request = (struct vs_mod_conn_req *)(message);
	struct __packed vs_mod_conn_cnf
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t REQ_ID;
		uint8_t MSTATUS;
		uint16_t ERR_REC_CODE;
		uint32_t RSVD;
	}
	* mc_confirm = (struct vs_mod_conn_cnf *)(message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	int i;
	int bwused;
	int lowest;
	uint16_t lowcid;
	while (1)
	{
		Request (plc, "COQOS connection information (requesting bandwidth notification)");
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_CONN_INFO | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		request->REQ_TYPE = 0x04;
		request->CID = 0x00;
		request->CSPEC_VER = 0x01;
		request->TOT_BW_USED = TOT_BW_USED;
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		while (ReadMME (plc, 0, (VS_CONN_INFO | MMTYPE_CNF)) <= 0);
		if (confirm->MSTATUS)
		{
			Failure (plc, "Could not set up bandwidth notification.");
		}
		else
		{
			Confirm (plc, "Set up bandwidth monitoring.");
			break;
		}
		sleep (1);
	}
	while (1)
	{

/*
 * Wait for indications saying too much bandwidth is
 * in use.
 */

		while (ReadMME (plc, 0, (VS_CONN_INFO | MMTYPE_IND)) <= 0)
		{
			continue;
		}
		if (indicate->MSTATUS)
		{
			Failure (plc, "Connection Info Indicate error");
			continue;
		}
		printf ("Number of Connections: %d\n", indicate->NUM_CONN);
		conn_info = (struct conn_info *)(confirm->CONN_INFO);
		bwused = 0;
		lowest = conn_info->CONN_COQOS_PRIO;
		lowcid = conn_info->CID;
		for (i = 0; i < indicate->NUM_CONN; i++)
		{
			bwused += conn_info->BW_USED;
			if (conn_info->CONN_COQOS_PRIO < lowest)
			{
				lowest = conn_info->CONN_COQOS_PRIO;
				lowcid = conn_info->CID;
			}
			conn_info++;
		}
		printf ("Total bandwidth used: %d\n", bwused);
		if (bwused <= TOT_BW_USED)
		{
			continue;
		}
		printf ("Bandwidth limit exceeded dropping: %04X\n", lowcid);
		Request (plc, "COQOS Modify Connection (suspend)");
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_CONN_MOD | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		mc_request->MOD_CTRL = 0x00;
		mc_request->CID = lowcid;
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		while (ReadMME (plc, 0, (VS_CONN_MOD | MMTYPE_CNF)) <= 0);
		if (mc_confirm->MSTATUS)
		{
			Failure (plc, "Could not suspend stream.");
			continue;
		}
		printf ("Connection ID: %04X suspended\n", lowcid);
		sleep (5);

/* flush out old data */

		while (ReadMME (plc, 0, (VS_CONN_INFO | MMTYPE_IND)) > 0);
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
		"ei:qv",
		"limit [device] [...]",
		"CoQos Stream Utility",
		"e\tredirect stderr to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};

#include "../plc/plc.c"

	uint16_t limit;
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
	if (!argc)
	{
		error (1, ECANCELED, "No limit given");
	}
	limit = (uint16_t)(uintspec (* argv++, 10, 90000));
	argc--;
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		con_man (&plc, limit);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		con_man (&plc, limit);
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

