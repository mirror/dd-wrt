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
 *   signed NetworkDevices1 (struct plc * plc, void * memory, size_t extent);
 *
 *   plc.h
 *
 *   return a list powerline network device addresses on a powerline
 *   network; the list consists of a known device plus all others on
 *   the same powerline network; the device is defined by the channel
 *   peer address and appears first in the list;
 *
 *   the device address must be explicit; it cannot be the broadcast
 *   or localcast address;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef NETWORKDEVICES1_SOURCE
#define NETWORKDEVICES1_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/types.h"
#include "../tools/error.h"
#include "../plc/plc.h"

signed NetworkDevices1 (struct plc * plc, void * memory, size_t extent)

{
	extern const byte broadcast [ETHER_ADDR_LEN];
	extern const byte localcast [ETHER_ADDR_LEN];
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	uint8_t * origin = (byte *)(memory);
	uint8_t * offset = (byte *)(memory);

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

	ssize_t packetsize;
	if (!memcmp (channel->peer, broadcast, sizeof (channel->peer)))
	{
		error (1, EINVAL, "Can't use broadcast address");
	}
	if (!memcmp (channel->peer, localcast, sizeof (channel->peer)))
	{
		error (1, EINVAL, "Can't use localcast address");
	}
	memset (memory, 0, extent);
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_NW_INFO | MMTYPE_REQ));
	if (sendpacket (channel, message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0)
	{
		error (1, errno, CHANNEL_CANTSEND);
	}
	while ((packetsize = readpacket (channel, message, sizeof (* message))) > 0)
	{
		if (UnwantedMessage (message, packetsize, 0, (VS_NW_INFO | MMTYPE_CNF)))
		{
			continue;
		}
		network = (struct network *)(&networks->networks);
		while (networks->NUMAVLNS--)
		{
			if (extent < sizeof (request->ethernet.OSA))
			{
				break;
			}
			memcpy (offset, request->ethernet.OSA, sizeof (request->ethernet.OSA));
			offset += sizeof (request->ethernet.OSA);
			extent -= sizeof (request->ethernet.OSA);
			station = (struct station *)(&network->stations);
			while (network->NUMSTAS--)
			{
				if (extent < sizeof (station->MAC))
				{
					break;
				}
				memcpy (offset, station->MAC, sizeof (station->MAC));
				offset += sizeof (station->MAC);
				extent -= sizeof (station->MAC);
				station++;
			}
			network = (struct network *)(station);
		}
	}
	return ((offset - origin) / ETHER_ADDR_LEN);
}


#endif

