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
 *   uint16_t pibscalers (struct _file_ * pib);
 *
 *   return the number of scalers (carriers) based on the FWVERSION
 *   stored in a .pib file; this function is a fix so that older
 *   toolkit programs will work with newer chipsets;
 *
 *   chipsets with the following carriers are supported:
 *     - 1155 carriers
 *     - 1345 carriers
 *     - 2880 carriers
 *
 *--------------------------------------------------------------------*/

#ifndef PIBSCALERS_SOURCE
#define PIBSCALERS_SOURCE

#include <unistd.h>
#include <errno.h>

#include "../plc/plc.h"
#include "../pib/pib.h"
#include "../tools/error.h"

uint16_t pibscalers (struct _file_ * pib)

{
	// TODO: improve chipset detection

	struct pib_header pib_header;
	if (read (pib->file, &pib_header, sizeof (pib_header)) != sizeof (pib_header))
	{
		error (1, errno, "%s", pib->name);
	}
	if ((pib_header.FWVERSION  == 0x01) &&
		(pib_header.PIBVERSION == 0x00))
	{
		return (PLC_CARRIERS);
	}
	if (pib_header.FWVERSION < 0x05)
	{
		return (INT_CARRIERS);
	}
	return (AMP_CARRIERS);
}


#endif

