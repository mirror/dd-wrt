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
 *   nvram.c - NVRAM type information;
 *
 *   nvram.h
 *
 *   nvram codes for display purposes;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef NVRAM_SOURCE
#define NVRAM_SOURCE

#include "../tools/types.h"
#include "../tools/symbol.h"
#include "../ram/nvram.h"

#define NVRAM_TYPES (sizeof (nvram_types) / sizeof (struct _type_))

static const struct _type_ nvram_types [] =

{
	{
		0x00000005,
		"500KB"
	},
	{
		0x00000010,
		"1MB"
	},
	{
		0x00000011,
		"M25P20"
	},
	{
		0x00000012,
		"M25P40"
	},
	{
		0x00000013,
		"M25P80"
	},
	{
		0x00000014,
		"M25P16_ES"
	},
	{
		0x00000015,
		"M25P32_ES"
	},
	{
		0x00000016,
		"M25P64_ES"
	},
	{
		0x00000046,
		"AT26DF161"
	},
	{
		0x00000146,
		"AT26DF161A"
	},
	{
		0x00001020,
		"M25P05A"
	},
	{
		0x00001120,
		"M25P10A"
	},
	{
		0x00001402,
		"S25FL016AOLMFI01"
	},
	{
		0x00001420,
		"M25P80"
	},
	{
		0x00001520,
		"M25P16"
	},
	{
		0x00001532,
		"FM25S16/FM25Q16"
	},
	{
		0x00001571,
		"M25PX16"
	},
	{
		0x00001620,
		"M25P32"
	},
	{
		0x00001632,
		"FM25Q32"
	},
	{
		0x00001720,
		"M25P64"
	},
	{
		0x00004125,
		"SST25VF016B"
	}
};

char const * NVRAMName (uint16_t NVRAMTYPE)

{
	return (typename (nvram_types, NVRAM_TYPES, NVRAMTYPE, "Unknown NVRAM Type"));
}


#endif

