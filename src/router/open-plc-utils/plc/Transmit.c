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
 *   signed Transmit (struct plc * plc, byte source [], byte target []) ;
 *
 *   send TCP/IP frames to a remote powerline device to establish
 *   the source device TX PHY rate and remote device RX PHY rate;
 *
 *--------------------------------------------------------------------*/

#ifndef TRANSMIT_SOURCE
#define TRANSMIT_SOURCE

#include <memory.h>
#include <errno.h>
#include <sys/time.h>

#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/types.h"
#include "../tools/timer.h"
#include "../plc/plc.h"

signed Transmit (struct plc * plc, byte source [], byte target [])

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	struct timeval ts;
	struct timeval tc;
	unsigned timer = 0;
	if (_allclr (plc->flags, PLC_SILENCE))
	{
		char sourcename [ETHER_ADDR_LEN * 3];
		char targetname [ETHER_ADDR_LEN * 3];
		hexdecode (source, ETHER_ADDR_LEN, sourcename, sizeof (sourcename));
		hexdecode (target, ETHER_ADDR_LEN, targetname, sizeof (targetname));
		fprintf (stderr, "%s %s %s\n", channel->ifname, sourcename, targetname);
	}
	memset (message, 0xA5, sizeof (* message));
	EthernetHeader (message, target, source, ETHERTYPE_IP);
	if (gettimeofday (&ts, NULL) == -1)
	{
		error (1, errno, CANT_START_TIMER);
	}
	for (timer = 0; timer < plc->timer; timer = SECONDS (ts, tc))
	{
		if (sendpacket (channel, message, sizeof (* message)) <= 0)
		{
			error (1, ECANCELED, CHANNEL_CANTSEND);
		}
		if (gettimeofday (&tc, NULL) == -1)
		{
			error (1, errno, CANT_RESET_TIMER);
		}
		SLEEP (100);
	}
	return (0);
}


#endif

