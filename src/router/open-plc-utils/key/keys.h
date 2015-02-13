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
 *   keys.h -
 *
 *.  Qualcomm Atheros HomePlug AV Powerline Toolkit
 *:  Copyright (c) 2009-2013 by Qualcomm Atheros Inc. ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *--------------------------------------------------------------------*/

#ifndef KEYS_HEADER
#define KEYS_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"
#include "../key/HPAVKey.h"

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define PASSWORD_SILENCE (1 << 0)
#define PASSWORD_VERBOSE (1 << 1)

#define DAK0 "00000000000000000000000000000000"
#define DAK1 "689F074B8B0275A2710B0B5779AD1630"
#define DAK2 "F084B4E8F6069FF1300C9BDB812367FF"

#define NMK0 "00000000000000000000000000000000"
#define NMK1 "50D3E4933F855B7040784DF815AA8DB7"
#define NMK2 "B59319D7E8157BA001B018669CCEE30D"

#define KEYS 3

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

typedef struct key

{
	char const * phrase;
	uint8_t DAK [HPAVKEY_DAK_LEN];
	uint8_t NMK [HPAVKEY_NMK_LEN];
}

key;

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

extern const struct key keys [];

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

void MACPasswords (unsigned vendor, unsigned device, unsigned count, unsigned alpha, unsigned bunch, char space, flag_t flags);
void RNDPasswords (unsigned vendor, unsigned device, unsigned count, unsigned alpha, unsigned bunch, char space, flag_t flags);
char * strnpwd (char buffer [], unsigned length, unsigned count, unsigned group, char space);
void putpwd (unsigned count, unsigned group, char space);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

