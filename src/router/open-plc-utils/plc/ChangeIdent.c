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
 *   signed ChangeIdent (struct plc * plc);
 *
 *   plc.h
 *
 *   read PIB file header; replace MAC, DAK and NMK; update preferred
 *   NID; re-write PIB file header and PIB file checksum; use the MAC,
 *   DAK and NMK values stored in struct plc;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef CHANGEIDENT_SOURCE
#define CHANGEIDENT_SOURCE

#include <unistd.h>
#include <stdint.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../key/HPAVKey.h"
#include "../pib/pib.h"

signed ChangeIdent (struct plc * plc)

{
	struct simple_pib simple_pib;
	memset (&simple_pib, 0, sizeof (simple_pib));
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, plc->PIB.name);
	}
	if (read (plc->PIB.file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTREAD, plc->PIB.name);
	}
	memcpy (simple_pib.MAC, plc->MAC, sizeof (simple_pib.MAC));
	memcpy (simple_pib.DAK, plc->DAK, sizeof (simple_pib.DAK));
	memcpy (simple_pib.NMK, plc->NMK, sizeof (simple_pib.NMK));
	{
		uint8_t level = simple_pib.PreferredNID [HPAVKEY_NID_LEN-1] >> 4;
		HPAVKeyNID (simple_pib.PreferredNID, simple_pib.NMK, level & 1);
	}
	simple_pib.CHECKSUM = 0;
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, plc->PIB.name);
	}
	if (write (plc->PIB.file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTSAVE, plc->PIB.name);
	}
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, plc->PIB.name);
	}
	simple_pib.CHECKSUM = fdchecksum32 (plc->PIB.file, LE16TOH (simple_pib.PIBLENGTH), 0);
	if (lseek (plc->PIB.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, plc->PIB.name);
	}
	if (write (plc->PIB.file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTSAVE, plc->PIB.name);
	}
	return (0);
}


#endif

