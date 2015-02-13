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
 *   signed VersionInfo2 (struct plc * plc);
 *
 *   plc.h
 *
 *   read hardware and software version information from an INT6000
 *   using a VS_SW_VER Request Message;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef VERSIONINFO2_SOURCE
#define VERSIONINFO2_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../tools/error.h"
#include "../tools/memory.h"
#include "../tools/symbol.h"
#include "../plc/plc.h"

signed VersionInfo2 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_sw_ver_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t COOKIE;
	}
	* request = (struct vs_sw_ver_request *) (message);
	struct __packed vs_sw_ver_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MDEVICE_CLASS;
		uint8_t MVERLENGTH;
		char MVERSION [254];
		uint32_t IDENT;
		uint32_t STEPPING_NUM;
		uint32_t COOKIE;
		uint32_t RSVD [6];
	}
	* confirm = (struct vs_sw_ver_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Request Version Information");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_SW_VER | MMTYPE_REQ));
	request->COOKIE = HTOLE32 (plc->cookie);
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_SW_VER | MMTYPE_CNF)) > 0)
	{

#if 0

		fprintf (stderr, "DEVICE CLASS (%d)\n", LE32TOH (confirm->MDEVICE_CLASS));
		fprintf (stderr, "VERSION LENGTH (%d)\n", LE32TOH (confirm->MVERLENGTH));
		fprintf (stderr, "VERSION STRING [%s]\n", LE32TOH (confirm->MVERSION));
		fprintf (stderr, "IDENT [0x%08X]\n", LE32TOH (confirm->IDENT));
		fprintf (stderr, "STEPPING NUMBER [0x%08X]\n", LE32TOH (confirm->STEPPING_NUM));
		fprintf (stderr, "COOKIE [0x%08X]\n", LE32TOH (confirm->COOKIE));

#endif

		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
		chipset (confirm);
		Display (plc, "%s %s", chipsetname (confirm->MDEVICE_CLASS), confirm->MVERSION);
	}
	return (0);
}


#endif

