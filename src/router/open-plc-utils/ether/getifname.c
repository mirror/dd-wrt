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
 *   char * getifname (signed number);
 *
 *   ether.h
 *
 *   return the PCAP interface name for a given interface number; this
 *   function is only needed when using LIBPCAP or WINPCAP libraries;
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef GETIFNAME_SOURCE
#define GETIFNAME_SOURCE

#include <string.h>

#if defined (WINPCAP) || defined (LIBPCAP)
#include <pcap.h>
#endif

#include "../ether/ether.h"
#include "../tools/error.h"

char * getifname (signed index)

{
	char * name = (char *)(0);

#if defined (__linux__)

#elif defined (__APPLE__) || defined (__OpenBSD__)

#elif defined (WINPCAP) || defined (LIBPCAP)

	char buffer [PCAP_ERRBUF_SIZE];
	pcap_if_t *devices = (pcap_if_t *)(0);
	pcap_if_t *device;
	signed count;
	if (pcap_findalldevs (&devices, buffer) == -1)
	{
		error (1, errno, "can't enumerate pcap devices");
	}
	for (device = devices, count = 1; device; device = device->next, count++)
	{
		if (count == index)
		{
			name = strdup (device->name);
			break;
		}
	}
	if (!device)
	{
		error (1, EINVAL, "invalid interface: %d", index);
	}
	pcap_freealldevs (devices);

#else
#error "Unknown environment"
#endif

	return (name);
}


#endif

