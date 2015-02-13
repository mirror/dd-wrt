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
 *   signed PushButton (struct plc * plc);
 *
 *   plc.h
 *
 *   THis plugin for program plc emulates pushbutton functionality
 *   using a MS_PB_ENC message;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef PUSHBUTTON_SOURCE
#define PUSHBUTTON_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../tools/error.h"
#include "../tools/memory.h"
#include "../plc/plc.h"

signed PushButton (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed ms_pb_enc_request
	{
		struct ethernet_hdr ethernet;
		struct homeplug_hdr homeplug;
		uint8_t PBACTION;
	}
	* request = (struct ms_pb_enc_request *) (message);
	struct __packed ms_pb_enc_confirm
	{
		struct ethernet_hdr ethernet;
		struct homeplug_hdr homeplug;
		uint8_t MSTATUS;
		uint8_t AVLNSTAT;
	}
	* confirm = (struct ms_pb_enc_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	if (plc->pushbutton == 1)
	{
		Request (plc, "Join Network");
	}
	if (plc->pushbutton == 2)
	{
		Request (plc, "Leave Network");
	}
	if (plc->pushbutton == 3)
	{
		Request (plc, "Fetch Network Status");
	}
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	request->homeplug.MMV = 0;
	request->homeplug.MMTYPE = HTOLE16 (MS_PB_ENC | MMTYPE_REQ);
	request->PBACTION = plc->pushbutton;
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	if (ReadMFG (plc, 0, (MS_PB_ENC | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	if (plc->pushbutton == 1)
	{
		Confirm (plc, "Joining ...");
		return (0);
	}
	if (plc->pushbutton == 2)
	{
		Confirm (plc, "Leaving ...");
		return (0);
	}
	if (plc->pushbutton == 3)
	{
		Confirm (plc, "Membership Status %d", confirm->AVLNSTAT);
		return (0);
	}
	Failure (plc, "Unexpected pushbutton action code");
	return (-1);
}


#endif

