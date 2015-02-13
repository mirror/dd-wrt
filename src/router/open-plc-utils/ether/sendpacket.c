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
 *   ssize_t sendpacket (struct channel const * channel, void * memory, ssize_t extent);
 *
 *   channel.h
 *
 *   send one packet over a raw packet channel; return the number of
 *   bytes sent or -1 on error; dump outgoing packets on stdout when
 *   the VERBOSE flag is set;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SENDPACKET_SOURCE
#define SENDPACKET_SOURCE

#include <unistd.h>

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/flags.h"

ssize_t sendpacket (struct channel const * channel, void * memory, ssize_t extent)

{
	if (_anyset (channel->flags, CHANNEL_VERBOSE))
	{
		hexdump (memory, 0, extent, stdout);
	}

#if defined (__linux__)

	extent = sendto (channel->fd, memory, extent, 0, (struct sockaddr *) (0), (socklen_t) (0));

#elif defined (__APPLE__) || defined (__OpenBSD__)

	extent = write (channel->fd, memory, extent);

#elif defined (WINPCAP) || defined (LIBPCAP)

	if (pcap_sendpacket (channel->socket, (byte *)(memory), extent))
	{
		extent = -1;
	}

#else
#error "Unknown Environment"
#endif

	return (extent);
}


#endif

