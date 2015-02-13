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
 *   signed Antiphon (struct plc * plc);
 *
 *   plc.h
 *
 *   instruct one powerline device to send trash frames to another
 *   for a given period (in seconds) to establish source device RX
 *   PHY rate and target device TX PHY rate; this does not work if
 *   the source device is also the local device, for some reason;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef ANTIPHON_SOURCE
#define ANTIPHON_SOURCE

#include <memory.h>
#include <errno.h>

#include "../tools/error.h"
#include "../tools/flags.h"
#include "../plc/plc.h"

signed Antiphon (struct plc * plc, byte source [], byte target [])

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_fr_lbk_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t DURATION;
		uint8_t RESERVED;
		uint16_t LENGTH;
		uint8_t PACKET [1038];
	}
	* request = (struct vs_fr_lbk_request *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	if (_allclr (plc->flags, PLC_SILENCE))
	{
		char sourcename [ETHER_ADDR_LEN * 3];
		char targetname [ETHER_ADDR_LEN * 3];
		hexdecode (source, ETHER_ADDR_LEN, sourcename, sizeof (sourcename));
		hexdecode (target, ETHER_ADDR_LEN, targetname, sizeof (targetname));
		fprintf (stderr, "%s %s %s\n", channel->ifname, sourcename, targetname);
	}
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, source, target, channel->type);
	QualcommHeader (&request->qualcomm, 0, 41036);
	request->DURATION = plc->timer;
	request->LENGTH = HTOLE16 (sizeof (request->PACKET));
	memset (request->PACKET, 0xA5, sizeof (request->PACKET));
	EthernetHeader (request->PACKET, target, source, ETHERTYPE_IP);
	plc->packetsize = sizeof (* request);
	if (SendMME (plc) <= 0)
	{
		error (1, errno, CHANNEL_CANTSEND);
	}

#if 0

/*
 *	This causes a multi-device session to terminate if the device has recently
 *	been removed or powered-off; The device appears to be present but will not
 *	respond; Also, this terminates a session if the network is overloaded with
 *	traffic;
 */

	if (ReadMME (plc, 0, 41037) <= 0)
	{
		error (1, errno, CHANNEL_CANTREAD);
	}

#endif

	sleep (plc->timer);
	return (0);
}


#endif

