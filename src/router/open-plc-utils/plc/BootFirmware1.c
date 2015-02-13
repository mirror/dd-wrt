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
 *   signed BootFirmware1 (struct plc * plc);
 *
 *   plc.h
 *
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef BOOTFIRMWARE1_SOURCE
#define BOOTFIRMWARE1_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "../tools/files.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../plc/plc.h"
#include "../nvm/nvm.h"

signed BootFirmware1 (struct plc * plc)

{
	unsigned module = 0;
	struct nvm_header1 nvm_header;
	if (lseek (plc->NVM.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, plc->NVM.name);
	}
	do
	{
		if (read (plc->NVM.file, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			error (PLC_EXIT (plc), errno, NVM_HDR_CANTREAD, plc->NVM.name, module);
		}
		if (LE32TOH (nvm_header.HEADERVERSION) != 0x60000000)
		{
			error (PLC_EXIT (plc), ECANCELED, NVM_HDR_VERSION, plc->NVM.name, module);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			error (PLC_EXIT (plc), ECANCELED, NVM_HDR_CHECKSUM, plc->NVM.name, module);
		}

#if 0

		if (_anyset (LE16TOH (nvm_header.IGNOREMASK), (1 << (plc->hardwareID - 1))))
		{
			if (lseek (plc->NVM.file, LE32TOH (nvm_header.NEXTHEADER), SEEK_SET) == -1)
			{
				error (PLC_EXIT (plc), errno, "Can't skip module: %s (%d)", plc->NVM.name, module);
			}
		}
		else

#endif

		if (nvm_header.IMAGETYPE == NVM_IMAGE_FIRMWARE)
		{
			if (plc->hardwareID < CHIPSET_AR7400)
			{
				if (WriteFirmware1 (plc, module, &nvm_header))
				{
					return (-1);
				}
				if (StartFirmware1 (plc, module, &nvm_header))
				{
					return (-1);
				}
				break;
			}
			if (WriteExecuteFirmware1 (plc, module, &nvm_header))
			{
				return (-1);
			}
			break;
		}
		if (lseek (plc->NVM.file, LE32TOH (nvm_header.NEXTHEADER), SEEK_SET) == -1)
		{
			error (PLC_EXIT (plc), errno, "Can't skip module: %s (%d)", plc->NVM.name, module);
		}
		module++;
	}
	while (nvm_header.NEXTHEADER);
	return (0);
}


#endif

