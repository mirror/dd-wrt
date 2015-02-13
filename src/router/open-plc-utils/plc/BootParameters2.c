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
 *   signed BootParameters2 (struct plc * plc)
 *
 *   pib.h
 *
 *   write the parameter information block to SDRAM on a panther/
 *   lynx powerline device;
 *
 *   the parameter information block is stored in an NVM image chain
 *   and the image header specifies the memory location;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef BOOTPARAMETERS2_SOURCE
#define BOOTPARAMETERS2_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "../tools/files.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../plc/plc.h"
#include "../pib/pib.h"

signed BootParameters2 (struct plc * plc)

{
	unsigned module = 0;
	struct nvm_header2 nvm_header;
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, plc->PIB.name);
	}
	do
	{
		if (read (plc->PIB.file, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			error (1, errno, NVM_HDR_CANTREAD, plc->PIB.name, module);
		}
		if (LE16TOH (nvm_header.MajorVersion) != 1)
		{
			error (1, ECANCELED, NVM_HDR_VERSION, plc->PIB.name, module);
		}
		if (LE16TOH (nvm_header.MinorVersion) != 1)
		{
			error (1, ECANCELED, NVM_HDR_VERSION, plc->PIB.name, module);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			error (1, ECANCELED, NVM_HDR_CHECKSUM, plc->PIB.name, module);
		}

#if 0

		if (_allclr (LE16TOH (nvm_header.ExecuteMask), (1 << (plc->hardwareID - 1))))
		{
			if (lseek (plc->PIB.file, LE32TOH (nvm_header.NextHeader), SEEK_SET) == -1)
			{
				error (1, errno, "Can't skip module: %s (%d)", plc->PIB.name, module);
			}
		}
		else

#endif

		if (LE32TOH (nvm_header.ImageType) == NVM_IMAGE_PIB)
		{
			if (WriteExecuteParameters2 (plc, module, &nvm_header))
			{
				return (-1);
			}
			break;
		}
		if (lseek (plc->PIB.file, LE32TOH (nvm_header.NextHeader), SEEK_SET) == -1)
		{
			error (1, errno, "Can't skip module: %s (%d)", plc->PIB.name, module);
		}
		module++;
	}
	while (~nvm_header.NextHeader);
	return (0);
}


#endif

