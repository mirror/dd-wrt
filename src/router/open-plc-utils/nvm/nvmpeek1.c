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
 *   void nvmpeek1 (void const * memory);
 *
 *   print a memory resident firmware image header on stdout;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef NVMPEEK1_SOURCE
#define NVMPEEK1_SOURCE

#include <stdio.h>

#include "../tools/memory.h"
#include "../tools/format.h"
#include "../nvm/nvm.h"

void nvmpeek1 (void const * memory)

{
	extern char const * nvm_imagetypes [];
	extern char const * nvm_platforms [];
	struct nvm_header1 * nvm_header = (struct nvm_header1 *)(memory);
	char const * string = "Unknown";
	char platform [100];
	printf ("\tHeader Version = 0x%08X-%02X\n", LE32TOH (nvm_header->HEADERVERSION), nvm_header->HEADERMINORVERSION);
	printf ("\tHeader Checksum = 0x%08X\n", LE32TOH (nvm_header->HEADERCHECKSUM));
	printf ("\tHeader Next = 0x%08X\n", LE32TOH (nvm_header->NEXTHEADER));
	printf ("\tFlash Address = 0x%08X\n", LE32TOH (nvm_header->IMAGEROMADDR));
	printf ("\tImage Address = 0x%08X\n", LE32TOH (nvm_header->IMAGEADDRESS));
	printf ("\tEntry Address = 0x%08X\n", LE32TOH (nvm_header->ENTRYPOINT));
	printf ("\tImage Checksum = 0x%08X\n", LE32TOH (nvm_header->IMAGECHECKSUM));
	printf ("\tImage Size = 0x%08X (%d)\n", LE32TOH (nvm_header->IMAGELENGTH), LE32TOH (nvm_header->IMAGELENGTH));
	if (LE32TOH (nvm_header->IMAGETYPE) < NVM_IMAGETYPES)
	{
		string = nvm_imagetypes [LE32TOH (nvm_header->IMAGETYPE)];
	}
	printf ("\tImage Type = %s\n", string);
	strfbits (platform, sizeof (platform), nvm_platforms, "|", LE16TOH (nvm_header->IGNOREMASK));
	printf ("\tImage Omit = %s\n", platform);
	return;
}


#endif

