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
 *   signed ExecuteApplets2 (struct plc * plc);
 *
 *   plc.h
 *
 *   download and execute all modules in a new nvm image files;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef EXECUTEAPPLETS2_SOURCE
#define EXECUTEAPPLETS2_SOURCE

#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../plc/plc.h"

signed ExecuteApplets2 (struct plc * plc)

{
	unsigned module = 0;
	struct nvm_header2 nvm_header;
	if (lseek (plc->NVM.file, 0, SEEK_SET))
	{
		if (_allclr (plc->flags, PLC_SILENCE))
		{
			error (PLC_EXIT (plc), errno, FILE_CANTHOME, plc->NVM.name);
		}
		return (-1);
	}
	do
	{
		if (read (plc->NVM.file, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			if (_allclr (plc->flags, PLC_SILENCE))
			{
				error (PLC_EXIT (plc), errno, NVM_HDR_CANTREAD, plc->NVM.name, module);
			}
			return (-1);
		}
		if (LE16TOH (nvm_header.MajorVersion) != 1)
		{
			if (_allclr (plc->flags, PLC_SILENCE))
			{
				error (PLC_EXIT (plc), 0, NVM_HDR_VERSION, plc->NVM.name, module);
			}
			return (-1);
		}
		if (LE32TOH (nvm_header.MinorVersion) != 1)
		{
			if (_allclr (plc->flags, PLC_SILENCE))
			{
				error (PLC_EXIT (plc), 0, NVM_HDR_VERSION, plc->NVM.name, module);
			}
			return (-1);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			if (_allclr (plc->flags, PLC_SILENCE))
			{
				error (PLC_EXIT (plc), 0, NVM_HDR_CHECKSUM, plc->NVM.name, module);
			}
			return (-1);
		}
		if (WriteExecuteApplet2 (plc, module, &nvm_header))
		{
			return (-1);
		}
		if (_anyset (plc->flags, PLC_QUICK_FLASH))
		{
			break;
		}
		while (!ReadMME (plc, 0, (VS_HOST_ACTION | MMTYPE_IND)));
		module++;
	}
	while (~nvm_header.NextHeader);
	return (0);
}


#endif

