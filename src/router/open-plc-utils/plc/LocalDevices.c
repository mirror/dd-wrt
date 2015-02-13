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
 *   unsigned LocalDevices (struct channel * channel,  struct message * message, void * memory, size_t extent);
 *
 *   plc.h
 *
 *   populate a memory region with consecutive Ethernet addresses;
 *   the addresses belong to Atheros powerline devices connected to
 *   the host interface specified in struct channel; each bridge may
 *   be connected to one or more HomePlug AV devices via powerline;
 *
 *   each powerline bridge normally belongs to a different powerline
 *   network but is is possible for multiple bridges to belong to the
 *   same powerline network thus leading to confusing configurations;
 *
 *   use function NetworkDevices() to discover all devices on each
 *   powerline network;
 *
 *   although this function accepts a channel structure, it ignores
 *   the channel peer address and sends a VS_SW_VER request message
 *   to the Local Management Address; this causes all local devices
 *   to respond; the list is a collection of source addresses taken
 *   from all responses;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef NETWORKBRIDGES_SOURCE
#define NETWORKBRIDGES_SOURCE

#include <memory.h>
#include <errno.h>

#include "../plc/plc.h"
#include "../ether/channel.h"
#include "../tools/types.h"
#include "../tools/error.h"

unsigned LocalDevices (struct channel const * channel, struct message * message, void * memory, size_t extent)

{
	extern const byte localcast [ETHER_ADDR_LEN];
	struct vs_sw_ver_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
	}
	* request = (struct vs_sw_ver_request *)(message);
	uint8_t * origin = (uint8_t *)(memory);
	uint8_t * offset = (uint8_t *)(memory);
	ssize_t packetsize;
	memset (memory, 0, extent);
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, localcast, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_SW_VER | MMTYPE_REQ));
	if (sendpacket (channel, message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) <= 0)
	{
		return (0);
	}
	while ((packetsize = readpacket (channel, message, sizeof (* message))) > 0)
	{
		if (UnwantedMessage (message, packetsize, 0, (VS_SW_VER | MMTYPE_CNF)))
		{
			continue;
		}
		if (extent >= sizeof (message->ethernet.OSA))
		{
			memcpy (offset, message->ethernet.OSA, sizeof (message->ethernet.OSA));
			offset += sizeof (message->ethernet.OSA);
			extent -= sizeof (message->ethernet.OSA);
		}
	}
	return ((unsigned)(offset - origin) / ETHER_ADDR_LEN);
}


#endif

