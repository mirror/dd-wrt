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
 *   signed Attributes2 (struct plc * plc);
 *
 *   plc.h
 *
 *   This plugin for program plc requests device attributes using
 *   a VS_OP_ATTRIBUTES message; attributes are pre-parsed versions
 *   of information returned by VS_SW_VER and other messages;
 *
 *   The VS_OP_ATTRIBUTES message structure changed between FW 3.3.4
 *   and 3.3.5 and fields are ocassionally appended to the end; you
 *   should not use this message for operational systems because the
 *   format may change again; it was originally intended for PTS use
 *   only;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef ATTRIBUTES2_SOURCE
#define ATTRIBUTES2_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/memory.h"
#include "../tools/format.h"
#include "../tools/error.h"
#include "../tools/flags.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

signed Attributes2 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_op_attributes_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t COOKIE;
		uint8_t RTYPE;
	}
	* request = (struct vs_op_attributes_request *) (message);
	struct __packed vs_op_attributes_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint16_t MSTATUS;
		uint32_t COOKIE;
		uint8_t RTYPE;
		uint16_t MLENGTH;
		uint8_t MBUFFER [1024];
	}
	* confirm = (struct vs_op_attributes_confirm *) (message);
	struct __packed attributes
	{
		uint8_t DEVICEFAMILY [16];
		uint8_t DEVICETYPE [16];
		uint32_t FWMAJORVERSION;
		uint32_t FWMINORVERSION;
		uint32_t COMPOSITEVERSION;
		uint32_t SUSTAININGVERSION;
		uint32_t BUILDNUMBER;
		uint8_t BUILDDATE [8];
		uint8_t RELEASETYPE [12];
		uint8_t FLAGS [3];
		uint32_t DRAMSIZE;
		uint32_t RAMBLOCKRXCOUNT;
		uint32_t RAMBLOCKTXCOUNT;
		uint32_t RAMBLOCKSHAREDCOUNT;
		uint32_t RAMBLOCKFREERXCOUNT;
		uint32_t RAMBLOCKFREETXCOUNT;
		uint32_t RAMBLOCKFREESHAREDCOUNT;
		uint32_t RELATIVESNRDIFFDB;
		uint32_t DSP384THREGVAL;
		uint8_t AFETXGAINDB;
		uint8_t AUTHMODE;
		uint8_t MICROCONTROLLERDIAGENABLED;
	}
	* attributes = (struct attributes *)(confirm->MBUFFER);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Fetch Device Attributes");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_OP_ATTRIBUTES | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	request->COOKIE = HTOLE32 (plc->cookie);
	request->RTYPE = 0;
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_OP_ATTRIBUTES | MMTYPE_CNF)) > 0)
	{

#if 0

		static char const * frequency [] =
		{
			"Unknown Frequency",
			"50Hz",
			"60Hz",
		};
		static char const * zero_cross [] =
		{
			"Not yet detected",
			"Detected",
			"Missing",
		};

#endif

		char string [512];
		size_t length = 0;
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}

#if 0
#if defined (__GNUC__)
#warning "Debug code active in module Attributes2"
#endif

		fprintf (stderr, "DEVICE FAMILY=[%-16.16s]\n", attributes->DEVICEFAMILY);
		fprintf (stderr, "DEVICE TYPE=[%-16.16s]\n", attributes->DEVICETYPE);
		fprintf (stderr, "FIRMWARE MAJOR VERSION=(%d)\n", LE32TOH (attributes->FWMAJORVERSION));
		fprintf (stderr, "FIRMWARE MINOR VERSION=(%d)\n", LE32TOH (attributes->FWMINORVERSION));
		fprintf (stderr, "COMPOSITE VERSION=(%d)\n", LE32TOH (attributes->COMPOSITEVERSION));
		fprintf (stderr, "SUSTAINING VERSION=(%d)\n", LE32TOH (attributes->SUSTAININGVERSION));
		fprintf (stderr, "BUILD NUMBER=(%d)\n", LE32TOH (attributes->BUILDNUMBER));
		fprintf (stderr, "BUILD DATE=[%-8.8s]\n", attributes->BUILDDATE);
		fprintf (stderr, "RELEASE TYPE=[%-12.12s]\n", attributes->RELEASETYPE);
		fprintf (stderr, "DRAM SIZE=[0x%08X](%d)\n", LE32TOH (attributes->DRAMSIZE), LE32TOH (attributes->DRAMSIZE));
		fprintf (stderr, "DRAM TYPES=[0x%02X]\n", attributes->FLAGS [0]);
		fprintf (stderr, "RESERVED FLAG=[0x%02X]\n", attributes->FLAGS [1]);
		fprintf (stderr, "LINE FREQUENCY=[0x%02X]\n", attributes->FLAGS [2]);
		fprintf (stderr, "AUTHORIZATION MODE=[0x%02X]\n", attributes->AUTHMODE);
		fprintf (stderr, "DSP384THREGVAL=[0x%02X]\n", attributes->DSP384THREGVAL);
		fprintf (stderr, "AFETXGAINDB=[0x%02X]\n", attributes->AFETXGAINDB);
		fprintf (stderr, "AUTHORIZATION MODE=[0x%02X]\n", attributes->AUTHMODE);

#if 0

		fprintf (stderr, "DSP384THREGVAL=[0x%08X]\n", LE32TOH (attributes->DSP384THREGVAL));
		fprintf (stderr, "AFETXGAINDB=[0x%02X]\n", attributes->AFETXGAINDB);

#endif
#endif

		length += snprintf (string + length, sizeof (string) - length, "%s", attributes->DEVICEFAMILY);
		length += snprintf (string + length, sizeof (string) - length, "-%s", attributes->DEVICETYPE);
		length += snprintf (string + length, sizeof (string) - length, "-%s", attributes->DEVICEFAMILY);
		length += snprintf (string + length, sizeof (string) - length, "-%d", LE32TOH (attributes->FWMAJORVERSION));
		length += snprintf (string + length, sizeof (string) - length, ".%d", LE32TOH (attributes->FWMINORVERSION));
		length += snprintf (string + length, sizeof (string) - length, ".%d", LE32TOH (attributes->COMPOSITEVERSION));
		length += snprintf (string + length, sizeof (string) - length, ".%d", LE32TOH (attributes->BUILDNUMBER));
		length += snprintf (string + length, sizeof (string) - length, "-%02d", LE32TOH (attributes->SUSTAININGVERSION));
		length += snprintf (string + length, sizeof (string) - length, "-%8.8s", attributes->BUILDDATE);
		length += snprintf (string + length, sizeof (string) - length, "-%s", attributes->RELEASETYPE);
		length += snprintf (string + length, sizeof (string) - length, " (%dmb)", LE32TOH (attributes->DRAMSIZE));
		Display (plc, "%s", string);
	}
	return (0);
}


#endif

