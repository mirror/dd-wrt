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
 *   signed QualcommHeader1 (struct qualcomm_fmi * header, uint8_t MMV, uint16_t MMTYPE, uint16_tFMI);
 *
 *   mme.h
 *
 *   Encode memory with an Atheros vendor specific message header
 *   having HomePlug message protocol version (INTELLON_MMV) and
 *   Atheros message type (MMTYPE);
 *
 *   return the number of bytes actually encoded or 0 on encode error;
 *   the error code is stored in errno;
 *
 *   see the INT6000 Firmware Technical Reference Manual for more
 *   about MME headers and message types; the Atheros OUI is
 *   implicit in this function;
 *
 *   MMV is the version number of the MME command set; currently,
 *   there is only one command set for Atheros MMEs;
 *
 *   MMTYPE indicates the desired Atheros device operation taken
 *   from the TRM; some operations are undocumented and should not
 *   be used;
 *
 *   OUI is the Organizationally Unique Identifier resgistered with
 *   the IEEE by the vendor and is a constant for Atheros Devices;
 *
 *   There is no need to flush the header since this function writes
 *   to all locations unless there is an error; the caller may elect
 *   to flush the buffer before calling this function;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef FRAGMENTHEADER_SOURCE
#define FRAGMENTHEADER_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../tools/endian.h"
#include "../mme/mme.h"

signed QualcommHeader1 (struct qualcomm_fmi * header, uint8_t MMV, uint16_t MMTYPE)

{
	header->MMV = MMV;
	header->MMTYPE = HTOLE16 (MMTYPE);
	header->FMSN = 0;
	header->FMID = 0;
	header->OUI [0] = 0x00;
	header->OUI [1] = 0xB0;
	header->OUI [2] = 0x52;
	return (sizeof (* header));
}


#endif

