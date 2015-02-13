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
 *   signed ResetAndWait (struct plc * plc);
 *
 *   plc.h
 *
 *   Reset the device using a VS_RS_DEV Request message then wait for
 *   the device to reset and reboot before returning;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef RESETANDWAIT_SOURCE
#define RESETANDWAIT_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/memory.h"

signed ResetAndWait (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_rs_dev_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
	}
	* request = (struct vs_rs_dev_request *) (message);
	struct __packed vs_rs_dev_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MDEVICEID;
		uint8_t MVERLENGTH;
		char MVERSION [PLC_VERSION_STRING];
	}
	* confirm = (struct vs_rs_dev_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	char firmware [PLC_VERSION_STRING];
	Request (plc, "Reset Device");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_RS_DEV | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	memset (firmware, 0, sizeof (firmware));

#if 0

	if (ReadMME (plc, (VS_RS_DEV | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	Confirm (plc, "Resetting ...");
	if (WaitForReset (plc))
	{
		Failure (plc, "Device did not Reset");
		return (-1);
	}
	if (WaitForStart (plc, firmware, sizeof (firmware)))
	{
		Failure (plc, "Device did not Start");
		return (-1);
	}

#else

	while (ReadMME (plc, 0, (VS_RS_DEV | MMTYPE_CNF)) > 0)
	{
		if (confirm->MSTATUS)
		{
			continue;
		}
		return (0);
	}

#endif

	return (-1);
}


#endif

