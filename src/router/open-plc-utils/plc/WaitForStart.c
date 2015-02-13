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
 *   signed WaitForStart (struct plc * plc, char string [], size_t length);
 *
 *   plc.h
 *
 *   poll the local device with VS_SW_VER messages until it responds
 *   or the retry count expires; return 0 if the device responds in
 *   time or -1 if it does not;
 *
 *   if the device responds then set the struct plc hardwareID field
 *   to indicate the true hardware platform; function chipset corrects
 *   some unfortunate bootloader errors;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef WAITFORSTART_SOURCE
#define WAITFORSTART_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <sys/time.h>

#include "../plc/plc.h"
#include "../tools/timer.h"
#include "../tools/error.h"

signed WaitForStart (struct plc * plc, char string [], size_t length)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	struct timeval ts;
	struct timeval tc;
	unsigned timer = 0;

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_sw_ver_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
	}
	* request = (struct vs_sw_ver_request *) (message);
	struct __packed vs_sw_ver_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t MDEVICEID;
		uint8_t MVERLENGTH;
		char MVERSION [PLC_VERSION_STRING];
		uint16_t MPLATFORM;
	}
	* confirm = (struct vs_sw_ver_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (string, 0, length);
	if (gettimeofday (&ts, NULL) == -1)
	{
		error (1, errno, CANT_START_TIMER);
	}
	for (timer = 0; timer < plc->timer; timer = SECONDS (ts, tc))
	{
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_SW_VER | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		if (ReadMME (plc, 0, (VS_SW_VER | MMTYPE_CNF)) < 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (gettimeofday (&tc, NULL) == -1)
		{
			error (1, errno, CANT_RESET_TIMER);
		}
		if (plc->packetsize)
		{
			chipset (confirm);
			plc->hardwareID = confirm->MDEVICEID;
			memcpy (channel->peer, request->ethernet.OSA, sizeof (channel->peer));
			memcpy (string, confirm->MVERSION, confirm->MVERLENGTH);
			return (0);
		}
	}
	return (-1);
}


#endif

