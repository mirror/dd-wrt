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
 *   signed ReadMME (struct plc * plc, uint8_t MMV, uint16_t MMTYPE);
 *
 *   plc.h
 *
 *   wait for a QCA vendor specific message of given MMV and MMTYPE;
 *   discard unwanted messages until the desired message arrives or
 *   the channel timeout expires; return packetsize on success, 0 on
 *   timeout or -1 on error;
 *
 *   see SendMME for the send counterpart to this function;
 *
 *   a well designed UnwantedMessage function is critical here;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef READMME_SOURCE
#define READMME_SOURCE

#include <sys/time.h>

#include <stdint.h>
#include <memory.h>
#include <errno.h>

#include "../tools/error.h"
#include "../tools/timer.h"
#include "../plc/plc.h"
#include "../mme/mme.h"

signed ReadMME (struct plc * plc, uint8_t MMV, uint16_t MMTYPE)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	struct timeval ts;
	struct timeval tc;
	if (gettimeofday (&ts, NULL) == -1)
	{
		error (1, errno, CANT_START_TIMER);
	}
	while ((plc->packetsize = readpacket (channel, message, sizeof (* message))) >= 0)
	{
		if (UnwantedMessage (message, plc->packetsize, MMV, MMTYPE))
		{
			if (gettimeofday (&tc, NULL) == -1)
			{
				error (1, errno, CANT_RESET_TIMER);
			}
			if (channel->timeout < 0)
			{
				continue;
			}
			if (channel->timeout > MILLISECONDS (ts, tc))
			{
				continue;
			}
			plc->packetsize = 0;
		}
		break;
	}
	return (plc->packetsize);
}


#endif

