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
 *   signed NetworkTraffic2 (struct plc * plc);
 *
 *   plc.h
 *
 *   generate full mesh network traffic between powerline devices on
 *   all accessible powerline networks;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef NETWORKTRAFFIC2_SOURCE
#define NETWORKTRAFFIC2_SOURCE

#include <memory.h>
#include <errno.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../plc/plc.h"

signed NetworkTraffic2 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_nw_info_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_fmi qualcomm;
	}
	* request = (struct vs_nw_info_request *)(message);
	struct __packed vs_nw_info_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_fmi qualcomm;
		uint8_t SUB_VERSION;
		uint8_t RESERVED;
		uint16_t DATA_LEN;
		uint8_t DATA [1];
	}
	* confirm = (struct vs_nw_info_confirm *)(message);
	struct __packed station
	{
		uint8_t MAC [ETHER_ADDR_LEN];
		uint8_t TEI;
		uint8_t Reserved1;
		uint16_t Reserved2;
		uint8_t BDA [ETHER_ADDR_LEN];
		uint16_t AVGTX;
		uint8_t COUPLING;
		uint8_t Reserved3;
		uint16_t AVGRX;
		uint16_t Reserved4;
	}
	* station;
	struct __packed network
	{
		uint8_t NID [7];
		uint8_t Reserved1 [2];
		uint8_t SNID;
		uint8_t TEI;
		uint8_t Reserved2 [4];
		uint8_t ROLE;
		uint8_t CCO_MAC [ETHER_ADDR_LEN];
		uint8_t CCO_TEI;
		uint8_t Reserved3 [3];
		uint8_t NUMSTAS;
		uint8_t Reserved4 [5];
		struct station stations [1];
	}
	* network;
	struct __packed networks
	{
		uint8_t Reserved;
		uint8_t NUMAVLNS;
		struct network networks [1];
	}
	* networks = (struct networks *) (confirm->DATA);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	byte bridgelist [255] [ETHER_ADDR_LEN];
	unsigned bridges = LocalDevices (channel, message, bridgelist, sizeof (bridgelist));
	while (bridges--)
	{
		byte devicelist [255] [ETHER_ADDR_LEN];
		unsigned devices = 0;
		unsigned device;
		unsigned remote;
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, bridgelist [bridges], channel->host, channel->type);
		QualcommHeader1 (&request->qualcomm, 1, (VS_NW_INFO | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		if (SendMME (plc) <= 0)
		{
			error (0, errno, CHANNEL_CANTSEND);
			continue;
		}
		if (ReadMME (plc, 1, (VS_NW_INFO | MMTYPE_CNF)) <= 0)
		{
			error (0, errno, CHANNEL_CANTREAD);
			continue;
		}
		memcpy (devicelist [devices++], request->ethernet.OSA, sizeof (devicelist [0]));
		network = (struct network *)(&networks->networks);
		while (networks->NUMAVLNS--)
		{
			station = (struct station *)(&network->stations);
			while (network->NUMSTAS--)
			{
				memcpy (devicelist [devices++], station->MAC, sizeof (devicelist [0]));
				station++;
			}
			network = (struct network *)(station);
		}
		for (device = 1; device < devices; device++)
		{
			Transmit (plc, devicelist [0], devicelist [device]);
			Antiphon (plc, devicelist [device], devicelist [0]);
		}
		for (device = 1; device < devices; device++)
		{
			for (remote = 1; remote < devices; remote++)
			{
				if (remote != device)
				{
					Antiphon (plc, devicelist [device], devicelist [remote]);
				}
			}
		}
	}
	return (0);
}


#endif

