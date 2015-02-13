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
 *   signed TopologyPrint (struct plcnetworks * plctopology);
 *
 *   plc.h
 *
 *   print plctopology structure in human readable format on stdout;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef TOPOLOGYPRINT_SOURCE
#define TOPOLOGYPRINT_SOURCE

#include <stdio.h>

#include "../plc/plc.h"
#include "../tools/memory.h"

signed PLCTopologyPrint (struct plctopology * plctopology)

{
	signed plcnetworks = plctopology->plcnetworks;
	struct plcnetwork * plcnetwork = (struct plcnetwork *)(&plctopology->plcnetwork);
	while (plcnetworks--)
	{
		signed plcstations = plcnetwork->plcstations;
		struct plcstation * plcstation = (struct plcstation *)(&plcnetwork->plcstation);
		while (plcstations--)
		{
			char address [ETHER_ADDR_LEN * 3];
			printf ("%s ", plcstation->LOC? "LOC": "REM");
			printf ("%s ", plcstation->CCO? "CCO": "STA");
			printf ("%03d ", plcstation->TEI);
			printf ("%s ", hexstring (address, sizeof (address), plcstation->MAC, sizeof (plcstation->MAC)));
			printf ("%s ", hexstring (address, sizeof (address), plcstation->BDA, sizeof (plcstation->BDA)));
			printf ("%03d ", plcstation->RX);
			printf ("%03d ", plcstation->TX);
			printf ("%s ", plcstation->hardware);
			printf ("%s ", plcstation->firmware);
			printf ("\n");
			plcstation++;
		}
		plcnetwork = (struct plcnetwork *)(plcstation);
	}

	return (0);
}


#endif

