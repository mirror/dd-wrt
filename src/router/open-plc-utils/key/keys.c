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
 *   keys.c - Encryption Key Data and Functions;
 *
 *   keys.h
 *
 *   Tabulate default Atheros pass phrases and their DAK, NMK and
 *   NID for search and conversion purposes;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef KEYS_SOURCE
#define KEYS_SOURCE

#include "../key/keys.h"

const struct key keys [KEYS] =

{
	{
		"none/secret",
		{
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00
		},
		{
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00
		}
	},
	{
		"HomePlugAV",
		{
			0x68,
			0x9F,
			0x07,
			0x4B,
			0x8B,
			0x02,
			0x75,
			0xA2,
			0x71,
			0x0B,
			0x0B,
			0x57,
			0x79,
			0xAD,
			0x16,
			0x30
		},
		{
			0x50,
			0xD3,
			0xE4,
			0x93,
			0x3F,
			0x85,
			0x5B,
			0x70,
			0x40,
			0x78,
			0x4D,
			0xF8,
			0x15,
			0xAA,
			0x8D,
			0xB7
		}
	},
	{
		"HomePlugAV0123",
		{
			0xF0,
			0x84,
			0xB4,
			0xE8,
			0xF6,
			0x06,
			0x9F,
			0xF1,
			0x30,
			0x0C,
			0x9B,
			0xDB,
			0x81,
			0x23,
			0x67,
			0xFF
		},
		{
			0xB5,
			0x93,
			0x19,
			0xD7,
			0xE8,
			0x15,
			0x7B,
			0xA0,
			0x01,
			0xB0,
			0x18,
			0x66,
			0x9C,
			0xCE,
			0xE3,
			0x0D
		}
	}
};


#endif

