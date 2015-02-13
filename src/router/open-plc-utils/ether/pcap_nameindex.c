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
 *   struct if_nameindex * pcap_nameindex (void)
 *
 *   ether.h
 *
 *   a WinPcap version of POSIX if_nameindex function; return error
 *   in non-pcap environments;
 *
 *   see The Open Group Base Specifications Issue 6 IEEE Std 1003.1,
 *   2004 Edition for a description of this function;
 *
 *
 *--------------------------------------------------------------------*/

#ifndef PCAP_NAMEINDEX_SOURCE
#define PCAP_NAMEINDEX_SOURCE

#include <pcap.h>
#include <string.h>
#include <stdlib.h>

#include "../ether/ether.h"

struct if_nameindex * pcap_nameindex (void)

{

#if defined (WINPCAP) || defined (LIBPCAP)

	char buffer [PCAP_ERRBUF_SIZE];
	pcap_if_t * devices = (pcap_if_t *)(0);
	pcap_if_t * device;
	if (pcap_findalldevs (&devices, buffer) != -1)
	{
		struct if_nameindex * ifs;
		struct if_nameindex * ifp;
		unsigned count = 1;
		for (device = devices; device; device = device->next)
		{
			count++;
		}
		ifp = ifs = (struct if_nameindex *)(malloc (count * sizeof (struct if_nameindex)));
		if (ifs) for (device = devices; device; device = device->next)
		{
			ifp->if_index = device->index;
			ifp->if_name = strdup (device->name);
			ifp++;
		}
		memset (ifp, 0, sizeof (* ifp));
		pcap_freealldevs (devices);
		return (ifs);
	}

#endif

	return ((struct if_nameindex *)(0));
}


#endif

