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
 *   signed Topology1 (struct plc * plc);
 *
 *   plc.h
 *
 *   display network topology on stdout; the topology shows bridges
 *   associated with a target bridge along with the tx/rx phy rates
 *   between the target and associated bridge plus the hardware and
 *   firmware revison of each bridge; the target bridge is shown on
 *   the first line and associated bridge are shown after;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef TOPOLOGY1_SOURCE
#define TOPOLOGY1_SOURCE

#include <memory.h>
#include <errno.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../plc/plc.h"

signed Topology1 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_nw_info_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
	}
	* request = (struct vs_nw_info_request *)(message);
	struct __packed vs_nw_info_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t data [1];
	}
	* confirm = (struct vs_nw_info_confirm *)(message);
	struct __packed station
	{
		uint8_t MAC [ETHER_ADDR_LEN];
		uint8_t TEI;
		uint8_t BDA [ETHER_ADDR_LEN];
		uint8_t AVGTX;
		uint8_t AVGRX;
	}
	* station;
	struct __packed network
	{
		uint8_t NID [7];
		uint8_t SNID;
		uint8_t TEI;
		uint8_t ROLE;
		uint8_t CCO_MAC [ETHER_ADDR_LEN];
		uint8_t CCO_TEI;
		uint8_t NUMSTAS;
		struct station stations [1];
	}
	* network;
	struct __packed networks
	{
		uint8_t NUMAVLNS;
		struct network networks [1];
	}
	* networks = (struct networks *) (confirm->data);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	byte list [255] [ETHER_ADDR_LEN];
	signed bridges = LocalDevices (channel, message, list, sizeof (list));
	while (bridges--)
	{
		char address [ETHER_ADDR_LEN * 3];
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, list [bridges], channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_NW_INFO | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		if (SendMME (plc) <= 0)
		{
			error (1, errno, CHANNEL_CANTSEND);
		}
		if (ReadMME (plc, 0, (VS_NW_INFO | MMTYPE_CNF)) <= 0)
		{
			error (0, errno, CHANNEL_CANTREAD);
			continue;
		}
		network = (struct network *)(&networks->networks);
		if (_allclr (channel->flags, PLC_SILENCE))
		{
			printf (" P/L NET TEI ------ MAC ------ ------ BDA ------  TX  RX CHIPSET FIRMWARE\n");
		}
		printf (" LOC");
		printf (" %s", memcmp (confirm->ethernet.OSA, network->CCO_MAC, sizeof (confirm->ethernet.OSA))? "STA": "CCO");
		printf (" %03d", network->TEI);
		printf (" %s", hexstring (address, sizeof (address), confirm->ethernet.OSA, sizeof (confirm->ethernet.OSA)));
		printf (" %s", hexstring (address, sizeof (address), confirm->ethernet.ODA, sizeof (confirm->ethernet.ODA)));
		printf (" n/a");
		printf (" n/a");
		Platform (channel, confirm->ethernet.OSA);
		printf ("\n");
		while (networks->NUMAVLNS--)
		{
			station = (struct station *)(&network->stations);
			while (network->NUMSTAS--)
			{
				printf (" REM");
				printf (" %s", memcmp (station->MAC, network->CCO_MAC, sizeof (station->MAC))? "STA": "CCO");
				printf (" %03d", station->TEI);
				printf (" %s", hexstring (address, sizeof (address), station->MAC, sizeof (station->MAC)));
				printf (" %s", hexstring (address, sizeof (address), station->BDA, sizeof (station->BDA)));
				printf (" %03d", station->AVGTX);
				printf (" %03d", station->AVGRX);
				Platform (channel, station->MAC);
				printf ("\n");
				station++;
			}
			network = (struct network *)(station);
		}
	}
	return (0);
}


#endif

