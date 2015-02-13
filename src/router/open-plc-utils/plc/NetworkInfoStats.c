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
 *   signed NetworkInfoStats (struct plc * plc);
 *
 *   plc.h
 *
 *   Request network membership information from the peer device using
 *   the VS_NW_INFO_STATS message;
 *
 *   This function is similar to function NetworkInformation() but the
 *   output format is different;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef NETWORKINFOSTATS_SOURCE
#define NETWORKINFOSTATS_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"

signed NetworkInfoStats (struct plc * plc)

{
	extern char const * StationRole [STATIONROLES];
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

#if defined(INT6x00)

	struct __packed vs_ns_info_stats_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t FIRST_TEI;
	}
	* request = (struct vs_ns_info_stats_request *)(message);
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
		uint8_t ACCESS uint8_t NEIGHBOR_NETWORKS;
		uint8_t CCO_TEI;
		uint8_t NUMSTAS;
		struct station stations [1];
	}
	* network;
	struct __packed vs_ns_info_stats_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t FIRST_TEI;
		uint8_t IN_AVLN;
		struct network network;
	}
	* confirm = (struct vs_ns_info_stats_confirm *)(message);

#elif defined (AR7x00)

	struct __packed vs_ns_info_stats_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_fmi qualcomm;
		uint8_t MME_SUBVER;
		uint8_t RESERVED [3];
		uint8_t FIRST_TEI;
		UINT8_6 NUM_STAS;
	}
	* request = (struct vs_ns_info_stats_request *)(message);
	struct __packed station
	{
		uint8_t MAC [ETHER_ADDR_LEN];
		uint8_t TEI;
		uint8_t Reserved [3];
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
		uint8_t Reserved1;
		uint8_t SNID;
		uint8_t TEI;
		uint8_t Reserved2 [2];
		uint8_t ROLE;
		uint8_t CCO_MAC [ETHER_ADDR_LEN];
		uint8_t ACCESS;
		uint8_t NEIGHBOR_NETWORKS;
		uint8_t CCO_TEI;
		uint8_t Reserved3 [7];
		uint8_t NUMSTAS;
		uint8_t Reserved4 [5];
		struct station stations [1];
	}
	* network;
	struct __packed vs_ns_info_stats_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_fmi qualcomm;
		uint8_t SUB_VERSION;
		uint8_t Reserved;
		uint16_t DATA_LEN;
		uint8_t FIRST_TEI;
		uint8_t NUM_STAS;
		uin16_t Reserved;
		uint8_t IN_AVLN;
		struct network network;
	}
	* confirm = (struct vs_ns_info_stats_confirm *)(message);

#else
#error "Unspecified Atheros chipset"
#endif

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Fetch Network Information");
	memset (message, 0, sizeof (* message));

#if defined (INT6x00)

	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_NW_INFO_STATS | MMTYPE_REQ));

#elif defined (AR7x00)

	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader1 (&request->qualcomm, 1, (VS_NW_INFO_STATS | MMTYPE_REQ));

#else
#error "Unspecified Atheros chipset"
#endif

	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}

#if defined (INT6x00)

	while (ReadMME (plc, 0, (VS_NW_INFO_STATS | MMTYPE_CNF)) > 0)

#elif defined (AR7x00)

	while (ReadMME (plc, 1, (VS_NW_INFO_STATS | MMTYPE_CNF)) > 0)

#else
#error "Unspecified Atheros chipset"
#endif

	{
		char string [24];
		Confirm (plc, "Found %d Network(s)\n", networks->NUMAVLNS);
		network = (struct network *)(&networks->networks);
		while (networks->NUMAVLNS--)
		{
			printf ("\tnetwork->NID = %s\n", hexstring (string, sizeof (string), network->NID, sizeof (network->NID)));
			printf ("\tnetwork->SNID = %d\n", network->SNID);
			printf ("\tnetwork->TEI = %d\n", network->TEI);
			printf ("\tnetwork->ROLE = 0x%02X (%s)\n", network->ROLE, StationRole [network->ROLE]);
			printf ("\tnetwork->CCO_DA = %s\n", hexstring (string, sizeof (string), network->CCO_MAC, sizeof (network->CCO_MAC)));
			printf ("\tnetwork->CCO_TEI = %d\n", network->CCO_TEI);
			printf ("\tnetwork->STATIONS = %d\n", network->NUMSTAS);
			printf ("\n");
			station = (struct station *)(&network->stations);
			while (network->NUMSTAS--)
			{
				char * TX = "";
				char * RX = "";
				printf ("\t\tstation->MAC = %s\n", hexstring (string, sizeof (string), station->MAC, sizeof (station->MAC)));
				printf ("\t\tstation->TEI = %d\n", station->TEI);
				printf ("\t\tstation->BDA = %s\n", hexstring (string, sizeof (string), station->BDA, sizeof (station->BDA)));

#if defined (AR7x00)

				station->AVGTX = LE16TOH (station->AVGTX);
				station->AVGRX = LE16TOH (station->AVGRX);
				TX = ((station->COUPLING >> 4) & 0x0F)? "Alternate": "Primary";
				RX = ((station->COUPLING >> 0) & 0x0F)? "Alternate": "Primary";

#endif

				printf ("\t\tstation->AvgPHYDR_TX = %03d mbps %s\n", station->AVGTX, TX);
				printf ("\t\tstation->AvgPHYDR_RX = %03d mbps %s\n", station->AVGRX, RX);
				printf ("\n");
				station++;
			}
			network = (struct network *)(station);
		}
	}
	return (0);
}


#endif

