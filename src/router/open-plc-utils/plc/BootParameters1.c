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
 *   signed BootParameters1 (struct plc * plc)
 *
 *   pib.h
 *
 *   write the parameter information block to SDRAM on a thunderbolt/
 *   lightning powerline device;
 *
 *   The PIB is written to different SDRAM locations depending on the
 *   chipset used; we use PIB major/minor version to determine where
 *   to write the PIB but may switch to using the hardware platform
 *   code derived from the VS_SW_VER message;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef BOOTPARAMETERS1_SOURCE
#define BOOTPARAMETERS1_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "../tools/files.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../plc/plc.h"
#include "../pib/pib.h"

signed BootParameters1 (struct plc * plc)

{
	struct pib_header pib_header;
	uint32_t offset;
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (PLC_EXIT (plc), errno, FILE_CANTHOME, plc->PIB.name);
	}
	if (read (plc->PIB.file, &pib_header, sizeof (pib_header)) != sizeof (pib_header))
	{
		error (PLC_EXIT (plc), errno, FILE_CANTREAD, plc->PIB.name);
	}
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (PLC_EXIT (plc), errno, FILE_CANTHOME, plc->PIB.name);
	}

#if 1

/*
 *   this code is a fix to accommodate PIB relocation in memory; it is not needed when
 *   the PIB is stored in an NVM file since the memory address is recorded in the image
 *   header;
 */

	if (BE16TOH (*(uint16_t *)(&pib_header)) < 0x0305)
	{
		offset = LEGACY_PIBOFFSET;
	}
	else if (BE16TOH (*(uint16_t *)(&pib_header)) < 0x0500)
	{
		offset = INT6x00_PIBOFFSET;
	}
	else
	{
		offset = AR7x00_PIBOFFSET;
	}

#endif

	if (plc->hardwareID < CHIPSET_AR7400)
	{
		if (WriteMEM (plc, &plc->PIB, 0, offset, LE16TOH (pib_header.PIBLENGTH)))
		{
			return (-1);
		}
		return (0);
	}
	if (WriteExecutePIB (plc, offset, &pib_header))
	{
		return (-1);
	}
	return (0);
}


#endif

